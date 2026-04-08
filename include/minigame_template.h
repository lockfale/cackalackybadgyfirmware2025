#pragma once

#include <Arduino.h>
#include "game_timer.h"


class MINIGAME {
protected:
/**
 * @brief Destructor for MINIGAME.
 * @note This destructor is declared as default, which means the compiler will generate the default implementation.
 *       It is used to clean up any resources when the instance of MINIGAME is destroyed.
 */
~MINIGAME() = default;

public:
    /**
     * @brief Singleton accessor for MINIGAME.
     * @return Reference to the singleton instance.
     * @note This function uses the Meyers' singleton pattern to ensure that only one instance of MINIGAME exists.
     *       The instance is created the first time this function is called and is destroyed when the program ends.
     */
    static MINIGAME& getInstance(); // Singleton accessor


    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    MINIGAME(const MINIGAME&) = delete; // Delete copy constructor

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    MINIGAME& operator=(const MINIGAME&) = delete; // Delete copy assignment operator

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    MINIGAME(MINIGAME&&) = delete; // Delete move constructor

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    MINIGAME& operator=(MINIGAME&&) = delete; // Delete move assignment operator

    /**
     * @brief Updates the minigame state.
     * @note This function should be called periodically to keep the game running.
     *       It handles the game logic and updates the display accordingly.
     *       It is not thread-safe and should be used in a single-threaded context.
     */
    void update( void );

    /**
     * @brief Starts the minigame.
     * @note This function disables the menu before initializing required variables to starting the game.
     */
    void doStart( void );

    /**
     * @brief Stops the minigame.
     * @note This function should clear any game-related variables and re-enables the menu.
     */
    void doStop( void );

    /**
     * @brief Checks if the minigame is currently running.
     * @return True if the game is running, false otherwise.
     */
    bool isRunning( void );

    /**
     * @brief Handles the up button click for the minigame.
     */
    void clickUp( void );

    /**
     * @brief Handles the down button click for the minigame.
     */
    void clickDown( void );

    /**
     * @brief Handles the confirm button click for the minigame.
     */
    void clickConfirm( void );

    /**
     * @brief Handles the cancel button click for the minigame.
     */
    void clickCancel( void );

private:
    MINIGAME() = default; // Private constructor for singleton

    /**
     * @brief Enumeration for the different game states.
     * @note This enum is used to track the current state of the minigame.
     */
    enum class MGGameState {
        MG_DISABLED,
        MG_TITLE_BANNER,
        MG_INTERMEDIARY_BANNER,
        MG_DISPLAY_RESULT
    };

    // Static member to keep track of the banner ticks
    static uint16_t bannerTicker;

    /**
     * @brief Runs the minigame logic.
     */
    void runMinigame( void );

    /**
     * @brief Checks if the time elapsed since the last check is greater than the specified duration.
     * @param duration The duration to check against.
     * @return True if the time elapsed is greater than the duration, false otherwise.
     * @note This function uses a static variable to keep track of the previous time.
     *      This effectively ticks the game every 50 milliseconds.
     *      It is not thread-safe and should be used in a single-threaded context.
     */
    GameTimer _gameTimer;

    static constexpr int MG_CHECK_TIME_INTERVAL = 50;
    MGGameState currentState = MGGameState::MG_DISABLED;
    unsigned long mgPatternSeed;
};
