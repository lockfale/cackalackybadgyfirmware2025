#include "cyberSpa.h"

#include "badge_display.h"
#include "badgeMenu.h"
#include "CyberPartner.h"
#include "CPStore.h"
#include "haptic.h"
#include "neocontroller.h"

// 'bad_lotus', 96x63px
const unsigned char bad_lotus_bitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0xc0, 0x00, 0x70, 0x0e, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0,
	0x00, 0x70, 0x0e, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x70, 0x0e, 0x00,
	0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
	0x00, 0x00, 0x1c, 0x3f, 0x87, 0x80, 0x00, 0xe1, 0xfc, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x3f,
	0x87, 0x80, 0x00, 0xe1, 0xfc, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x3f, 0x87, 0x80, 0x00, 0xe1,
	0xfc, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x7f, 0x80, 0x00, 0xfe, 0x00, 0x38, 0x00, 0x00,
	0x00, 0x00, 0x1c, 0x00, 0x7f, 0x80, 0x00, 0xfe, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00,
	0x7f, 0x80, 0x00, 0xfe, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x7f, 0x80, 0x00, 0xfe,
	0x00, 0x38, 0x00, 0x00, 0xff, 0xff, 0xfc, 0x00, 0x07, 0x80, 0x00, 0xe0, 0x00, 0x3f, 0xff, 0xff,
	0xff, 0xff, 0xfc, 0x00, 0x07, 0x80, 0x00, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00,
	0x07, 0x80, 0x00, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xf0, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x0f, 0xf0, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x0f,
	0xf0, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x0f, 0xf0, 0x00, 0x1c, 0x00,
	0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x0f, 0x0e, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e,
	0x00, 0x38, 0x00, 0x70, 0x0e, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x70,
	0x0e, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x70, 0x0e, 0x00, 0x1c, 0x00,
	0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x70, 0x0e, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e,
	0x00, 0x38, 0x00, 0x70, 0x0e, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x70,
	0x0e, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x38, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
	0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e,
	0x03, 0xc0, 0x07, 0x80, 0x00, 0xe0, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x07, 0x80,
	0x00, 0xe0, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x07, 0x80, 0x00, 0x1e, 0x03, 0xc0,
	0x78, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e,
	0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x78, 0x00,
	0x00, 0x1e, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x1e, 0x03, 0xc0, 0x78, 0x00, 0x00, 0x01, 0xc0, 0x38,
	0x07, 0x80, 0x00, 0xe0, 0x1c, 0x03, 0x80, 0x00, 0x00, 0x01, 0xc0, 0x38, 0x07, 0x80, 0x00, 0xe0,
	0x1c, 0x03, 0x80, 0x00, 0x00, 0x01, 0xc0, 0x38, 0x07, 0x80, 0x00, 0xe0, 0x1c, 0x03, 0x80, 0x00,
	0x00, 0x01, 0xc0, 0x38, 0x07, 0x80, 0x00, 0xe0, 0x1c, 0x03, 0x80, 0x00, 0x00, 0xff, 0xff, 0xc3,
	0xf8, 0x70, 0x0e, 0x1f, 0xc3, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0xc3, 0xf8, 0x70, 0x0e, 0x1f,
	0xc3, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0xc3, 0xf8, 0x70, 0x0e, 0x1f, 0xc3, 0xff, 0xff, 0x80,
	0x00, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x07, 0x80, 0x00, 0xe0, 0x00, 0x3f,
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x07, 0x80, 0x00, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff,
	0xfc, 0x00, 0x07, 0x80, 0x00, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x07, 0x80,
	0x00, 0x1e, 0x00, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x00,
	0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x78, 0x00, 0x00, 0x1e,
	0x00, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x78, 0x00,
	0x00, 0x01, 0xc0, 0x00, 0x78, 0x00, 0x00, 0x1e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x01, 0xc0, 0x00,
	0x78, 0x00, 0x00, 0x1e, 0x00, 0x03, 0x80, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x78, 0x00, 0x00, 0x1e,
	0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x1f, 0xff, 0x87, 0xf0, 0x0f, 0xe1, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff,
	0x87, 0xf0, 0x0f, 0xe1, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xf0, 0x0f, 0xe1,
	0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};
int lotusTicks = 0;


/**
 * @brief Singleton accessor for CYBERSPA.
 * @return Reference to the singleton instance.
 * @note This function uses the Meyers' singleton pattern to ensure that only one instance of CYBERSPA exists.
 *       The instance is created the first time this function is called and is destroyed when the program ends.
 */
