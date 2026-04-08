/**
 * @file ProgressConfig.cpp
 * @brief Implements the ProgressConfig class methods.
 * @note This file contains the implementation of the ProgressConfig class, which manages
 * any progress (registration, cyberpartner stats, challenge unlocks, etc) for the
 * badge. It includes methods for loading and saving the configuration to a file,
 * as well as initializing the file system.
 */
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "obfuscate.h"
#include "ProgressConfig.h"
#include <NetworkTaskManager.h>
#include "CryptoUtils.h"

namespace {

}

bool ProgressConfig::_badgeRegistered = false;
bool ProgressConfig::_helloworldUnlocked = false;
bool ProgressConfig::_suUnlocked = false;
bool ProgressConfig::_gotDroppedUnlocked = false;
bool ProgressConfig::_gotStarvedUnlocked = false;
bool ProgressConfig::_gotThirstyUnlocked = false;
bool ProgressConfig::_gotDepressedUnlocked = false;
bool ProgressConfig::_gotOldUnlocked = false;
bool ProgressConfig::_gotHeavyUnlocked = false;
bool ProgressConfig::_gotSkinnyUnlocked = false;
bool ProgressConfig::_gotChillyUnlocked = false;
bool ProgressConfig::_gotSweatyUnlocked = false;
bool ProgressConfig::_beatOgradyUnlocked = false;
bool ProgressConfig::_beatRoulottoUnlocked = false;
bool ProgressConfig::_d3adb33fUnlocked = false;
bool ProgressConfig::_tonyHawk = false;

/**
 * @brief Constructs the ProgressConfig object.
 * @note This is a private constructor to enforce the singleton pattern.
 */
//ProgressConfig::ProgressConfig() {}

ProgressConfig& ProgressConfig::getInstance() {
    static ProgressConfig instance;
    return instance;
}

/**
 * @brief Initializes the LittleFS file system.
 * @return true if initialization is successful, false otherwise.
 */
bool ProgressConfig::initFS() {
    static bool fsInitialized = false;
    if (!fsInitialized) {
        fsInitialized = LittleFS.begin();
        if (!fsInitialized) {
            Serial.println("Failed to mount LittleFS!");
        }
    }
    return fsInitialized;
}

/**
 * @brief Resets the progress configuration to default values.
 */
void ProgressConfig::resetProgress() {
    _badgeRegistered = false;
    _helloworldUnlocked = false;
    _suUnlocked = false;
    _gotDroppedUnlocked = false;
    _gotStarvedUnlocked = false;
    _gotThirstyUnlocked = false;
    _gotDepressedUnlocked = false;
    _gotOldUnlocked = false;
    _gotHeavyUnlocked = false;
    _gotSkinnyUnlocked = false;
    _gotChillyUnlocked = false;
    _gotSweatyUnlocked = false;
    _beatOgradyUnlocked = false;
    _beatRoulottoUnlocked = false;
    _d3adb33fUnlocked = false;
    _tonyHawk = false;

    Serial.print("Resetting+");
    saveProgressFile();
}

/**
 * @brief Loads the progress configuration from the file.
 * @return True if the configuration is successfully loaded, false otherwise.
 */
bool ProgressConfig::loadProgressFile() {
    #ifdef PROGRESSDEBUG
    Serial.println("ProgressConfig::loadProgressFile()");
    #endif

    JsonDocument doc;
    if (!CryptoUtils::loadEncryptedFile(_progressfile, doc, 0xD4E8A7C3)) {
        Serial.println("CPStore::loadStoreFile failed");
        saveProgressFile(); // Reset to defaults if loading fails
        return false;
    }

    _badgeRegistered = doc["badgeRegistered"].as<bool>() | false;
    _helloworldUnlocked = doc["helloworldUnlocked"].as<bool>() | false;
    _suUnlocked = doc["suUnlocked"].as<bool>() | false;
    _gotDroppedUnlocked = doc["gotDroppedUnlocked"].as<bool>() | false;
    _gotStarvedUnlocked = doc["gotStarvedUnlocked"].as<bool>() | false;
    _gotThirstyUnlocked = doc["gotThirstyUnlocked"].as<bool>() | false;
    _gotDepressedUnlocked = doc["gotDepressedUnlocked"].as<bool>() | false;
    _gotOldUnlocked = doc["gotOldUnlocked"].as<bool>() | false;
    _gotHeavyUnlocked = doc["gotHeavyUnlocked"].as<bool>() | false;
    _gotSkinnyUnlocked = doc["gotSkinnyUnlocked"].as<bool>() | false;
    _gotChillyUnlocked = doc["gotChillyUnlocked"].as<bool>() | false;
    _gotSweatyUnlocked = doc["gotSweatyUnlocked"].as<bool>() | false;
    _beatOgradyUnlocked = doc["beatOgradyUnlocked"].as<bool>() | false;
    _beatRoulottoUnlocked = doc["beatRoulottoUnlocked"].as<bool>() | false;
    _d3adb33fUnlocked = doc["d3adb33fUnlocked"].as<bool>() | false;
    _tonyHawk = doc["tonyHawk"].as<bool>() | false;

    return true;
}

