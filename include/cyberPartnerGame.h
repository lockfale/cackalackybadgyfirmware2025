/**
 * @file cyberPartnerGame.h
 * @brief Declares the CyberPartnerGame class methods.
 *
 * This file contains the declaration of the CyberPartnerGame class, which manages
 * the game logic, animations, and display updates for the CyberPartner game.
 */
#pragma once

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>
#include <functional>
#include "CyberPartner.h"
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define CPGAMEDEBUG // uncomment to enable CPGame debugging
#endif

/**
 * @class CyberPartnerGame
 * @brief Manages the CyberPartner game logic and display.
 *
 * This class handles the game logic, animations, and display updates for the
 * CyberPartner game. It uses the TaskScheduler library to manage tasks and
 * animations.
 */
class CyberPartnerGame {

public:
    /**
     * @brief Retrieves the singleton instance of the CyberPartnerGame class.
     *
     * This function provides access to the single instance of the `CyberPartnerGame` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `CyberPartnerGame` instance.
     *
     * @details
     * - The `CyberPartnerGame` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * CyberPartnerGame& cpgameInstance = CyberPartnerGame::getInstance();
     * cpgameInstance.doStart();
     * ```
     */
    static CyberPartnerGame& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    CyberPartnerGame(const CyberPartnerGame&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    CyberPartnerGame& operator=(const CyberPartnerGame&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    CyberPartnerGame(CyberPartnerGame&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    CyberPartnerGame& operator=(CyberPartnerGame&&) = delete;

    /**
     * @brief Enters the CyberPartner game.
     *
     * This function initializes the game and starts the necessary tasks.
     */
    static void start();

    /**
     * @brief Handles the left button action.
     */
    static void performLeftAction();

    /**
     * @brief Handles the middle button action.
     */
    static void performMiddleAction();

    /**
     * @brief Handles the right button action.
     */
    static void performRightAction();

    /**
     * @brief Enables the game.
     * @return True if the game is successfully enabled, false otherwise.
     */
    static void enable();

    /**
     * @brief Disables the game.
     */
    static void disable();

    /**
     * @brief Checks if the game is active.
     * @return True if the game is active, false otherwise.
     */
    static bool isActive();

    /**
     * @brief Sets the game status to active or inactive.
     * @param value The new status value (0 for inactive, 1 for active).
     */
    static void setActive(bool value);

    static void cancelBuzzTime();

    /**
     * @brief Handles the aftermath of launching/dropping the CyberPartner.
     */
    static void handleFreeFallEvent( float meters );

    /**
     * @brief Exits the CyberPartner game.
     */
    static void exit();

    protected:
    /**
     * @brief Default destructor for the CyberPartnerGame class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~CyberPartnerGame() = default;

private:
    /**
     * @brief Default constructor for the CyberPartnerGame class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    CyberPartnerGame() = default;

    static Scheduler& taskRunner; /**< Task scheduler for general tasks */
    static Scheduler& highPrioTaskRunner; /**< Task scheduler for high priority tasks */
    static Task displayTask; /**< Task for updating the display */
    static Task dayUpdateTask; /**< Task for performing yearly updates */
    static bool status; /**< Status of the CyberPartner game */
    static bool death_move; /**< Flag for death animation movement */
    static bool statsScreenEnabled; /**< Flag for stats screen visibility */
    static uint8_t minX; /**< Minimum X position for animations */
    static bool trackBuzzTime; /**< Flag for tracking buzz time */

    struct TaskData {
        Task* scannerTask;
        Task* displayTask;
        bool operationComplete;
        bool setDisplay;
        std::function<void()> operation;
    };
    static void genericTaskCallback();
    static void animateCyberPartner();
    static void animateEggPhase();
    static void animateDeathPhase();
    static void animateOtherPhases();
    static void statUpdateAction();
    static void dayUpdateAction();
    static void displayStatusBar();
    static void displayStatusAge();
    static void displayStatusWeight();
    static void toggleStatsScreen();
    static void displayStatsScreen();
    static void drawStatusSprite(uint16_t locX, uint16_t locY, uint8_t spriteIndex);
    static void displayStatusText(uint16_t locX, const char* text);
    static void displayTeardown();
    static void displayText(const char* text);
    static void handleScanResult(TaskData* taskData, std::function<void()> onDeviceFound);

    /**
     * @brief Displays the given data in hexadecimal format.
     * @param data The data to display.
     *
     * \warning This function is currently not implemented. Not sure what @nutcrunch was
     * going to use it for.
     */
    //template <size_t N>
    //static void displayHexData(const std::array<uint8_t, N>& data);
};
