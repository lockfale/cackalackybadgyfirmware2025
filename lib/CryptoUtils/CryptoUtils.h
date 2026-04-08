/**
 * @file CryptoUtils.h
 * @brief Defines the CryptoUtils class
 */
#pragma once

#include <ArduinoJson.h>

//#define CRYPTOUTILSDEBUG // uncomment to enable CryptoUtil debugging

class CryptoUtils {
public:
    static void deriveKey(uint8_t* key, uint32_t fileConstant);
    static void generateRandomIV(uint8_t* iv, size_t length);
    static bool saveEncryptedFile(const char* filePath, JsonDocument& doc, uint32_t fileConstant, bool forceSave = false);
    static bool loadEncryptedFile(const char* filePath, JsonDocument& doc, uint32_t fileConstant);
    static bool hasSynced();
};
