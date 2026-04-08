#include "minigame_template.h"

#include "badge_display.h"
#include "badgeMenu.h"
#include "neocontroller.h"

/**
 * @brief Singleton accessor for MINIGAME.
 * @return Reference to the singleton instance.
 * @note This function uses the Meyers' singleton pattern to ensure that only one instance of MINIGAME exists.
 *       The instance is created the first time this function is called and is destroyed when the program ends.
 */
MINIGAME& MINIGAME::getInstance() {
    static MINIGAME instance;
    return instance;
}

// Define static members
uint16_t MINIGAME::bannerTicker = 0;

/**
 * @brief Runs the minigame logic.
 * @note This function handles the game state transitions and updates the display accordingly.
 */
void MINIGAME::runMinigame( void ){
    //handle banner expirations first
    // Title Banner expired
    if (currentState == MGGameState::MG_TITLE_BANNER) {
        if (bannerTicker >= 120) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            currentState = MGGameState::MG_DISPLAY_RESULT;
            bannerTicker = 0;
        }
    } else if (currentState == MGGameState::MG_DISPLAY_RESULT) {
        if (bannerTicker >= 160) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            doStop();
        }
    }
    //then print banners, start timers, handle nonbanner game states
    if (currentState == MGGameState::MG_TITLE_BANNER) {
        if (bannerTicker < 1) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.println();
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.println(F(" Minigame"));
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.println(F("Stuff + Things"));
            BDISPLAY::display.display();
        }
        bannerTicker++;
    } else if (currentState == MGGameState::MG_DISPLAY_RESULT) {
        // Logic for matching player bet against spinResult
        // Increment/decremnet playerMoney
        if (bannerTicker < 1) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setCursor(0, 0);
            NeoController::getInstance().changeGreen();
            BDISPLAY::display.println(F(" Game Complete!"));
            BDISPLAY::display.display();
        }
        bannerTicker++;
    }
}

/**
 * @brief Checks if the time elapsed since the last check is greater than the specified duration.
 * @param duration The duration to check against.
 * @return True if the time elapsed is greater than the duration, false otherwise.
 * @note This function uses a static variable to keep track of the previous time.
 *       This effectively ticks the game every 50 milliseconds.
 *       It is not thread-safe and should be used in a single-threaded context.
 */

/**
 * @brief Updates the minigame state.
 * @note This function should be called periodically to keep the game running.
 *       It handles the game logic and updates the display accordingly.
 *       It is not thread-safe and should be used in a single-threaded context.
 */
void MINIGAME::update( void ){
    if (currentState != MGGameState::MG_DISABLED) {
        if (_gameTimer.check(MG_CHECK_TIME_INTERVAL)) {
            runMinigame();
        }
    }
}

/**
 * @brief Starts the minigame.
 * @note This function disables the menu before initializing required variables to starting the game.
 */
void MINIGAME::doStart( void ){
    BMenu::disableMenu();
    currentState = MGGameState::MG_TITLE_BANNER;
    bannerTicker = 0;
    mgPatternSeed = millis() + ESP.getCycleCount();  // uptime + CPU cycle count, yuge variation
    randomSeed(mgPatternSeed);
}

/**
 * @brief Stops the minigame.
 * @note This function should clear any game-related variables and re-enables the menu.
 */
void MINIGAME::doStop( void ){
    currentState = MGGameState::MG_DISABLED;
    bannerTicker = 0;
    mgPatternSeed = 0;
    randomSeed(mgPatternSeed);  // ~deseed random

    NeoController::getInstance().turnAllOff();
    // TODO optional networking things here
    BMenu::enableMenu();  // enable menu since we are done
}

/**
 * @brief Checks if the minigame is currently running.
 * @return True if the game is running, false otherwise.
 */
bool MINIGAME::isRunning( void ){
    if (currentState == MGGameState::MG_DISABLED) {
        return false;
    } else {
        return true;
    }
}

/**
 * @brief Handles the up button click for the minigame.
 */
void MINIGAME::clickUp( void ){
    if (currentState == MGGameState::MG_TITLE_BANNER) {
        doStop();
    } else if (currentState == MGGameState::MG_DISPLAY_RESULT) {
        doStop();
    }
}

/**
 * @brief Handles the down button click for the minigame.
 */
void MINIGAME::clickDown( void ){
    if (currentState == MGGameState::MG_TITLE_BANNER) {
        doStop();
    } else if (currentState == MGGameState::MG_DISPLAY_RESULT) {
        doStop();
    }
}

/**
 * @brief Handles the confirm button click for the minigame.
 */
void MINIGAME::clickConfirm( void ){
    if (currentState == MGGameState::MG_TITLE_BANNER) {
        doStop();
    } else if (currentState == MGGameState::MG_DISPLAY_RESULT) {
        doStop();
    }
}

/**
 * @brief Handles the cancel button click for the minigame.
 */
void MINIGAME::clickCancel( void ){
    if (currentState == MGGameState::MG_TITLE_BANNER) {
        doStop();
    } else if (currentState == MGGameState::MG_DISPLAY_RESULT) {
        doStop();
    }
}