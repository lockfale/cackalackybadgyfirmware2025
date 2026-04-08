/**
 * @file registration.cpp
 * @brief Implements the registration class methods.
 *
 * This file contains the logic for handling registration
 */
#include <Arduino.h>
#include "badge_display.h"
#include "badge_eeprom.h"
#include "badgeMenu.h"
#include "neocontroller.h"
#include "registration.h"
#include "ProgressConfig.h"
#include "badgeOneWireMenu.h"
#include "cyberPartnerGame.h"
#include "NetworkTaskManager.h"

Registration::REGISTRATION_STATE_t Registration::reg_state = Registration::REGISTRATION_STATE_t::REGISTRATION_DISABLED;
bool Registration::isReg = false;

constexpr size_t BUTTON_ID_SIZE = 8;
std::array<uint8_t, BUTTON_ID_SIZE> Registration::buttonID = {0};
std::array<uint8_t, BUTTON_ID_SIZE> Registration::masterKey = {0};

Registration& Registration::getInstance() {
    static Registration instance;
    return instance;
}

bool Registration::isActive( void )
{
    return reg_state != REGISTRATION_STATE_t::REGISTRATION_DISABLED;
}

bool Registration::isRegistered()
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: isRegistered()"));
    #endif

    return isReg;
}

void Registration::setButtonID(const std::array<uint8_t, 8>& key)
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: setButtonID()"));
    #endif
    buttonID = key;
}

std::array<uint8_t, 8> Registration::getButtonID()
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: getButtonID()"));
    #endif
    return buttonID;
}

void Registration::setMasterKey(const std::array<uint8_t, 8>& key)
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: setMasterKey()"));
    #endif
    masterKey = key;
}

std::array<uint8_t, 8> Registration::getMasterKey()
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: getMasterKey()"));
    #endif
    return masterKey;
}

void Registration::doStart( void )
{
    // disable menu before starting
    BMenu::disableMenu();

    reg_state = REGISTRATION_STATE_t::REGISTRATION_RUNNING;

    bEeprom& prom = bEeprom::getInstance();
    if(!prom.isBadgeRegistered()) {
        registerBadge();
    } else {
        verifyButton();
    }
}

void Registration::doStop( void )
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: doStop()"));
    #endif

    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.display();

    // Store the current state in a temporary variable
    auto currentState = reg_state;

    // Disable Registration
    reg_state = REGISTRATION_STATE_t::REGISTRATION_DISABLED;

    if (currentState == REGISTRATION_STATE_t::DISCORD_REGISTRATION) {
        BMenu::enableMenu();
        return;
    }

    // Now that we are registered, we can start the CyberPartnerGame
    CyberPartnerGame::getInstance().start();
}

void Registration::unRegisterBadge( void )
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: unRegisterBadge()"));
    #endif
    bEeprom::getInstance().unRegisterBadge();
}

void Registration::registerBadge( void )
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: registerBadge()"));
    #endif
    showDisplayState(DisplayState::NeedsRegistration);

    BadgeOneWireMenu::registerButtonToBadge([](const std::array<uint8_t, 8>& buttonId, const std::array<uint8_t, 8>& masterKey) {
        // This code runs after registration is complete
        #ifdef REGISTRATIONDEBUG
        Serial.println(F("REGISTRATION: Badge registration complete."));
        #endif

        // Save the button ID and master key
        Registration& reg = Registration::getInstance();
        reg.setButtonID(buttonId);
        reg.setMasterKey(masterKey);

        // Write the button ID to EEPROM
        bEeprom& prom = bEeprom::getInstance();
        prom.writeiButtonAddressToEEPROM(buttonId);
        prom.registerBadge();

        // Convert button ID to a hex string
        char buttonIdStr[17];
        for (int i = 0; i < 8; i++) {
            sprintf(&buttonIdStr[i * 2], "%02X", buttonId[i]);
        }

        // Convert master key to a hex string
        char masterKeyHex[8 * 2 + 1];
        for (size_t i = 0; i < 8; i++) {
            sprintf(&masterKeyHex[i * 2], "%02X", masterKey[i]);
        }
        masterKeyHex[8 * 2] = '\0';

        // Create the payload string
        char payload[64];
        snprintf(payload, sizeof(payload), "%s|%s", buttonIdStr, masterKeyHex);

        // Output the size of the concatenated string
        #ifdef REGISTRATIONDEBUG
        Serial.print("Payload Size: ");
        Serial.println(strlen(payload)); // Print the number of characters in the payload

        Serial.print("Payload: ");
        Serial.println(payload); // Print the number of characters in the payload
        #endif

        // TODO: Set the buttonId and masterKey as username/password for the MQTT broker

        // Queue the payload for the network task manager
        NetworkTaskManager::getInstance().queueReceivedEventTask("register", payload);

        // Proceed to the next step
        reg.showDisplayState(DisplayState::FinishedRegistration);
        isReg = true;
    });
}

