/**
 * @file badge_eeprom.cpp
 * @brief Implements the bEeprom class methods.
 *
 * This file contains the logic for handling bEeprom
 */
#include "badge_eeprom.h"
#include <EEPROM.h>

constexpr size_t EEPROM_SIZE = 512; // Define the size of the EEPROM

constexpr size_t BADGE_REGISTERED_START_ADDRESS = 1; // Define the start address for badge registered field
constexpr size_t BADGE_REGISTERED_SIZE = 8; // Define the size of the badge registered field
constexpr uint8_t BADGE_REGISTERED_MAGIC[8] = {0xA5, 0x3C, 0x7F, 0xD2, 0xE8, 0x91, 0x4B, 0x6A};

constexpr size_t REGISTERED_IBUTTON_START_ADDRESS = BADGE_REGISTERED_START_ADDRESS + BADGE_REGISTERED_SIZE; // Start after the magic value
constexpr size_t IBUTTON_ADDRESS_SIZE = 8; // Define the size of the iButton Address
constexpr uint8_t IBUTTON_OBFUSCATION_KEY[8] = {0x4E, 0x92, 0x1F, 0xA7, 0xD3, 0x6C, 0x58, 0xB4};

bEeprom& bEeprom::getInstance() {
    static bEeprom instance;
    return instance;
}

bool bEeprom::setup() {
    EEPROM.begin(EEPROM_SIZE);

    // Write a test value
    EEPROM.write(0, 42);
    EEPROM.commit();

    // Read the test value back
    uint8_t testValue = EEPROM.read(0);
    if (testValue == 42) {
        #ifdef EEPROMDEBUG
        Serial.println(F("EEPROM: Initialization successful"));
        #endif
        return true;
    } else {
        Serial.println(F("EEPROM: Initialization failed"));
        return false;
    }
}

void bEeprom::unRegisterBadge() {
    for (int i = 0; i < BADGE_REGISTERED_SIZE; i++) {
        EEPROM.write(BADGE_REGISTERED_START_ADDRESS + i, 0x00); // Clear the magic bytes
    }
    EEPROM.commit(); // Save changes to flash memory
    #ifdef EEPROMDEBUG
    Serial.println("Badge registration magic bytes written to EEPROM.");
    #endif
}

void bEeprom::registerBadge() {
    for (int i = 0; i < BADGE_REGISTERED_SIZE; i++) {
        EEPROM.write(BADGE_REGISTERED_START_ADDRESS + i, BADGE_REGISTERED_MAGIC[i]);
    }
    EEPROM.commit(); // Save changes to flash memory
    #ifdef EEPROMDEBUG
    Serial.println("Badge registration magic bytes written to EEPROM.");
    #endif
}

bool bEeprom::isBadgeRegistered() {
    uint8_t readBytes[BADGE_REGISTERED_SIZE];
    for (int i = 0; i < BADGE_REGISTERED_SIZE; i++) {
        readBytes[i] = EEPROM.read(BADGE_REGISTERED_START_ADDRESS + i);
        if (readBytes[i] != BADGE_REGISTERED_MAGIC[i]) {
            #ifdef EEPROMDEBUG
            Serial.println("Badge is not registered.");
            #endif
            return false; // Magic bytes do not match
        }
    }

    #ifdef EEPROMDEBUG
    Serial.println("Badge is registered.");
    #endif
    return true; // Magic bytes match
}

// Function to write the device address to EEPROM
void bEeprom::writeiButtonAddressToEEPROM(const std::array<uint8_t, 8>& address) {
    for (int i = 0; i < IBUTTON_ADDRESS_SIZE; i++) {
        // XOR the iButton address with the obfuscation key
        uint8_t obfuscatedByte = address[i] ^ IBUTTON_OBFUSCATION_KEY[i];
        EEPROM.write(REGISTERED_IBUTTON_START_ADDRESS + i, obfuscatedByte);
    }

    #ifdef EEPROMDEBUG
    debugPrintObfuscatediButtonAddress();
    #endif

    EEPROM.commit(); // Save changes to flash memory
    Serial.println("Obfuscated iButton address written to EEPROM.");
}

std::array<uint8_t, 8> bEeprom::readiButtonAddressFromEEPROM() {
    std::array<uint8_t, 8> address;

    for (int i = 0; i < IBUTTON_ADDRESS_SIZE; i++) {
        // Read the obfuscated byte from EEPROM
        uint8_t obfuscatedByte = EEPROM.read(REGISTERED_IBUTTON_START_ADDRESS + i);
        // XOR the obfuscated byte with the obfuscation key to deobfuscate
        address[i] = obfuscatedByte ^ IBUTTON_OBFUSCATION_KEY[i];
    }

    Serial.println("Deobfuscated iButton address read from EEPROM.");
    return address;
}

void bEeprom::debugPrintObfuscatediButtonAddress() {
    Serial.print("Obfuscated iButton Address in EEPROM: ");
    for (int i = 0; i < IBUTTON_ADDRESS_SIZE; i++) {
        // Read the raw obfuscated byte from EEPROM
        uint8_t obfuscatedByte = EEPROM.read(REGISTERED_IBUTTON_START_ADDRESS + i);

        // Print the byte in hexadecimal format
        if (obfuscatedByte < 16) {
            Serial.print("0"); // Add leading zero for single-digit hex values
        }
        Serial.print(obfuscatedByte, HEX);

        // Add a colon between bytes, but not after the last byte
        if (i < IBUTTON_ADDRESS_SIZE - 1) {
            Serial.print(":");
        }
    }
    Serial.println(); // End the line after printing all bytes
}