/**
 * @brief Saves the progress configuration to the file.
 * @return True if the configuration is successfully saved, false otherwise.
 */
bool ProgressConfig::saveProgressFile() {
    #ifdef PROGRESSDEBUG
    Serial.println("Saving progress configuration file.");
    #endif

    JsonDocument doc;

    // Only save progress as long as it is not the default. Don't want to give away any secrets.
    if (_badgeRegistered) {
        doc["badgeRegistered"] = _badgeRegistered;
    }
    if (_helloworldUnlocked) {
        doc["helloworldUnlocked"] = _helloworldUnlocked;
    }
    if (_suUnlocked) {
        doc["suUnlocked"] = _suUnlocked;
    }
    if (_gotDroppedUnlocked) {
        doc["gotDroppedUnlocked"] = _gotDroppedUnlocked;
    }
    if (_gotStarvedUnlocked) {
        doc["gotStarvedUnlocked"] = _gotStarvedUnlocked;
    }
    if (_gotThirstyUnlocked) {
        doc["gotThirstyUnlocked"] = _gotThirstyUnlocked;
    }
    if (_gotDepressedUnlocked) {
        doc["gotDepressedUnlocked"] = _gotDepressedUnlocked;
    }
    if (_gotOldUnlocked) {
        doc["gotOldUnlocked"] = _gotOldUnlocked;
    }
    if (_gotHeavyUnlocked) {
        doc["gotHeavyUnlocked"] = _gotHeavyUnlocked;
    }
    if (_gotSkinnyUnlocked) {
        doc["gotSkinnyUnlocked"] = _gotSkinnyUnlocked;
    }
    if (_gotChillyUnlocked) {
        doc["gotChillyUnlocked"] = _gotChillyUnlocked;
    }
    if (_gotSweatyUnlocked) {
        doc["gotSweatyUnlocked"] = _gotSweatyUnlocked;
    }
    if (_beatOgradyUnlocked) {
        doc["beatOgradyUnlocked"] = _beatOgradyUnlocked;
    }
    if (_beatRoulottoUnlocked) {
        doc["beatRoulottoUnlocked"] = _beatRoulottoUnlocked;
    }
    if (_d3adb33fUnlocked) {
        doc["d3adb33fUnlocked"] = _d3adb33fUnlocked;
    }
    if (_tonyHawk) {
        doc["tonyHawk"] = _tonyHawk;
    }

    return CryptoUtils::saveEncryptedFile(_progressfile, doc, 0xD4E8A7C3);
}

/**
 * @brief Registers the badge.
 * @note This method registers the badge and updates the status accordingly.
 */
void ProgressConfig::registerBadge() {
    _badgeRegistered = true;
}

/**
 * @brief Checks if the badge is registered.
 * @note This method checks if the badge is registered and updates the status accordingly.
 */
bool ProgressConfig::isBadgeRegistered() {
    return _badgeRegistered;
}

/**
 * @brief Record that the HelloWorld Challenge has been completed.
 */
void ProgressConfig::unlockHelloWorld() {
    _helloworldUnlocked = true;
    reportAchievement("helloWorld");
}

/**
 * @brief Record that the HelloWorld Challenge has not been completed.
 */
void ProgressConfig::lockHelloWorld() {
    _helloworldUnlocked = false;
}

/**
 * @brief Check to see if the HelloWorld Challenge has been completed.
 */
bool ProgressConfig::isHelloWorldUnlocked() {
    return _helloworldUnlocked;
}

