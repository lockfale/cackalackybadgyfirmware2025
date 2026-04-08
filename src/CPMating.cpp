/**
 * @file CPMating.cpp
 * @brief Implements the CPMating class methods.
 *
 * This file contains the logic for handling CPMating Display functionality.
 * It includes the constructor, destructor, and methods for displaying the act of mating.
 */
#include "CPMating.h"
//#include <ArduinoJson.h>
#include "badgeMenu.h"
#include "badgeOneWireMenu.h"
#include "badge_eeprom.h"
#include "badge_display.h"
#include "registration.h"
#include "CyberPartner.h"

constexpr char touchStr[] PROGMEM = "Touch Someone";
constexpr char anotherStr[] PROGMEM = "else's button";

namespace {

    const uint8_t cpmate_sprites_sizes[][2] = {
        {58, 18}, // Dimensions for cpmate_connect
        {58, 14},  // Dimensions for cpmate_fail
        {64, 12}
    };

    // 'cpmate_connect', 58x18px
    const unsigned char cpmate_connect [] PROGMEM = {
        0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xc3, 0x00, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x30, 0xc0, 0x00, 0x00,
        0x00, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x00, 0x00, 0x00, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x00, 0x00,
        0xf3, 0xc3, 0x30, 0xc0, 0xc3, 0x30, 0xf3, 0xc0, 0xf3, 0xc3, 0x30, 0xc0, 0xc3, 0x30, 0xf3, 0xc0,
        0xff, 0xc3, 0x30, 0xc0, 0xc3, 0x30, 0xff, 0xc0, 0xff, 0xc3, 0x30, 0xc0, 0xc3, 0x30, 0xff, 0xc0,
        0x3f, 0x03, 0x30, 0xc0, 0xc3, 0x30, 0x3f, 0x00, 0x3f, 0x03, 0x30, 0xc0, 0xc3, 0x30, 0x3f, 0x00,
        0x0c, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x0c, 0x00,
        0x00, 0x00, 0xc3, 0x00, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x30, 0xc0, 0x00, 0x00,
        0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00
    };

    // 'cpmate_fail', 58x14px
    const unsigned char cpmate_fail [] PROGMEM = {
        0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x00,
        0x00, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x00, 0x00, 0x00, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x00, 0x00,
        0xf3, 0xc3, 0x30, 0x33, 0x03, 0x30, 0xf3, 0xc0, 0xf3, 0xc3, 0x30, 0x33, 0x03, 0x30, 0xf3, 0xc0,
        0xff, 0xc3, 0x30, 0x0c, 0x03, 0x30, 0xff, 0xc0, 0xff, 0xc3, 0x30, 0x0c, 0x03, 0x30, 0xff, 0xc0,
        0x3f, 0x03, 0x30, 0x33, 0x03, 0x30, 0x3f, 0x00, 0x3f, 0x03, 0x30, 0x33, 0x03, 0x30, 0x3f, 0x00,
        0x0c, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x30, 0xc0, 0xc3, 0x0c, 0x0c, 0x00,
        0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x00
    };

    // 'seesaw', 64x12px
    const unsigned char seesaw [] PROGMEM = {
        0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xc3, 0xc3, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xc3, 0xc3, 0x00, 0x00, 0x03,
        0xc0, 0x00, 0x00, 0xc3, 0xc3, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0xc3, 0xc3, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xcf, 0xf0, 0x00, 0x0f, 0x00, 0xff, 0xff, 0xff, 0xcf, 0xf0, 0x00, 0x0f, 0x00,
        0x00, 0xf0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x00, 0xf0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0x00,
        0x00, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00
    };


    const unsigned char* cpmate_sprites_images[] = {
        cpmate_connect, cpmate_fail
    };
}


const uint8_t (*CPMating::sprite_sizes)[2] = cpmate_sprites_sizes;
const unsigned char* const* CPMating::sprite_data = cpmate_sprites_images;

extern Scheduler baseTaskRunner;
Scheduler& CPMating::taskRunner = baseTaskRunner;

Task CPMating::displayTask(200, TASK_FOREVER);

CPMating::CPMATING_STATE_t CPMating::cpmating_state = CPMATING_STATE_t::CPMATING_DISPLAY_DISABLED;