CYBERSPA& CYBERSPA::getInstance() {
    static CYBERSPA instance;
    return instance;
}

// Define static members
uint16_t CYBERSPA::bannerTicker = 0;

/**
 * @brief Runs the CYBERSPA game logic.
 * @note This function handles the game state transitions and updates the display accordingly.
 */
void CYBERSPA::runCyberSpa( void ){
    //handle banner expirations first
    // Title Banner expired
    if (currentState == SPAGameState::SPA_TITLE_BANNER) {
        if (bannerTicker >= 100 +  lotusTicks) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            currentState = SPAGameState::SPA_MENU_BANNER;
            bannerTicker = 0;
        }
    } else if (currentState == SPAGameState::SPA_MENU_BANNER) {
        if (confirmation) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            if (selectedOption == 0) {
                if (CPStore::getInstance().getMoney() <= 20) {
                    brokeFlag = true;
                }
            } else {
                if (CPStore::getInstance().getMoney() <= 45) {
                    brokeFlag = true;
                }
            }
            currentState = SPAGameState::SPA_PERFORMANCE_BANNER;
            bannerTicker = 0;
        }
    } else if (currentState == SPAGameState::SPA_PERFORMANCE_BANNER) {
        if (bannerTicker >= 2 + bannerLimit ) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            BadgeHaptic::getInstance().hapticOff();
            currentState = SPAGameState::SPA_DISPLAY_RESULT;
            bannerTicker = 0;
        }
    } else if (currentState == SPAGameState::SPA_DISPLAY_RESULT) {
        if (bannerTicker >= 160) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            doStop();
        }
    }
    //then print banners, start timers, handle nonbanner game states
    if (currentState == SPAGameState::SPA_TITLE_BANNER) {
        if (bannerTicker < 1) {
            lotusTicks = BDISPLAY::startScrollingBitmap(bad_lotus_bitmap, 96, 0, 96, 63, 5, 0);
        } else if (BDISPLAY::scrollingBitmapState.active) {
            BDISPLAY::updateScrollingBitmap();
        } else {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.println();
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.println(F(" Cyber Spa"));
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F("     No Refunds!"));
            BDISPLAY::display.display();
        }
        bannerTicker++;
    } else if (currentState == SPAGameState::SPA_MENU_BANNER) {
        if (!confirmation) {
            // Display the menu for the first time
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.println(F(" Select Massage:"));
            BDISPLAY::display.setTextSize(2);

            // Highlight the selected option
            if (selectedOption == 0) {
                BDISPLAY::display.setTextColor(BLACK, WHITE); // Highlight Regular Massage
                BDISPLAY::display.println(F(" Reg $20"));
                BDISPLAY::display.setTextColor(WHITE, BLACK); // Normal for Premium Massage
                BDISPLAY::display.println(F(" Preem $45"));
            } else {
                BDISPLAY::display.setTextColor(WHITE, BLACK); // Normal for Regular Massage
                BDISPLAY::display.println(F(" Reg $20"));
                BDISPLAY::display.setTextColor(BLACK, WHITE); // Highlight Premium Massage
                BDISPLAY::display.println(F(" Preem $45"));
            }

            BDISPLAY::display.display();
        }
        bannerTicker++;
    } else if (currentState == SPAGameState::SPA_PERFORMANCE_BANNER) {
        // Logic for matching player bet against spinResult
        // Increment/decremnet playerMoney
        if (bannerTicker < 1) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setCursor(0, 0);
            if (!brokeFlag) {
                BDISPLAY::display.println(F(" Massage Time!"));
                BadgeHaptic::getInstance().hapticOn();
                if (selectedOption == 0) {
                    bannerLimit = 1000;
                } else {
                    bannerLimit = 4000;
                }
            } else {
                bannerLimit = 120;
                BDISPLAY::display.println(F(" Not Enough Money!"));
                NeoController::getInstance().changeRed();
            }
            BDISPLAY::display.display();
        // Show some soothing images during the massage
        } else {
            if (BDISPLAY::scrollingBitmapState.active) {
                BDISPLAY::updateScrollingBitmap();
            } else {
                if (selectedOption == 0) {
                    BDISPLAY::startScrollingBitmap(bad_lotus_bitmap, 96, 0, 96, 63, 5, 0);
                } else {
                    BDISPLAY::startScrollingBitmap(bad_lotus_bitmap, 96, 0, 96, 63, 20, 0);
                }
            }
        }
        bannerTicker++;
    } else if (currentState == SPAGameState::SPA_DISPLAY_RESULT) {
        // Logic for matching player bet against spinResult
        // Increment/decremnet playerMoney
        if (bannerTicker < 1) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setCursor(0, 0);
            NeoController::getInstance().changeGreen();
            if (!brokeFlag) {
                BDISPLAY::display.println(F(""));
                BDISPLAY::display.println(F(" Spa Time is Up!"));
            } else {
                BDISPLAY::display.println(F(""));
                BDISPLAY::display.println(F(" Go earn, come back"));
            }
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(" No Refunds!"));
            BDISPLAY::display.display();
            if (selectedOption == 0) {
                BDISPLAY::display.println(F(" Regular Massage: Complete"));
                CyberPartner::getInstance().increaseHappiness(20);
                CPStore::getInstance().decreaseMoney(20, true);
            } else {
                BDISPLAY::display.println(F(" Premium Massage: Complete"));
                CyberPartner::getInstance().increaseHappiness(75);
                CPStore::getInstance().decreaseMoney(45, true);
            }
            // TODO save CP state (to campture happiness? unsure if done via decreaseMoney above)
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
 * @brief Updates the CYBERSPA state.
 * @note This function should be called periodically to keep the game running.
 *       It handles the game logic and updates the display accordingly.
 *       It is not thread-safe and should be used in a single-threaded context.
 */
