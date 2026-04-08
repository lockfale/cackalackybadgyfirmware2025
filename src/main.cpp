/**
 * @file main.cpp
 * @brief Main entry point for the badge firmware.
 *
 * This file contains the setup and loop functions for the badge firmware.
 * It initializes hardware components, sets up tasks, and handles button
 * interactions and game logic.
 */

#include "badge.h"

// Standard Library Includes
#include <Arduino.h>
#include <LittleFS.h>

// Third-Party Library Includes
#include <PinButton.h>

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskScheduler.h>

// Hardware Includes
#include "accelerometer.h"
#include "badge_heartbeat.h"
#include "badge_haptic_tasks.h"
#include "badge_status.h"
#include "badge_display.h"
#include "haptic.h"
#include "PCF8574.h"

// Networking Includes
#include "BadgeWiFi.h"
#include "BadgeMQTT.h"
#include "NetworkConfig.h"
#include "NetworkTaskManager.h"

#ifndef ENABLE_KIOSK_MODE
// Game Logic Includes
#include "ballhole.h"
#include "slot.h"
#include "cyberSpa.h"
#include "cyberPartnerGame.h"
#include "CPStoreDisplay.h"
#include "CPInventoryDisplay.h"
#include "CPMating.h"
#include "minigame_template.h" // TODO removeme?
#include "ogrady.h"
#include "roulotto.h"
#include "weightShake.h"
#endif  // ENABLE_KIOSK_MODE

// Utility Includes
#include "badge_eeprom.h"
#include "badgeEmoji.h"
#include "badgeMenu.h"
#include "badgeOneWireMenu.h"
#include "ccserial.h"
#include "configData.h"
#include "ProgressConfig.h"
#include "qrcode.h"
#include "registration.h"

#define BADGE_SERVER_FINGERPRINT "8F:4A:D8:8B:4B:EC:66:1B:E4:22:04:8A:8C:A5:5B:4B:02:57:CB:5C" // badge.cackalacky.ninja
#define BUTTON_COOLDOWN_RESET 200
int constexpr ACCELEROMETER_PIN = 0x0D; // original value: 13

uint32_t buttonCoolDown = 0;

bool displayLogo = true;
bool startedRegistration = false;
bool tap_handler_flag = false;
bool freefall_handler_flag = false;

#ifdef BADGEDEBUG
int8_t detected_tap_type = 0;
unsigned long detected_tap_time = 0;
#endif  // BADGEDEBUG

PinButton bootButton(BOOT_BUTT);
PinButton leftButton(LEFT_BUTT,INPUT, true);
PinButton rightButton(RIGHT_BUTT, true);
PinButton middleButton(MIDDLE_BUTT, INPUT_PULLUP, true);

PCF8574 pcf8574(0x20, 4, 5);

// Scheduler
Scheduler baseTaskRunner;
Scheduler highPriorityTaskRunner;

Task t_accelerometer(50, TASK_FOREVER, []() { BadgeAcceler::getInstance().taskCallback(); }, &baseTaskRunner);
Task t_haptic(100, TASK_FOREVER, &BadgeHapticTask::taskCallback, &baseTaskRunner);
Task t_networkQueueCheck(500, TASK_FOREVER, []() { NetworkTaskManager::getInstance().taskNetworkQueueCheck(); }, &baseTaskRunner);
Task t_ingressMQTTQueueCheck(250, TASK_FOREVER, []() { NetworkTaskManager::getInstance().processMQTTQueueIngress(); }, &baseTaskRunner);

/**
 * @brief Interrupt service routine for LIS2DW12 interrupt events.
 *
 * Set an interrupt flag to be handled in the main loop.
 * This function is called when the accelorometer registers an interrupt
 * This function is called when a signal is sent to pin 0x37 (For more info, see main.cpp setup()
 */