CPMating& CPMating::getInstance() {
    static CPMating instance;
    return instance;
}

/**
 * @brief Configures the status bar for the CyberPartner game.
 */
void CPMating::displayStatusBar() {
    BDISPLAY::clearAndDisplayBatteryPercentage();

    BDISPLAY::display.setCursor(0, 0);
    BDISPLAY::display.setTextSize(1);
}

void CPMating::animateCPMating() {
    CPMating::displayStatusBar();

    BDISPLAY::displayCenteredText(touchStr, 10, 1);
    BDISPLAY::displayCenteredText(anotherStr, 20, 1);

    static uint8_t state = 0;

    // Initialize sprite dimensions based on the initial action
    uint8_t width = pgm_read_byte(&sprite_sizes[static_cast<uint8_t>(CPMateSprite::Connect)][0]);
    uint8_t height = pgm_read_byte(&sprite_sizes[static_cast<uint8_t>(CPMateSprite::Connect)][1]);

    int16_t x = (BDISPLAY::SCREEN_WIDTH - width) / 2;  // Center horizontally (128 is the screen width)
    int16_t y = 30;

    BDISPLAY::display.drawBitmap(x, y, (const uint8_t*)sprite_data[static_cast<uint8_t>(CPMateSprite::Connect)], width, height, WHITE);

    // Handle different states using a switch statement
    switch (state) {
        case 0:
            BDISPLAY::display.fillRect(x + 12, y, 34, height, BLACK);
            break;
        case 1:
            BDISPLAY::display.fillRect(x + 16, y, 26, height, BLACK);
            break;
        case 2:
            BDISPLAY::display.fillRect(x + 20, y, 18, height, BLACK);
            break;
        default:
            // Do nothing for the last state
            break;
    }

    // Update the state
    state = (state + 1) % 4; // Cycle through states 0, 1, 2, 3


    BDISPLAY::display.display();
}

void CPMating::animatePlaying() {
    CPMating::displayStatusBar();

    BDISPLAY::displayCenteredText(touchStr, 10, 1);
    BDISPLAY::displayCenteredText(anotherStr, 20, 1);

    static uint8_t state = 0;

    // Initialize sprite dimensions based on the initial action
    uint8_t width = pgm_read_byte(&sprite_sizes[2][0]);
    uint8_t height = pgm_read_byte(&sprite_sizes[2][1]);

    int16_t x = (BDISPLAY::SCREEN_WIDTH - width) / 2;  // Center horizontally (128 is the screen width)
    int16_t y = 30;

    BDISPLAY::display.drawBitmap(x, y, (const uint8_t*)sprite_data[2], width, height, WHITE);

    // Handle different states using a switch statement
    switch (state) {
        case 0:
            BDISPLAY::display.fillRect(x + 12, y, 34, height, BLACK);
            break;
        case 1:
            BDISPLAY::display.fillRect(x + 16, y, 26, height, BLACK);
            break;
        case 2:
            BDISPLAY::display.fillRect(x + 20, y, 18, height, BLACK);
            break;
        default:
            // Do nothing for the last state
            break;
    }

    // Update the state
    state = (state + 1) % 2; // Cycle through states 0, 1, 2, 3


    BDISPLAY::display.display();
}


void CPMating::displayStub()
{
    BDISPLAY::display.setCursor(0, 0);
    BDISPLAY::display.println(F("Mating..."));
    BDISPLAY::display.display();
}

/**
 * @brief Handles the left action button press.
 */
void CPMating::performLeftAction() {
    // nothing to do
    // this is a placeholder for the left button action
}

/**
 * @brief Handles the middle action button press.
 */
void CPMating::performMiddleAction() {
    // nothing to do
    // this is a placeholder for the middle button action
}

/**
 * @brief Handles the right action button press.
 */
void CPMating::performRightAction() {
    // nothing to do
    // this is a placeholder for the right button action
}

/**
 * @brief Handles the boot button action.
 */
void CPMating::performBootAction()
{
    doStop();
}

bool CPMating::isActive( void )
{
    return cpmating_state != CPMATING_STATE_t::CPMATING_DISPLAY_DISABLED;
}

