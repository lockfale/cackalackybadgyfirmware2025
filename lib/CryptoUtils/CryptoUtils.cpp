/**
 * @file CryptoUtils.cpp
 * @brief Implements the CryptoUtils class methods.
 *
 */

#include "CryptoUtils.h"
#include <Crypto.h>
#include <AES.h>
#include <SHA256.h>
#include <CTR.h>
#include <LittleFS.h>
#include "accelerometer.h"

#define AES_BLOCK_SIZE 16
#define MAX_PLAINTEXT_LEN 512

void CryptoUtils::deriveKey(uint8_t* key, uint32_t fileConstant) {
    uint32_t chipId = ESP.getChipId();
    uint8_t input[8];
    memcpy(input, &chipId, 4);
    memcpy(input + 4, &fileConstant, 4);

    // Create an instance of the SHA256 class
    SHA256 sha256;

    // Feed the input data into the hash function
    sha256.update(input, sizeof(input));

    // Finalize the hash and store the result
    uint8_t hash[32];
    sha256.finalize(hash, sizeof(hash));

    // Use the first 16 bytes of the hash as the key
    memcpy(key, hash, AES_BLOCK_SIZE);
}

void CryptoUtils::generateRandomIV(uint8_t* iv, size_t length) {
    // Read accelerometer data
    BadgeAcceler& accelInstance = BadgeAcceler::getInstance();
    int16_t x = accelInstance.readFloatAccelX();
    int16_t y = accelInstance.readFloatAccelY();
    int16_t z = accelInstance.readFloatAccelZ();

    // Combine accelerometer readings to generate a seed
    uint32_t seed = (uint32_t)x ^ ((uint32_t)y << 10) ^ ((uint32_t)z << 20);
    randomSeed(seed);

    for (size_t i = 0; i < length; i++) {
        iv[i] = random(0, 256);
    }
}

bool CryptoUtils::hasSynced() {
    // when badge hasn't synced it's time
    if (time(nullptr) < 1742444909) {
        return false;
    }
    return true;
}

bool CryptoUtils::saveEncryptedFile(const char* filePath, JsonDocument& doc, uint32_t fileConstant, bool forceSave) {
    #ifdef CRYPTOUTILSDEBUG
    Serial.println(F("saveEncryptedFile(): Saving encrypted file..."));
    #endif

    if (!forceSave && !hasSynced()) {
        Serial.println(F("saveEncryptedFile(): Badge hasn't synced, not saving."));
        return false;
    }

    File file = LittleFS.open(filePath, "w");
    if (!file) {
        Serial.println(F("Failed to open file for writing."));
        return false;
    }

    // Serialize the JSON document to a character buffer
    static char jsonString[MAX_PLAINTEXT_LEN]; // Adjust size as needed
    serializeJson(doc, jsonString, sizeof(jsonString));
    size_t jsonStringLen = strlen(jsonString);

    CTR<AESTiny128> ctr;

    uint8_t derivedKey[AES_BLOCK_SIZE];
    deriveKey(derivedKey, fileConstant);

    uint8_t iv[AES_BLOCK_SIZE];
    generateRandomIV(iv, sizeof(iv));

    #ifdef CRYPTOUTILSDEBUG
    Serial.println("saveEncryptedFile(): Key: (encryption)");
    for (size_t i = 0; i < AES_BLOCK_SIZE; i++) {
        Serial.printf("%02X ", derivedKey[i]);
    }
    Serial.println();

    Serial.println("saveEncryptedFile(): IV: (encryption)");
    for (size_t i = 0; i < AES_BLOCK_SIZE; i++) {
        Serial.printf("%02X ", iv[i]);
    }
    Serial.println();
    #endif

    ctr.setKey(derivedKey, sizeof(derivedKey));
    ctr.setIV(iv, sizeof(iv));

    uint8_t encrypted[MAX_PLAINTEXT_LEN];
    ctr.encrypt(encrypted, (const uint8_t*)jsonString, jsonStringLen);

    file.write(iv, sizeof(iv));

    #ifdef CRYPTOUTILSDEBUG
    Serial.println("saveEncryptedFile(): Encrypted data:");
    for (size_t i = 0; i < jsonStringLen; i++) {
        Serial.printf("%02X ", encrypted[i]);
    }
    Serial.println();
    #endif
    file.write(encrypted, jsonStringLen);
    file.close();

    #ifdef CRYPTOUTILSDEBUG
    Serial.println("saveEncryptedFile(): File contents after saving:");
    file = LittleFS.open(filePath, "r");
    if (file) {
        while (file.available()) {
            Serial.printf("%02X ", file.read());
        }
        file.close();
    }
    Serial.println();
    #endif

    return true;
}