void Registration::displayMessage(const __FlashStringHelper* message) {
    BDISPLAY::display.setTextSize(1);
    BDISPLAY::display.setTextColor(WHITE);
    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.setCursor(0, 0);
    BDISPLAY::display.println();
    BDISPLAY::display.println(message);
    BDISPLAY::display.display();
}

void Registration::showDisplayState(DisplayState state) {
    switch (state) {
        case DisplayState::NeedsRegistration:
            displayMessage(F("Badge not registered\n\nPress your iButton\nagainst your badge."));
            break;
        case DisplayState::FinishedRegistration:
            displayMessage(F("Badge registered\n\nPress any key\nto continue."));
            break;
        case DisplayState::ShowAlreadyRegistered:
            displayMessage(F("This badge is \n  already registered.\n\nVerify your iButton\nagainst your badge."));
            break;
        case DisplayState::VerificationFailed:
            displayMessage(F("Verification failed.\n\nRestarting."));
            break;
        case DisplayState::FinishVerify:
            displayMessage(F("iButton verified.\n\nWelcome back.\n\nPress any key\nto continue."));
            break;
        default:
            displayMessage(F("Unknown state."));
            break;
    }
}

void Registration::verifyButton( void )
{
    #ifdef REGISTRATIONDEBUG
    Serial.println(F("REGISTRATION: verifyButton()"));
    #endif

    showDisplayState(DisplayState::ShowAlreadyRegistered);

    BadgeOneWireMenu::registerButtonToBadge([](const std::array<uint8_t, 8>& buttonId, const std::array<uint8_t, 8>& masterKey) {
        // This code runs after registration is complete
        #ifdef REGISTRATIONDEBUG
        Serial.println(F("REGISTRATION: Button verification complete."));
        #endif

        // Check if the button ID matches the one stored in EEPROM
        auto& reg = Registration::getInstance();
        bEeprom& prom = bEeprom::getInstance();
        if (buttonId != prom.readiButtonAddressFromEEPROM()) {
            #ifdef REGISTRATIONDEBUG
            Serial.println(F("REGISTRATION: Button ID does not match."));
            #endif
            reg.showDisplayState(DisplayState::VerificationFailed);

            unsigned long restartTime = millis() + 5000;
            while (millis() < restartTime) {
                yield(); // Allow background tasks to run
            }
            ESP.restart();
        }
        // Save the button ID and master key
        reg.setButtonID(buttonId);
        reg.setMasterKey(masterKey);

        JsonDocument queueDoc;
        CyberPartner::getInstance().queueGeneralUpdate("state/get", queueDoc);


        // Proceed to the next step
        reg.showDisplayState(DisplayState::FinishVerify);
        isReg = true;
    });
}

void Registration::discordRegistration() {
    // disable menu before starting
    BMenu::disableMenu();
    NeoController::getInstance().changeGreen();

    reg_state = REGISTRATION_STATE_t::DISCORD_REGISTRATION;
    BDISPLAY::clearAndDisplayBatteryPercentage();
    BDISPLAY::display.setCursor(0, 10);
    BDISPLAY::display.setTextSize(1);

    BDISPLAY::display.println("Discord Registration\n");
    BDISPLAY::display.println("Use code for\nregistration\n");

    constexpr size_t DEVICE_ADDRESS_SIZE = 8;
    // Display the device address on the badge display
    for (int i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
        if (buttonID[i] < 16) {
            BDISPLAY::display.print("0");
        }
        BDISPLAY::display.print(buttonID[i], HEX);

        // Add a space after every 4 characters, except at the end
        if ((i + 1) % 2 == 0 && i < DEVICE_ADDRESS_SIZE - 1) {
            BDISPLAY::display.print(" ");
        }
    }
    BDISPLAY::display.display();
}