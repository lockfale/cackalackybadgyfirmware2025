/**
 * @file ProgressConfig.h
 * @brief Manages the progress configuration for the badge.
 *
 * This file contains the declaration of the ProgressConfig class, which follows
 * a singleton pattern to ensure only one instance exists. It manages the progress
 * (registration, cyberpartner stats, challenge unlocks, etc) for the badge,
 * including loading and saving the configuration to a file, as well as initializing
 * the file system.
 */
#pragma once
#include <Arduino.h>
#include <string>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define PROGRESSDEBUG // uncomment to enable badge debugging
#endif

/**
 * @class ProgressConfig
 * @brief Manages the progress configuration for the badge.
 *
 * This class follows a singleton pattern to ensure only one instance exists.
 * It manages the progress (registration, cyberpartner stats, challenge unlocks, etc)
 * for the badge, including loading and saving the configuration to a file.
 * It also provides methods to reset the progress to default values.
 *
 * @note This class is not thread-safe.
 *
 * Example usage:
 * @code
 * ProgressConfig& config = ProgressConfig::getInstance();
 * config.loadProgressFile();
 * // Modify progress
 * config.saveProgressFile();
 * @endcode
 */
class ProgressConfig
{
public:
    /**
     * @brief Retrieves the singleton instance of the ProgressConfig class.
     *
     * This function provides access to the single instance of the `ProgressConfig` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `ProgressConfig` instance.
     *
     * @details
     * - The `ProgressConfig` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * ProgressConfig& matingInstance = ProgressConfig::getInstance();
     * matingInstance.doStart();
     * ```
     */
    static ProgressConfig& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    ProgressConfig(const ProgressConfig&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    ProgressConfig& operator=(const ProgressConfig&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    ProgressConfig(ProgressConfig&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    ProgressConfig& operator=(ProgressConfig&&) = delete;

    /**
     * @brief Loads the progress configuration from the file.
     * @return True if the configuration is successfully loaded, false otherwise.
     */
    static bool loadProgressFile();

    /**
     * @brief Saves the progress configuration to the file.
     * @return True if the configuration is successfully saved, false otherwise.
     */
    static bool saveProgressFile();

    /**
     * @brief Resets the progress configuration to default values.
     */
    static void resetProgress();

    /**
     * @brief Record that the badge has been registered.
     */
    static void registerBadge();

    /**
     * @brief Check to see if the badge has been registered.
     */
    static bool isBadgeRegistered();

    /**
     * @brief Record that the HelloWorld Challenge has been completed.
     */
    static void unlockHelloWorld();

    /**
     * @brief Record that the HelloWorld Challenge has not been completed.
     */
    static void lockHelloWorld();

    /**
     * @brief Check to see if the HelloWorld Challenge has been completed.
     */
    static bool isHelloWorldUnlocked();

    /**
     * @brief Record that the SU Challenge has been completed.
     */
    static void unlockSu();

    /**
     * @brief Record that the SU Challenge has not been completed.
     */
    static void lockSu();

    /**
     * @brief Check to see if the SU Challenge has been completed.
     */
    static bool isSuUnlocked();

    /**
     * @brief Record that the GotDropped Challenge has been completed.
     */
    static void unlockGotDropped();

    /**
     * @brief Record that the GotDropped Challenge has not been completed.
     */
    static void lockGotDropped();

    /**
     * @brief Check to see if the GotDropped Challenge has been completed.
     */
    static bool isGotDroppedUnlocked();

    /**
     * @brief Record that the GotStarved Challenge has been completed.
     */
    static void unlockGotStarved();

    /**
     * @brief Record that the GotStarved Challenge has not been completed.
     */
    static void lockGotStarved();

    /**
     * @brief Check to see if the GotStarved Challenge has been completed.
     */
    static bool isGotStarvedUnlocked();

    /**
     * @brief Record that the GotThirsty Challenge has been completed.
     */
    static void unlockGotThirsty();

    /**
     * @brief Record that the GotThirsty Challenge has not been completed.
     */
    static void lockGotThirsty();

    /**
     * @brief Check to see if the GotThirsty Challenge has been completed.
     */
    static bool isGotThirstyUnlocked();

    /**
     * @brief Record that the GotDepressed Challenge has been completed.
     */
    static void unlockGotDepressed();

    /**
     * @brief Record that the GotDepressed Challenge has not been completed.
     */
    static void lockGotDepressed();

    /**
     * @brief Check to see if the GotDepressed Challenge has been completed.
     */
    static bool isGotDepressedUnlocked();

    /**
     * @brief Record that the GotOld Challenge has been completed.
     */
    static void unlockGotOld();

    /**
     * @brief Record that the GotOld Challenge has not been completed.
     */
    static void lockGotOld();

    /**
     * @brief Check to see if the GotOld Challenge has been completed.
     */
    static bool isGotOldUnlocked();

    /**
     * @brief Record that the GotHeavy Challenge has been completed.
     */
    static void unlockGotHeavy();

    /**
     * @brief Record that the GotHeavy Challenge has not been completed.
     */
    static void lockGotHeavy();

    /**
     * @brief Check to see if the GotHeavy Challenge has been completed.
     */
    static bool isGotHeavyUnlocked();

    /**
     * @brief Record that the GotSkinny Challenge has been completed.
     */
    static void unlockGotSkinny();

    /**
     * @brief Record that the GotSkinny Challenge has not been completed.
     */
    static void lockGotSkinny();

    /**
     * @brief Check to see if the GotSkinny Challenge has been completed.
     */
    static bool isGotSkinnyUnlocked();

    /**
     * @brief Record that the Chilly Challenge has been completed.
     */
    static void unlockGotChilly();

    /**
     * @brief Record that the Chilly Challenge has not been completed.
     */
    static void lockGotChilly();

    /**
     * @brief Check to see if the Chilly Challenge has been completed.
     */
    static bool isGotChillyUnlocked();

    /**
     * @brief Record that the Sweaty Challenge has been completed.
     */
    static void unlockGotSweaty();

    /**
     * @brief Record that the Sweaty Challenge has not been completed.
     */
    static void lockGotSweaty();

    /**
     * @brief Check to see if the Sweaty Challenge has been completed.
     */
    static bool isGotSweatyUnlocked();

    /**
     * @brief Record that the Ogrady Challenge has been completed.
     */
    static void unlockBeatOgrady();

    /**
     * @brief Record that the Ogrady Challenge has not been completed.
     */
    static void lockBeatOgrady();

    /**
     * @brief Check to see if the Ogrady Challenge has been completed.
     */
    static bool isBeatOgradyUnlocked();

    /**
     * @brief Record that the Roulotto Challenge has been completed.
     */
    static void unlockBeatRoulotto();

    /**
     * @brief Record that the Roulotto Challenge has not been completed.
     */
    static void lockBeatRoulotto();

    /**
     * @brief Check to see if the Roulotto Challenge has been completed.
     */
    static bool isBeatRoulottoUnlocked();

    /**
     * @brief Record that the Spoof OneWire Challenge has been completed.
     */
    static void unlockD3adb33f();

    /**
     * @brief Record that the Spoof OneWire Challenge has not been completed.
     */
    static void lockD3adb33f();

    /**
     * @brief Check to see if the Spoof OneWire Challenge has been completed.
     */
    static bool isD3adb33fUnlocked();

    /**
     * @brief Record that the 900 Challenge has been completed.
     */
    static void unlockTonyHawk();

    /**
     * @brief Record that the 900 Challenge has not been completed.
     */
    static void lockTonyHawk();

    /**
     * @brief Check to see if the 900 Challenge has been completed.
     */
    static bool isTonyHawkUnlocked();

protected:
    /**
     * @brief Default destructor for the ProgressConfig class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~ProgressConfig() = default;

private:
    /**
     * @brief Default constructor for the ProgressConfig class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    ProgressConfig() = default;

    static constexpr const char *_progressfile = "/progress.json";
    static bool _badgeRegistered;
    static bool _helloworldUnlocked;
    static bool _suUnlocked;
    static bool _gotDroppedUnlocked;
    static bool _gotStarvedUnlocked;
    static bool _gotThirstyUnlocked;
    static bool _gotDepressedUnlocked;
    static bool _gotOldUnlocked;
    static bool _gotHeavyUnlocked;
    static bool _gotSkinnyUnlocked;
    static bool _gotChillyUnlocked;
    static bool _gotSweatyUnlocked;
    static bool _beatOgradyUnlocked;
    static bool _beatRoulottoUnlocked;
    static bool _d3adb33fUnlocked;
    static bool _tonyHawk;

    /**
     * @brief Report any unlocked achievements to the MQTT server.
     * @note This function should only be called when an achievement is unlocked.
     */
    static void reportAchievement(String achievementName);

    /**
     * @brief Helper function to initialize LittleFS.
     * @return True if initialization is successful, false otherwise.
     */
    static bool initFS(); // Helper function to initialize LittleFS
};