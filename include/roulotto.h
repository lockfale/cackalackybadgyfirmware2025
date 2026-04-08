#pragma once

#include "game_timer.h"
#include <vector>
#include <array>
#include <Arduino.h>
#include <string>

class ROULOTTO {
protected:
    /**
     * @brief Destructor for Roulotto.
     * @note This destructor is declared as default, which means the compiler will generate the default implementation.
     *       It is used to clean up any resources when the instance of Roulotto is destroyed.
     */
    ~ROULOTTO() = default;

public:


    /**
     * @brief Singleton accessor for Roulotto.
     * @return Reference to the singleton instance.
     * @note This function uses the Meyers' singleton pattern to ensure that only one instance of MINIGAME exists.
     *       The instance is created the first time this function is called and is destroyed when the program ends.
     */
    static ROULOTTO& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    ROULOTTO(const ROULOTTO&) = delete; // Delete copy constructor

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    ROULOTTO& operator=(const ROULOTTO&) = delete; // Delete copy assignment operator

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    ROULOTTO(ROULOTTO&&) = delete; // Delete move constructor

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    ROULOTTO& operator=(ROULOTTO&&) = delete; // Delete move assignment operator

    /**
     * @brief Updates the Roulotto game state.
     * @note This function should be called periodically to keep the game running.
     *       It handles the game logic and updates the display accordingly.
     *       It is not thread-safe and should be used in a single-threaded context.
     */
    void update( void );

    /**
     * @brief Starts the Roulotto game.
     * @note This function disables the menu before initializing required variables to starting the game.
     */
    void doStart( void );

    /**
     * @brief Stops the Roulotto game.
     * @note This function should clear any game-related variables and re-enables the menu.
     */
    void doStop( void );

    /**
     * @brief Checks if the Roulotto game is currently running.
     * @return True if the game is running, false otherwise.
     */
    bool isRunning( void );

    /**
     * @brief Handles the up button click for the Roulotto game.
     */
    void clickUp( void );

    /**
     * @brief Handles the down button click for the Roulotto game.
     */
    void clickDown( void );

    /**
     * @brief Handles the confirm button click for the Roulotto game.
     */
    void clickConfirm( void );

    /**
     * @brief Handles the cancel button click for the Roulotto game.
     */
    void clickCancel( void );

private:
    ROULOTTO() = default; // Private constructor for singleton

    /**
     * @enum RLGameState
     * @brief States of the Roulotto game.
     */
    enum class RLGameState {
        RL_DISABLED,
        RL_TITLE_BANNER,
        RL_SELECT_BET_TYPE,
        RL_SELECT_INSIDE_BET,
        RL_SELECT_OUTSIDE_BET,
        RL_INPUT_BET_AMOUNT,
        RL_SPIN_WHEEL,
        RL_DISPLAY_RESULT
    };

    // Static member to keep track of the banner ticks
    static uint16_t bannerTicker;

    // Static definition of the red numbers on a roulette wheel
    static constexpr std::array<int, 18> redNumbers = {
        1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36
    };

    /**
     * @brief Checks if a number is green (0, 00, or 000).
     * @param number The number to check.
     * @return True if the number is green, false otherwise.
     */
    bool isGreen(int number);

    /**
     * @brief Checks if a number is red.
     * @param number The number to check.
     * @return True if the number is red, false otherwise.
     */
    bool isRed(int number);

    /**
     * @brief Gets the color of a number (Red, Black, or Green).
     * @param number The number to check.
     * @return The color of the number as a string.
     */
    std::string getColorOfNumber(int number);

    /**
     * @brief Converts the bet type to a string representation.
     * @param insideBet The inside bet tracking variable.
     * @param outsideBet The outside bet trackng variable.
     * @return The string representation of the bet type.
     */
    std::string betToString(int insideBet, int outsideBet);

    /**
     * @brief Converts the spin result to a string representation.
     * @param spinResult The integer result of the spin.
     * @return The string representation of the spin result.
     */
    std::string spinToString(int spinResult);

    /**
     * @brief Runs the Roulotto game logic.
     */
    void runRoulotto( void );

    /**
     * @brief Checks if the time elapsed since the last check is greater than the specified duration.
     * @param duration The duration to check against.
     * @return True if the time elapsed is greater than the duration, false otherwise.
     * @note This function uses a static variable to keep track of the previous time.
     *      This effectively ticks the game every 50 milliseconds.
     *      It is not thread-safe and should be used in a single-threaded context.
     */
    GameTimer _gameTimer;

    /**
     * @brief Spins the roulette wheel and returns a randomized, weighted result.
     * @return The result of the spin as an integer.
     */
    int spinWheel();

    /**
     * @brief Checks if the bet is a winning bet based on the spin result.
     * @param insideBet The inside bet tracking variable.
     * @param outsideBet The outside bet tracking variable.
     * @param spinResult The integer result of the spin.
     * @return True if the bet is a winning bet, false otherwise.
     */
    bool isWinningBet(int insideBet, int outsideBet, int spinResult);


    static constexpr int RL_CHECK_TIME_INTERVAL = 50;
    RLGameState currentState = RLGameState::RL_DISABLED;
    unsigned long rlPatternSeed;

    uint16_t playerMoney = 0;
    bool confirmation = false;
    uint16_t inputIndex = 0;


    int insideBetSelected = 0;
    int outsideBetSelected = 0;
    uint16_t currentBetAmount = 0;
    int spinResult = -1;

    std::vector<int> weights;
};