void CYBERSPA::update( void ){
    if (currentState != SPAGameState::SPA_DISABLED) {
        if (_gameTimer.check(SPA_CHECK_TIME_INTERVAL)) {
            runCyberSpa();
        }
    }
}

/**
 * @brief Starts the CYBERSPA minigame.
 * @note This function disables the menu before initializing required variables to starting the game.
 */
void CYBERSPA::doStart( void ){
    BMenu::disableMenu();
    currentState = SPAGameState::SPA_TITLE_BANNER;
    bannerTicker = 0;
    selectedOption = 0;
    confirmation = false;
    brokeFlag = false;
}

/**
 * @brief Stops the CYBERSPA minigame.
 * @note This function should clear any game-related variables and re-enables the menu.
 */
void CYBERSPA::doStop( void ){
    currentState = SPAGameState::SPA_DISABLED;
    bannerTicker = 0;
    selectedOption = 0;
    confirmation = false;
    brokeFlag = false;
    BadgeHaptic::getInstance().hapticOff();

    NeoController::getInstance().turnAllOff();
    // TODO optional networking things here
    BMenu::enableMenu();  // enable menu since we are done
}

/**
 * @brief Checks if the minigame is currently running.
 * @return True if the game is running, false otherwise.
 */
bool CYBERSPA::isRunning( void ){
    if (currentState == SPAGameState::SPA_DISABLED) {
        return false;
    } else {
        return true;
    }
}

/**
 * @brief Handles the up button click for the minigame.
 */
void CYBERSPA::clickUp( void ){
    if (currentState == SPAGameState::SPA_MENU_BANNER) {
        // Navigate up in the menu
        selectedOption = (selectedOption - 1 + 2) % 2; // Wrap around between 0 and 1
    }
}

/**
 * @brief Handles the down button click for the minigame.
 */
void CYBERSPA::clickDown( void ){
    if (currentState == SPAGameState::SPA_MENU_BANNER) {
        // Navigate down in the menu
        selectedOption = (selectedOption + 1) % 2; // Wrap around between 0 and 1
        bannerTicker = 0; // Force menu to refresh
    }
}

/**
 * @brief Handles the confirm button click for the minigame.
 */
void CYBERSPA::clickConfirm( void ){
    if (currentState == SPAGameState::SPA_MENU_BANNER) {
        // Confirm the selected option
        if (selectedOption == 0) {
            Serial.println(F("Regular Massage Selected"));
        } else {
            Serial.println(F("Premium Massage Selected"));
        }

        confirmation = true;
        bannerTicker = 0;
    }
}

/**
 * @brief Handles the cancel button click for the CYBERSPA.
 */
void CYBERSPA::clickCancel( void ){
    if (currentState == SPAGameState::SPA_TITLE_BANNER) {
        doStop();
    } else if (currentState == SPAGameState::SPA_DISPLAY_RESULT) {
        doStop();
    }
}