void CPMating::doStart( void )
{
    // disable menu before starting
    BMenu::disableMenu();

    cpmating_state = CPMATING_STATE_t::CPMATING_DISPLAY_RUNNING;

    CPMating& mating = CPMating::getInstance();

    // Get the current life phase of the CyberPartner
    CyberPartner& partner = CyberPartner::getInstance();
    LifePhase currentPhase = partner.getLifePhase();

    // Check if the life phase is Adult or older
    if (currentPhase < LifePhase::Child) {
        showDisplayState(DisplayState::TooYoung); // Show "too young" message
        return; // Stop further execution
    }
    else if (currentPhase == LifePhase::Death) {
        showDisplayState(DisplayState::TooDead); // Show "too dead" message
        return; // Stop further execution
    }

    mating.showDisplayState(DisplayState::MeetAFriend);

    mating.scanButton();
}
void CPMating::doStop( void )
{
    cpmating_state = CPMATING_STATE_t::CPMATING_DISPLAY_DISABLED;

    displayTask.disable();
    taskRunner.deleteTask(displayTask);
    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.display();

    // enable menu since we are done
    BMenu::enableMenu();
}

void CPMating::displayMessage(const __FlashStringHelper* message) {
    BDISPLAY::display.setTextSize(1);
    BDISPLAY::display.setTextColor(WHITE);
    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.setCursor(0, 0);
    BDISPLAY::display.println();
    BDISPLAY::display.println(message);
    BDISPLAY::display.display();
}

void CPMating::showDisplayState(DisplayState state) {
    switch (state) {
        case DisplayState::TooYoung:
            //displayTask.disable();
            //taskRunner.deleteTask(displayTask);
            displayMessage(F("You are too young\nto play."));
            break;
        case DisplayState::TooDead:
            //displayTask.disable();
            //taskRunner.deleteTask(displayTask);
            displayMessage(F("You are too dead\nto play."));
            break;
        case DisplayState::MeetAFriend:
            displayTask.setCallback(animateCPMating);
            taskRunner.addTask(displayTask);
            displayTask.enable();
            break;
        case DisplayState::Masturbation:
            displayTask.disable();
            taskRunner.deleteTask(displayTask);
            displayMessage(F("You are using\nyour own button.\nStop it!\n\nPress escape."));
            break;
        case DisplayState::Mating:
            displayTask.disable();
            taskRunner.deleteTask(displayTask);
            displayMessage(F("Playing...\nSo Much Fun!\n\nPress escape."));
            break;
        default:
            displayTask.disable();
            taskRunner.deleteTask(displayTask);
            displayMessage(F("Unknown state."));
            break;
    }
}

void CPMating::scanButton( void )
{
    #ifdef CPMATINGDEBUG
    Serial.println(F("CPMATING: scanButton()"));
    #endif

    showDisplayState(DisplayState::MeetAFriend);

    BadgeOneWireMenu::registerButtonToBadge([](const std::array<uint8_t, 8>& buttonId, const std::array<uint8_t, 8>& masterKey) {
        // This code runs after mating is complete
        #ifdef CPMATINGDEBUG
        Serial.println(F("CPMating: Button information has been recieved."));
        #endif

        // Check if the button ID matches the one stored in EEPROM
        auto& mating = CPMating::getInstance();
        bEeprom& prom = bEeprom::getInstance();
        unsigned long restartTime = millis() + 1000;
        while (millis() < restartTime) {
            yield(); // Allow background tasks to run
        }

        JsonDocument matingDoc;
        if (buttonId == prom.readiButtonAddressFromEEPROM()) {
            #ifdef CPMATINGDEBUG
            Serial.println(F("CPMating: Visiting Button ID matches registered Button ID. You can't do that!"));
            #endif

            matingDoc["action"] = "masturbation";
            mating.showDisplayState(DisplayState::Masturbation);
        } else {
            matingDoc["action"] = "mating";
            mating.showDisplayState(DisplayState::Mating);
        }
        // Convert button ID to a hex string
        char buttonIdStr[17];
        for (int i = 0; i < 8; i++) {
            sprintf(&buttonIdStr[i * 2], "%02X", buttonId[i]);
        }

        matingDoc["buttonID"] = buttonIdStr;
        CyberPartner& partner = CyberPartner::getInstance();
        partner.queueGeneralUpdate("cp/mating", matingDoc);

    });
}