bool CryptoUtils::loadEncryptedFile(const char* filePath, JsonDocument& doc, uint32_t fileConstant) {
    #ifdef CRYPTOUTILSDEBUG
    Serial.println(F("loadEncryptedFile(): Loading encrypted file..."));
    Serial.println("File contents before loading:");
    File filetmp = LittleFS.open(filePath, "r");
    if (filetmp) {
        while (filetmp.available()) {
            Serial.printf("%02X ", filetmp.read());
        }
        filetmp.close();
    }
    Serial.println();
    #endif

    File file = LittleFS.open(filePath, "r");
    if (!file) {
        Serial.println(F("Failed to open file for reading."));
        return false;
    }

    size_t totalSize = file.size();
    if (totalSize < AES_BLOCK_SIZE || totalSize > AES_BLOCK_SIZE + MAX_PLAINTEXT_LEN) {
        Serial.println(F("Invalid file size."));
        file.close();
        return false;
    }

    uint8_t iv[AES_BLOCK_SIZE];
    if (file.read(iv, sizeof(iv)) != sizeof(iv)) {
        Serial.println(F("Failed to read IV."));
        file.close();
        return false;
    }

    #ifdef CRYPTOUTILSDEBUG
    Serial.println("loadEncryptedFile(): IV (decryption):");
    for (size_t i = 0; i < AES_BLOCK_SIZE; i++) {
        Serial.printf("%02X ", iv[i]);
    }
    Serial.println();
    #endif

    size_t cipherLen = totalSize - AES_BLOCK_SIZE;
    uint8_t encrypted[MAX_PLAINTEXT_LEN];

    if (file.read(encrypted, cipherLen) != (int)cipherLen) {
        Serial.println(F("Failed to read encrypted data."));
        file.close();
        return false;
    }
    file.close();

    #ifdef CRYPTOUTILSDEBUG
    Serial.println("loadEncryptedFile(): Encrypted data (decryption):");
    for (size_t i = 0; i < cipherLen; i++) {
        Serial.printf("%02X ", encrypted[i]);
    }
    Serial.println();
    #endif

    CTR<AESTiny128> ctr;

    uint8_t derivedKey[16];
    deriveKey(derivedKey, fileConstant);

    #ifdef CRYPTOUTILSDEBUG
    Serial.println("loadEncryptedFile(): Key (decryption):");
    for (size_t i = 0; i < AES_BLOCK_SIZE; i++) {
        Serial.printf("%02X ", derivedKey[i]);
    }
    Serial.println();
    #endif

    uint8_t decrypted[MAX_PLAINTEXT_LEN + 1];
    ctr.setKey(derivedKey, sizeof(derivedKey));
    ctr.setIV(iv, sizeof(iv));
    ctr.decrypt(decrypted, encrypted, cipherLen);
    decrypted[cipherLen] = '\0';

    #ifdef CRYPTOUTILSDEBUG
    Serial.println("loadEncryptedFile(): Decrypted data (decryption):");
    for (size_t i = 0; i < cipherLen; i++) {
        Serial.printf("%c", decrypted[i]);
    }
    Serial.println();

    Serial.println("loadEncryptedFile(): Decrypted data in hex (decryption):");
    for (size_t i = 0; i < cipherLen; i++) {
        Serial.printf("%02X ", decrypted[i]);
    }
    Serial.println();
    #endif

    DeserializationError error = deserializeJson(doc, (char*)decrypted, cipherLen);

    if (error) {
        Serial.println(F("loadEncryptedFile(): Failed to parse JSON."));
        return false;
    }

    return true;
}