void IRAM_ATTR sharedInterruptHandler() {
    uint8_t status = BadgeAcceler::getInstance().getInterruptStatus();
    if (status & 0x02) { // Free-fall event  // Status: 67 (decimal)
        // Start the freefall timer
        BadgeAcceler::getInstance().isFreeFallDetected();
        freefall_handler_flag = true;
    }
    if (status & 0x08) { // Single-tap event  // Status: 73 (decimal)
        tap_handler_flag = true;
    }
    if (status & 0x10) { // Double-tap event
        tap_handler_flag = true;
    }
}

uint32_t bootHeapSize;

/**
 * @brief Gets the badge ID.
 * @return The badge ID.
 */
uint32_t getBadgeID() {
    return ESP.getChipId();
}

PCF8574::DigitalInput di = pcf8574.digitalReadAll();

/**
 * @brief Setup function for initializing hardware and tasks.
 *
 * This function initializes the hardware components, sets up the tasks,
 * and loads the network configuration. It also handles the initial setup
 * of the NeoController, display, accelerometer, and other components.
 */
void setup() {
    bootHeapSize = ESP.getFreeHeap();
    Serial.begin(serial_baud_rate);
    attachInterrupt(ACCELEROMETER_PIN, sharedInterruptHandler, RISING);  // previously attaching to ACCELEROMETER_PIN
    BadgeAcceler::getInstance().setTapInterruptAttached(true);

    while(!Serial);
    Serial.println();

    bEeprom::getInstance().setup();
    LittleFS.begin();
    if (!NetworkConfig::getInstance().loadNetworkFile()) {
        Serial.println("Failed to load network credentials.");
    } else {
        Serial.println("Network credentials loaded successfully.");
    }

    // Setup hardware
    NeoController& neo = NeoController::getInstance();
    neo.setup();
    neo.changeRed();
    BDISPLAY::setup();
    BadgeAcceler::getInstance().setup();

    pcf8574.pinMode(P0, INPUT);
    pcf8574.pinMode(P1, INPUT);
    pcf8574.pinMode(P2, INPUT);
    bool pcf8574_ready = false;
    if (pcf8574.begin()) {
        Serial.println(F("PCF8574: Ok"));
        pcf8574_ready = true;
    } else {
        Serial.println(F("PCF8574: Failed"));
    }

    #ifndef ENABLE_KIOSK_MODE
    // Save file things
    CFG::saveSecret();
    ProgressConfig::getInstance().loadProgressFile();

    // Game & Unlocks setup
    // Apply the display effect on boot if the challenge has been unlocked
    if (ProgressConfig::getInstance().isD3adb33fUnlocked()) {
        BDISPLAY::display.invertDisplay(true); // Invert colors
    } else {
        BDISPLAY::display.invertDisplay(false); // Normal colors
    }
    BH::getInstance().setup();
    SLOT::getInstance().setup();
    OGS::getInstance().setup();
    #endif  // ENABLE_KIOSK_MODE

    // Getting Serial online
    ccSerial::setup();

    EMOJI::setup();

    if(pcf8574_ready) {
        // Ignore events for 300ms after boot
        int counter = 0;

        Serial.println(F("Wait for pins to stabilize, ignoring button reads"));
        while (counter < 350) {
            delay(1);
            counter++;
            PCF8574::DigitalInput di = pcf8574.digitalReadAll();
            leftButton.update_advanced(di.p0);
            middleButton.update_advanced(di.p1);
            rightButton.update_advanced(di.p2);
        }
        Serial.println(F("Stabilized"));
    }
    Serial.println("ESP8266 Boot Heap Stats:");
    Serial.print("Heap at Boot: ");
    Serial.print(bootHeapSize);
    Serial.println(" bytes");
    Serial.print("Badge ID: ");
    Serial.printf("badge-%08X\n", ESP.getChipId());
    Serial.println();

    baseTaskRunner.setHighPriorityScheduler(&highPriorityTaskRunner);
    baseTaskRunner.enableAll(true);

    // Initialize network tasks
    NetworkTaskManager::getInstance().setupNetworkTasks();
    BadgeHeartbeat::startHeartbeat();


    #ifdef ENABLE_KIOSK_MODE
    // Enable the Menu in Kiosk mode
    BMenu::enableMenu();
    #endif  // ENABLE_KIOSK_MODE
}

