/**
 * @file CPMating.h
 * @brief Defines the CPMating class, which displays when CyberPartners mate.
 *
 * This file declares the CPMating singleton.
 */
#pragma once

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>
#include <Arduino.h>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define CPMATINGDEBUG // uncomment to enable CPMating debugging
#endif

class CPMating
{
public:
    /**
     * @brief Retrieves the singleton instance of the CPMating class.
     *
     * This function provides access to the single instance of the `CPMating` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `CPMating` instance.
     *
     * @details
     * - The `CPMating` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * CPMating& mating = CPMating::getInstance();
     * mating.doStart();
     * ```
     */
    static CPMating& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    CPMating(const CPMating&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    CPMating& operator=(const CPMating&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    CPMating(CPMating&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    CPMating& operator=(CPMating&&) = delete;

    static void displayStub();

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
     * @brief Handles the boot button action.
     */
    static void performBootAction();

    /**
     * @brief Check if the CPMating is running.
     * @return True if running, false otherwise.
     */
    static bool isActive();

	/**
     * @brief Start the CPMating.
     */
    static void doStart();

    /**
     * @brief Stop the CPMating.
     */
    static void doStop();

protected:
    /**
     * @brief Default destructor for the CPMating class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~CPMating() = default;

private:
    /**
     * @brief Default constructor for the CPMating class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    CPMating() = default;

    static Scheduler& taskRunner; /**< Task scheduler for general tasks */
    static Task displayTask; /**< Task for updating the display */

    static const uint8_t (*sprite_sizes)[2];  /**< Pointer to an array of sprite size data */
    static const unsigned char* const* sprite_data;  /**< Array of sprite image data */

    static std::array<uint8_t, 8> visiting_buttonID; /**< ID of the visiting button */

    /**
     * @enum CPMATING_STATE_t
     * @brief Represents the states of the CPMating display.
     *
     * This enumeration defines the possible states for the CPMating display,
     * indicating whether it is currently disabled or running.
     */
	enum class CPMATING_STATE_t {
        CPMATING_DISPLAY_DISABLED, ///< CPMating is disabled
        CPMATING_DISPLAY_RUNNING   ///< CPMating is running
    };

    /**
     * @brief Enumeration for the CPMating display states.
     *
     * This enumeration defines the possible states for the CPMating display,
     * indicating whether it is currently disabled or running.
     */
    enum class CPMateSprite : uint8_t {
        Connect = 0,  // Represents the "cpmate_connect" sprite
        Fail = 1,     // Represents the "cpmate_fail" sprite
        Seesaw = 2    // Represents the "seesaw" sprite
    };

    /**
     * @brief Current state of the CPMating display.
     *
     * This static member variable holds the current state of the CPMating display,
     * as defined by the `CPMATING_STATE_t` enumeration.
     */
    static CPMATING_STATE_t cpmating_state;

    enum class DisplayState {
        TooYoung,
        TooDead,
		MeetAFriend,
        Masturbation,
        Mating
	};

    static void animateCPMating();
    static void animatePlaying();
    static void displayStatusBar();
    static void displayMessage(const __FlashStringHelper* message);
	static void showDisplayState(DisplayState state);

    static void scanButton();
};
