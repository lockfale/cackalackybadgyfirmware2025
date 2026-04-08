#pragma once

#include <Arduino.h>
#include "game_timer.h"


class WEIGHTSHAKE {
protected:
/**
 * @brief Destructor for WEIGHTSHAKE.
 * @note This destructor is declared as default, which means the compiler will generate the default implementation.
 *       It is used to clean up any resources when the instance of WEIGHTSHAKE is destroyed.
 */
~WEIGHTSHAKE() = default;

public:
    /**
     * @brief Singleton accessor for WEIGHTSHAKE.
     * @return Reference to the singleton instance.
     * @note This function uses the Meyers' singleton pattern to ensure that only one instance of WEIGHTSHAKE exists.
     *       The instance is created the first time this function is called and is destroyed when the program ends.
     */
    static WEIGHTSHAKE& getInstance(); // Singleton accessor


    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    WEIGHTSHAKE(const WEIGHTSHAKE&) = delete; // Delete copy constructor

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    WEIGHTSHAKE& operator=(const WEIGHTSHAKE&) = delete; // Delete copy assignment operator

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    WEIGHTSHAKE(WEIGHTSHAKE&&) = delete; // Delete move constructor

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    WEIGHTSHAKE& operator=(WEIGHTSHAKE&&) = delete; // Delete move assignment operator

    /**
     * @brief Updates the WEIGHTSHAKE state.
     * @note This function should be called periodically to keep the game running.
     *       It handles the game logic and updates the display accordingly.
     *       It is not thread-safe and should be used in a single-threaded context.
     */
    void update( void );

    /**
     * @brief Starts the WEIGHTSHAKE.
     * @note This function disables the menu before initializing required variables to starting the game.
     */
    void doStart( void );

    /**
     * @brief Stops the WEIGHTSHAKE minigame.
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
    WEIGHTSHAKE() = default; // Private constructor for singleton

    /**
     * @brief Enumeration for the different game states.
     * @note This enum is used to track the current state of the minigame.
     */
    enum class WSGameState {
        WS_DISABLED,
        WS_TITLE_BANNER,
        WS_INTERMEDIARY_BANNER,
        WS_DISPLAY_RESULT
    };

    // Static member to keep track of the banner ticks, pumps, and max weight loss
    uint16_t bannerTicker;
    uint16_t pumpCount;
    int cpWeightLossMax;
    bool buttonFlag;

    // variables needed to calculate a pump
    float lastAccelZ = 0.0; // Store the last Z-axis acceleration value
    bool isMovingUp = false; // Track if the hardware is moving up
    const float upThreshold = 1.0; // Threshold for detecting upward motion
    const float downThreshold = -1.0; // Threshold for detecting downward motion

    /**
     * @brief Runs the minigame logic.
     */
    void runWeightShake( void );

    /**
     * @brief Checks if the time elapsed since the last check is greater than the specified duration.
     * @param duration The duration to check against.
     * @return True if the time elapsed is greater than the duration, false otherwise.
     * @note This function uses a static variable to keep track of the previous time.
     *      This effectively ticks the game every 50 milliseconds.
     *      It is not thread-safe and should be used in a single-threaded context.
     */
    GameTimer _gameTimer;

    static constexpr int WS_CHECK_TIME_INTERVAL = 50;
    WSGameState currentState = WSGameState::WS_DISABLED;
    unsigned long wsPatternSeed;
};