/**
 * @brief Main loop function for handling tasks and button interactions.
 *
 * This function executes the scheduled tasks, updates the button states,
 * and handles interactions with the various game modes and menus.
 */
void loop() {
    baseTaskRunner.execute();
    // Check button status and update the Display
    bootButton.update();
    static unsigned long lastButtonReadTime = 0;

    // Check if it's time to read the buttons again
    unsigned long currentTime = millis();
    if (currentTime - lastButtonReadTime >= 20) {
        lastButtonReadTime = currentTime;
        di = pcf8574.digitalReadAll();
    }
    leftButton.update_advanced(di.p0);
    middleButton.update_advanced(di.p1);
    rightButton.update_advanced(di.p2);

    #ifndef ENABLE_KIOSK_MODE
    // For logo display, but will trigger for any other scrolling bitmap
    if (BDISPLAY::scrollingBitmapState.active) {
        if (displayLogo) {
            BDISPLAY::updateScrollingBitmap();
        }
    } else {
        // If the logo is not being displayed, set the flag to false
        displayLogo = false;
        if (!startedRegistration) {
            Registration::getInstance().doStart();
            startedRegistration = true;
        }
    }
    #endif  // ENABLE_KIOSK_MODE

    // Disable accelerometer interrupts while the haptic motor is running
    // this eliminates legitimate HAPTAPs while the motor is on
    if (BadgeHaptic::getInstance().isHapticOn()) {
        // Only detach the interrupt once per haptic
/*      // Leaving these gnarly printdebugs in case we have more haptic/accelo woes down the road
        #ifdef BADGEDEBUG
        Serial.printf("Detach check:\n\tisTapInterruptAttached: %d\n\tTIMEDELTA (CT-LADT)%lums (%lums-%lums)\n",
            BadgeAcceler::getInstance().isTapInterruptAttached(), (currentTime - BadgeAcceler::getInstance().getLastAccelerDetachTime()), currentTime, BadgeAcceler::getInstance().getLastAccelerDetachTime());
        #endif  // BADGEDEBUG
*/
            if (BadgeAcceler::getInstance().isTapInterruptAttached()){
                detachInterrupt(digitalPinToInterrupt(ACCELEROMETER_PIN));
                BadgeAcceler::getInstance().setLastAccelerDetachTime(millis());
                BadgeAcceler::getInstance().setTapInterruptAttached(false);
                #ifdef BADGEDEBUG
                Serial.printf("Detached Tap Interrupt after %lums\n", BadgeAcceler::getInstance().getLastAccelerDetachTime());
                #endif  // BADGEDEBUG
        }
    // Otherwise the haptic is off, reattach the interrupt after 300ms
    // to eliminate inertia-driven "ghost" taps
    } else {
        currentTime = millis();
/*      // Leaving these gnarly printdebugs in case we have more haptic/accelo woes down the road
        #ifdef BADGEDEBUG
        Serial.printf("Reattach check:\n\tisTapInterruptAttached: %d\n\tTIMEDELTA (CT-LADT)%lums (%lums-%lums)\n",
            BadgeAcceler::getInstance().isTapInterruptAttached(), (currentTime - BadgeAcceler::getInstance().getLastAccelerDetachTime()), currentTime, BadgeAcceler::getInstance().getLastAccelerDetachTime());
        #endif  // BADGEDEBUG
*/
        if (!BadgeAcceler::getInstance().isTapInterruptAttached()) {
            if (currentTime - BadgeAcceler::getInstance().getLastAccelerDetachTime() >= 300) {
                attachInterrupt(ACCELEROMETER_PIN, sharedInterruptHandler, RISING);  // previously ACCELEROMETER_PIN
                BadgeAcceler::getInstance().setTapInterruptAttached(true);
                #ifdef BADGEDEBUG
                currentTime = millis();
                Serial.printf("Reattached Tap Interrupt after %lums\n", currentTime);
                #endif  // BADGEDEBUG
            }
        }
    }

    // Tap interrupt flag routine
    if (tap_handler_flag) {
        tap_handler_flag = false;

        // We loaded this mother down with print debugs,
        // leaving in case there are future tap-related debugging to do
        #ifdef BADGEDEBUG
            detected_tap_type = BadgeAcceler::getInstance().tapDetect();
            detected_tap_time = millis();
            Serial.printf("ACCEL: Interrupt fired type, time:\t %d, %lums\n",detected_tap_type, detected_tap_time);
            if (BadgeHaptic::getInstance().isHapticOn()) {
                if (detected_tap_type == 0){
                    Serial.printf("\tACCEL: Single tap (0), \"Haptic ON\", handled time: %lums\n", detected_tap_time);
                } else if (detected_tap_type == 1){
                    Serial.printf("\tACCEL: Double tap (1), \"Haptic ON\", handled time: %lums\n", detected_tap_time);
                } else if (detected_tap_type == 2){
                    Serial.printf("\tACCEL: No tap (2), \"Haptic ON\", handled time: %lums\n", detected_tap_time);
                } else if (detected_tap_type == 3){
                    Serial.printf("\tACCEL: HAP tap (3), \"Haptic ON\", handled time: %lums\n", detected_tap_time);
                }
            } else {
                if (detected_tap_type == 0){
                    Serial.printf("\tACCEL: Single tap, \"Haptic OFF\", handled time: %lums\n", detected_tap_time);
                } else if (detected_tap_type == 1){
                    Serial.printf("\tACCEL: Double tap, \"Haptic OFF\", handled time: %lums\n", detected_tap_time);
                } else if (detected_tap_type == 2){
                    Serial.printf("\tACCEL: No tap, \"Haptic OFF\", handled time: %lums\n", detected_tap_time);
                } else if (detected_tap_type == 3){
                    Serial.printf("\tACCEL: HAP tap, \"Haptic OFF\", handled time: %lums\n", detected_tap_time);
                }
            }
        #endif  // BADGEDEBUG
    }


    // LIS2DW12 sensor reads
    currentTime = millis();
    if (currentTime % 1000 == 0) {
        #ifdef BADGEDEBUG

        // Commenting the following out for now, it is causing warnings in build since they are not used
        //int16_t x = BadgeAcceler::getInstance().readFloatAccelX();
        //int16_t y = BadgeAcceler::getInstance().readFloatAccelY();
        //int16_t z = BadgeAcceler::getInstance().readFloatAccelZ();

        //int8_t l_temperature_c = BadgeAcceler::getInstance().getTempLoResCelcius();
        //int8_t l_temperature_f = BadgeAcceler::getInstance().getTempLoResFarenheit();
        //float h_temperature_c = BadgeAcceler::getInstance().getTempHiResCelcius();
        //float h_temperature_k = BadgeAcceler::getInstance().getTempHiResKelvin();
        #endif // BADGEDEBUG
        float h_temperature_f = BadgeAcceler::getInstance().getTempHiResFarenheit();

        ProgressConfig& progconfig = ProgressConfig::getInstance();
        if (h_temperature_f >= 90.0f && !progconfig.isGotSweatyUnlocked()) {
            Serial.printf("It's %.2f°F! How could you let this baby get so hot?\n", h_temperature_f);
            Serial.println(F("Hot challenge unlocked!"));
            progconfig.unlockGotSweaty();
            progconfig.saveProgressFile();
        } else if (h_temperature_f <= 60.0f && !progconfig.isGotChillyUnlocked()) {
            Serial.printf("It's %.2f°F! How could you let this baby get so cold?\n", h_temperature_f);
            Serial.println(F("Cold challenge unlocked!"));
            progconfig.unlockGotChilly();
            progconfig.saveProgressFile();
        }
/*
        #ifdef BADGEDEBUG
        Serial.printf("[LIS2DW12] Temp Low Res: %d°C, %d°F\n", l_temperature_c, l_temperature_f);
        Serial.printf("[LIS2DW12] Temp High Res: %.2f°C, %.2f°F, %.2f°K\n",
            h_temperature_c, h_temperature_f, h_temperature_k);
        Serial.printf("[LIS2DW12] Accel: X: %d, Y: %d, Z: %d\n", x, y, z);
        #endif // BADGEDEBUG
*/
    }

    Registration& reg = Registration::getInstance();

    if(BMenu::isMenuRunning()){
        if (leftButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            BMenu::moveMenuUp();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            BMenu::moveMenuDown();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            BMenu::enterMenu();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            BMenu::escMenu();
        }
        BMenu::updateMenu();
    }
    #ifndef ENABLE_KIOSK_MODE
    else if(BH::getInstance().isRunning()){
        BH::getInstance().update();
        if (leftButton.isClick() || middleButton.isClick()
            || rightButton.isClick() || bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            BH::getInstance().doStop();
        }
    }
    else if(SLOT::getInstance().isRunning()){
        SLOT::getInstance().update();
        if (leftButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;

        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;

        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            SLOT::getInstance().doClick();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            SLOT::getInstance().doStop();
        }
    }
    else if(CYBERSPA::getInstance().isRunning()){
        CYBERSPA::getInstance().update();
        if (leftButton.isClick() && bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CYBERSPA::getInstance().doStop();
        } else if (leftButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CYBERSPA::getInstance().clickUp();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CYBERSPA::getInstance().clickDown();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CYBERSPA::getInstance().clickConfirm();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CYBERSPA::getInstance().clickCancel();
        }
    }
    else if(OGS::getInstance().isRunning()){
        OGS::getInstance().update();
        if (leftButton.isClick() && bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            OGS::getInstance().doStop();
        } else if (leftButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            OGS::getInstance().clickYellowButton();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            OGS::getInstance().clickRedButton();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            OGS::getInstance().clickGreenButton();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            OGS::getInstance().clickBlueButton();
        }
    }
    else if(ROULOTTO::getInstance().isRunning()){
        ROULOTTO::getInstance().update();
        if (leftButton.isClick() && bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            ROULOTTO::getInstance().doStop();
        } else if (leftButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            ROULOTTO::getInstance().clickUp();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            ROULOTTO::getInstance().clickDown();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            ROULOTTO::getInstance().clickConfirm();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            ROULOTTO::getInstance().clickCancel();
        }
    }
    else if(MINIGAME::getInstance().isRunning()){
        MINIGAME::getInstance().update();
        if (leftButton.isClick() && bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            MINIGAME::getInstance().doStop();
        } else if (leftButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            MINIGAME::getInstance().clickUp();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            MINIGAME::getInstance().clickDown();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            MINIGAME::getInstance().clickConfirm();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            MINIGAME::getInstance().clickCancel();
        }
    }
    else if(WEIGHTSHAKE::getInstance().isRunning()){
        WEIGHTSHAKE::getInstance().update();
        if (leftButton.isClick() && bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            WEIGHTSHAKE::getInstance().doStop();
        } else if (leftButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            WEIGHTSHAKE::getInstance().clickUp();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            WEIGHTSHAKE::getInstance().clickDown();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            WEIGHTSHAKE::getInstance().clickConfirm();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            WEIGHTSHAKE::getInstance().clickCancel();
        }
    }
    else if(reg.isActive() && !displayLogo){
        if (leftButton.isClick() || middleButton.isClick()
            || rightButton.isClick() || bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            if(reg.isRegistered()) {
                Serial.println(F("Button Press: Stopping registration..."));
                reg.doStop();
            }
        }
    }
    #endif  // ENABLE_KIOSK_MODE
    else if(BADGE_STATUS::isRunning()){
        BADGE_STATUS::update();
        if (leftButton.isClick() || middleButton.isClick()
            || rightButton.isClick() || bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            BADGE_STATUS::doStop();
        }
    }
    #ifndef ENABLE_KIOSK_MODE
    else if(QRCode::getInstance().isRunning()){
        if (leftButton.isClick() || middleButton.isClick()
            || rightButton.isClick() || bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            QRCode::getInstance().doStop();
        }
    }
    #endif // ENABLE_KIOSK_MODE
    else if(BadgeOneWireMenu::isActive()) {
        if (leftButton.isClick() || middleButton.isClick()
            || rightButton.isClick() || bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            BadgeOneWireMenu::setActive(0);
            BMenu::enableMenu();
        }
    }
    #ifndef ENABLE_KIOSK_MODE
    else if(CyberPartnerGame::getInstance().isActive()) {
        // Immediately handle freefall routine/input
        // If badge is actively in freefall, or freefall hasn't been fully handled, do work
        if (freefall_handler_flag) {
            #ifdef BADGEDEBUG
            Serial.println("ACCEL: Free-fall detected, standing by for landing...");
            #endif // BADGEDEBUG
            while (BadgeAcceler::getInstance().isFreeFallDetected()) {
                delay(1);
            }
            float freeFallDistance = BadgeAcceler::calculateFreeFallDistance();
            // Ignore drops less than ~4"/10cm
            if (freeFallDistance > 0.1f) {
                // Tell the user they dun goofed an dropped (or launched) the baby
                Serial.printf("ACCEL: In freefall for approx: %.2fm / %.1f\"\n", freeFallDistance, freeFallDistance * 39.3701f);
                CyberPartnerGame::getInstance().handleFreeFallEvent(freeFallDistance);
            #ifdef BADGEDEBUG
            } else {
                Serial.println("ACCEL: Freefall distance too small, ignoring.");
            #endif // BADGEDEBUG
            }
            // Reset the handler flag
            freefall_handler_flag = false;
        }
        // Then handle button inputs
        if (leftButton.isSingleClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CyberPartnerGame::getInstance().performLeftAction();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CyberPartnerGame::getInstance().performMiddleAction();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CyberPartnerGame::getInstance().performRightAction();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CyberPartnerGame::getInstance().disable();
            BMenu::enableMenu();
        }
    } else if (CPStoreDisplay::isActive()) {
        if (leftButton.isSingleClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPStoreDisplay::performLeftAction();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPStoreDisplay::performMiddleAction();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPStoreDisplay::performRightAction();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPStoreDisplay::performBootAction();
        }
    } else if (CPInventoryDisplay::isActive()) {
        if (leftButton.isSingleClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPInventoryDisplay::performLeftAction();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPInventoryDisplay::performMiddleAction();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPInventoryDisplay::performRightAction();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPInventoryDisplay::performBootAction();
        }
    } else if (CPMating::isActive()) {
        if (leftButton.isSingleClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPMating::performLeftAction();
        } else if (middleButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPMating::performMiddleAction();
        } else if (rightButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPMating::performRightAction();
        } else if (bootButton.isClick()) {
            buttonCoolDown = BUTTON_COOLDOWN_RESET;
            CPMating::performBootAction();
        }
    }
    #endif // ENABLE_KIOSK_MODE
    else if (buttonCoolDown > 0) {
        buttonCoolDown--;
    }

    if (BMenu::isMenuRunning()) {
        // Need networking things here
        if (buttonCoolDown > 0) {
            buttonCoolDown--;
        }
    }

    ccSerial::update();

    yield();

}
