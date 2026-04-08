#pragma once

#include <Arduino.h>
#include "game_timer.h"


class CYBERSPA {
protected:
/**
 * @brief Destructor for CYBERSPA.
 * @note This destructor is declared as default, which means the compiler will generate the default implementation.
 *       It is used to clean up any resources when the instance of CYBERSPA is destroyed.
 */
~CYBERSPA() = default;

public:
    /**
     * @brief Singleton accessor for CYBERSPA.
     * @return Reference to the singleton instance.
     * @note This function uses the Meyers' singleton pattern to ensure that only one instance of CYBERSPA exists.
     *       The instance is created the first time this function is called and is destroyed when the program ends.
     */
    static CYBERSPA& getInstance(); // Singleton accessor


    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    CYBERSPA(const CYBERSPA&) = delete; // Delete copy constructor

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    CYBERSPA& operator=(const CYBERSPA&) = delete; // Delete copy assignment operator

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    CYBERSPA(CYBERSPA&&) = delete; // Delete move constructor

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    CYBERSPA& operator=(CYBERSPA&&) = delete; // Delete move assignment operator

    /**
     * @brief Updates the CYBERSPA state.
     * @note This function should be called periodically to keep the game running.
     *       It handles the game logic and updates the display accordingly.
     *       It is not thread-safe and should be used in a single-threaded context.
     */
    void update( void );

    /**
     * @brief Starts the CYBERSPA.
     * @note This function disables the menu before initializing required variables to starting the game.
     */
    void doStart( void );

    /**
     * @brief Stops the CYBERSPA.
     * @note This function should clear any game-related variables and re-enables the menu.
     */
    void doStop( void );

    /**
     * @brief Checks if the CYBERSPA is currently running.
     * @return True if the game is running, false otherwise.
     */
    bool isRunning( void );

    /**
     * @brief Handles the up button click for the CYBERSPA.
     */
    void clickUp( void );

    /**
     * @brief Handles the down button click for the CYBERSPA.
     */
    void clickDown( void );

    /**
     * @brief Handles the confirm button click for the CYBERSPA.
     */
    void clickConfirm( void );

    /**
     * @brief Handles the cancel button click for the CYBERSPA.
     */
    void clickCancel( void );

private:
    CYBERSPA() = default; // Private constructor for singleton

    /**
     * @brief Enumeration for the different game states.
     * @note This enum is used to track the current state of the CYBERSPA.
     */
    enum class SPAGameState {
        SPA_DISABLED,
        SPA_TITLE_BANNER,
        SPA_MENU_BANNER,
        SPA_PERFORMANCE_BANNER,
        SPA_DISPLAY_RESULT
    };

    // Static member to keep track of the banner ticks
    static uint16_t bannerTicker;
    uint32_t bannerLimit = 0; // Limit for the performance banner display
    int selectedOption = 0; // 0 = Regular Massage, 1 = Premium Massage
    bool confirmation = false; // Confirmation flag for the selected option
    bool brokeFlag = false; // Flag to indicate if not enough money is available

    /**
     * @brief Runs the CYBERSPA logic.
     */
    void runCyberSpa( void );

    /**
     * @brief Checks if the time elapsed since the last check is greater than the specified duration.
     * @param duration The duration to check against.
     * @return True if the time elapsed is greater than the duration, false otherwise.
     * @note This function uses a static variable to keep track of the previous time.
     *      This effectively ticks the game every 50 milliseconds.
     *      It is not thread-safe and should be used in a single-threaded context.
     */
    GameTimer _gameTimer;

    static constexpr int SPA_CHECK_TIME_INTERVAL = 50;
    SPAGameState currentState = SPAGameState::SPA_DISABLED;
    unsigned long spaPatternSeed;
};