/**
 * @brief Record that the SU Challenge has been completed.
 */
void ProgressConfig::unlockSu() {
    _suUnlocked = true;
    reportAchievement("su");
}

/**
 * @brief Record that the SU Challenge has not been completed.
 */
void ProgressConfig::lockSu() {
    _suUnlocked = false;
}

/**
 * @brief Check to see if the SU Challenge has been completed.
 */
bool ProgressConfig::isSuUnlocked() {
    return _suUnlocked;
}

/**
 * @brief Record that the GotDropped Challenge has been completed.
 */
void ProgressConfig::unlockGotDropped() {
    _gotDroppedUnlocked = true;
    reportAchievement("gotDropped");
}

/**
 * @brief Record that the GotDropped Challenge has not been completed.
 */
void ProgressConfig::lockGotDropped() {
    _gotDroppedUnlocked = false;
}

/**
 * @brief Check to see if the GotDropped Challenge has been completed.
 */
bool ProgressConfig::isGotDroppedUnlocked() {
    return _gotDroppedUnlocked;
}

/**
 * @brief Record that the GotStarved Challenge has been completed.
 */
void ProgressConfig::unlockGotStarved() {
    _gotStarvedUnlocked = true;
    reportAchievement("gotStarved");
}

/**
 * @brief Record that the GotStarved Challenge has not been completed.
 */
void ProgressConfig::lockGotStarved() {
    _gotStarvedUnlocked = false;
}

/**
 * @brief Check to see if the GotStarved Challenge has been completed.
 */
bool ProgressConfig::isGotStarvedUnlocked() {
    return _gotStarvedUnlocked;
}

/**
 * @brief Record that the GotThirsty Challenge has been completed.
 */
void ProgressConfig::unlockGotThirsty() {
    _gotThirstyUnlocked = true;
    reportAchievement("gotThirsty");
}

/**
 * @brief Record that the GotThirsty Challenge has not been completed.
 */
void ProgressConfig::lockGotThirsty() {
    _gotThirstyUnlocked = false;
}

/**
 * @brief Check to see if the GotThirsty Challenge has been completed.
 */
bool ProgressConfig::isGotThirstyUnlocked() {
    return _gotThirstyUnlocked;
}

/**
 * @brief Record that the GotDepressed Challenge has been completed.
 */
void ProgressConfig::unlockGotDepressed() {
    _gotDepressedUnlocked = true;
    reportAchievement("gotDepressed");
}

/**
 * @brief Record that the GotDepressed Challenge has not been completed.
 */
void ProgressConfig::lockGotDepressed() {
    _gotDepressedUnlocked = false;
}

/**
 * @brief Check to see if the GotDepressed Challenge has been completed.
 */
bool ProgressConfig::isGotDepressedUnlocked() {
    return _gotDepressedUnlocked;
}

/**
 * @brief Record that the GotOld Challenge has been completed.
 */
void ProgressConfig::unlockGotOld() {
    _gotDepressedUnlocked = true;
    reportAchievement("gotOld");
}

/**
 * @brief Record that the GotOld Challenge has not been completed.
 */
void ProgressConfig::lockGotOld() {
    _gotOldUnlocked = false;
}

/**
 * @brief Check to see if the GotOld Challenge has been completed.
 */
bool ProgressConfig::isGotOldUnlocked() {
    return _gotOldUnlocked;
}

/**
 * @brief Record that the GotHeavy Challenge has been completed.
 */
void ProgressConfig::unlockGotHeavy() {
    _gotHeavyUnlocked = true;
    reportAchievement("gotHeavy");
}

/**
 * @brief Record that the GotHeavy Challenge has not been completed.
 */
void ProgressConfig::lockGotHeavy() {
    _gotHeavyUnlocked = false;
}

/**
 * @brief Check to see if the GotHeavy Challenge has been completed.
 */
bool ProgressConfig::isGotHeavyUnlocked() {
    return _gotHeavyUnlocked;
}

/**
 * @brief Record that the GotSkinny Challenge has been completed.
 */
void ProgressConfig::unlockGotSkinny() {
    _gotSkinnyUnlocked = true;
    reportAchievement("gotSkinny");
}

/**
 * @brief Record that the GotSkinny Challenge has not been completed.
 */
void ProgressConfig::lockGotSkinny() {
    _gotSkinnyUnlocked = false;
}

/**
 * @brief Check to see if the GotSkinny Challenge has been completed.
 */
bool ProgressConfig::isGotSkinnyUnlocked() {
    return _gotSkinnyUnlocked;
}

/**
 * @brief Record that the GotChilly Challenge has been completed.
 */
void ProgressConfig::unlockGotChilly() {
    _gotChillyUnlocked = true;
    reportAchievement("gotChilly");
}

/**
 * @brief Record that the GotChilly Challenge has not been completed.
 */
void ProgressConfig::lockGotChilly() {
    _gotChillyUnlocked = false;
}

/**
 * @brief Check to see if the GotChilly Challenge has been completed.
 */
bool ProgressConfig::isGotChillyUnlocked() {
    return _gotChillyUnlocked;
}

/**
 * @brief Record that the GotSweaty Challenge has been completed.
 */
void ProgressConfig::unlockGotSweaty() {
    _gotSweatyUnlocked = true;
    reportAchievement("gotSweaty");
}

/**
 * @brief Record that the GotSweaty Challenge has not been completed.
 */
void ProgressConfig::lockGotSweaty() {
    _gotSweatyUnlocked = false;
}

/**
 * @brief Check to see if the GotSweaty Challenge has been completed.
 */
bool ProgressConfig::isGotSweatyUnlocked() {
    return _gotSweatyUnlocked;
}

/**
 * @brief Record that the BeatOgrady Challenge has been completed.
 */
void ProgressConfig::unlockBeatOgrady() {
    _beatOgradyUnlocked = true;
    reportAchievement("beatOgrady");
}

/**
 * @brief Record that the BeatOgrady Challenge has not been completed.
 */
void ProgressConfig::lockBeatOgrady() {
    _beatOgradyUnlocked = false;
}

/**
 * @brief Check to see if the BeatOgrady Challenge has been completed.
 */
bool ProgressConfig::isBeatOgradyUnlocked() {
    return _beatOgradyUnlocked;
}

/**
 * @brief Record that the Roulotto Challenge has been completed.
 */
void ProgressConfig::unlockBeatRoulotto() {
    _beatRoulottoUnlocked = true;
    reportAchievement("beatRoulotto");
}

/**
 * @brief Record that the Roulotto Challenge has not been completed.
 */
void ProgressConfig::lockBeatRoulotto() {
    _beatRoulottoUnlocked = false;
}

/**
 * @brief Check to see if the Roulotto Challenge has been completed.
 */
bool ProgressConfig::isBeatRoulottoUnlocked() {
    return _beatRoulottoUnlocked;
}

/**
 * @brief Record that the Spoof OneWire Challenge has been completed.
 */
void ProgressConfig::unlockD3adb33f() {
    _d3adb33fUnlocked = true;
    reportAchievement("d3adb33f");
}

/**
 * @brief Record that the Spoof OneWire Challenge has not been completed.
 */
void ProgressConfig::lockD3adb33f() {
    _d3adb33fUnlocked = false;
}

/**
 * @brief Check to see if the Spoof OneWire Challenge has been completed.
 */
bool ProgressConfig::isD3adb33fUnlocked() {
    return _d3adb33fUnlocked;
}

/**
 * @brief Record that the 900 Challenge has been completed.
 */
void ProgressConfig::unlockTonyHawk() {
    _tonyHawk = true;
    reportAchievement("tonyHawk");
}

/**
 * @brief Record that the 900 Challenge has not been completed.
 */
void ProgressConfig::lockTonyHawk() {
    _tonyHawk = false;
}

/**
 * @brief Check to see if the 900 Challenge has been completed.
 */
bool ProgressConfig::isTonyHawkUnlocked() {
    return _tonyHawk;
}

/**
 * @brief Report any unlocked achievements to the MQTT server.
 * @param achievement The name of the achievement to report.
 * @note This function should only be called when an achievement is unlocked.
 */
void ProgressConfig::reportAchievement(String achievement) {
    JsonDocument doc;
    doc["ts"] = time(nullptr);    // Add a timestamp to the JSON document

    // Serialize the JSON document to a character buffer
    static char payload[64]; // Adjust size as needed
    serializeJson(doc, payload, sizeof(payload));
    String endpoint = "cp/achmnt/" + achievement;
    NetworkTaskManager::getInstance().queueReceivedEventTask(endpoint, payload);
}