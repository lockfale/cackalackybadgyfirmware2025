#include "weightShake.h"

#include <algorithm>
#include "CyberPartner.h"
#include "accelerometer.h"
#include "badge_display.h"
#include "badgeMenu.h"
#include "neocontroller.h"

int marqueeTicks = 0;

// 'bad_dumbbell', 96x48px
const unsigned char bad_dumbbell_bitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x1f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 
	0xe0, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0xfc, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0xfc, 0x00, 0x00, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x3f, 
	0x00, 0x00, 0x1f, 0xc0, 0x00, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x1f, 0xc0, 
	0x00, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x3f, 0xff, 0xf8, 
	0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 
	0x00, 0x00, 0x03, 0xf8, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x03, 0xf8, 
	0x03, 0xf8, 0x00, 0x3f, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x03, 0xf8, 0x00, 0x3f, 
	0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x03, 0xf8, 0x00, 0x3f, 0x00, 0x00, 0x1f, 0xf8, 
	0x00, 0x00, 0x03, 0xf8, 0x1f, 0xc7, 0xff, 0xc7, 0xe0, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 
	0x1f, 0xc7, 0xff, 0xc7, 0xe0, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0x1f, 0xc7, 0xff, 0xc7, 
	0xe0, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0xfc, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 
	0x00, 0x00, 0x03, 0xf8, 0xfc, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 
	0xfc, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0xfc, 0x07, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xf8, 0xfc, 0x07, 0xfc, 0x00, 0xff, 0xff, 0x03, 0xff, 
	0x00, 0x00, 0x1f, 0xc0, 0xfc, 0x07, 0xfc, 0x00, 0xff, 0xff, 0x03, 0xff, 0x00, 0x00, 0x1f, 0xc0, 
	0xfc, 0x07, 0xfc, 0x00, 0xff, 0xff, 0x03, 0xff, 0x00, 0x00, 0x1f, 0xc0, 0xfc, 0x00, 0x03, 0xc0, 
	0xfc, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x03, 0xc0, 0xfc, 0x00, 0x00, 0x3f, 
	0xe0, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x03, 0xc0, 0xfc, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0xfc, 0x00, 
	0xfc, 0x07, 0x03, 0xc0, 0xfc, 0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0xfc, 0x07, 0x03, 0xc0, 
	0xfc, 0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0xfc, 0x07, 0x03, 0xc0, 0xfc, 0x00, 0x00, 0x07, 
	0xff, 0xff, 0xe0, 0x00, 0xff, 0xc7, 0xfc, 0x07, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
	0xff, 0xc7, 0xfc, 0x07, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xc7, 0xfc, 0x07, 
	0xe0, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x1f, 0xf8, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x3f, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x03, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * @brief Singleton accessor for WEIGHTSHAKE.
 * @return Reference to the singleton instance.
 * @note This function uses the Meyers' singleton pattern to ensure that only one instance of WEIGHTSHAKE exists.
 *       The instance is created the first time this function is called and is destroyed when the program ends.
 */
WEIGHTSHAKE& WEIGHTSHAKE::getInstance() {
    static WEIGHTSHAKE instance;
    return instance;
}

/**
 * @brief Runs the minigame logic.
 * @note This function handles the game state transitions and updates the display accordingly.
 */
void WEIGHTSHAKE::runWeightShake( void ){
    //handle banner expirations first
    // Title Banner expired
    if (currentState == WSGameState::WS_TITLE_BANNER) {
        if (bannerTicker >= 100 + marqueeTicks) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            bannerTicker = 0;
            if (cpWeightLossMax == 0) {
                currentState = WSGameState::WS_DISPLAY_RESULT;
            } else {
                currentState = WSGameState::WS_INTERMEDIARY_BANNER;
            }
        }
    } else if (currentState == WSGameState::WS_INTERMEDIARY_BANNER) {
        if (buttonFlag == true) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            bannerTicker = 0;
            buttonFlag = false;
            currentState = WSGameState::WS_DISPLAY_RESULT;
        }
    } else if (currentState == WSGameState::WS_DISPLAY_RESULT) {
        if (bannerTicker >= 160) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            NeoController::getInstance().turnAllOff();
            doStop();
        }
    }
    //then print banners, start timers, handle nonbanner game states
    if (currentState == WSGameState::WS_TITLE_BANNER) {
        if (bannerTicker < 1) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.println();
            BDISPLAY::display.setTextSize(2);
            marqueeTicks = BDISPLAY::startFullscreenMarquee("WeightShake");
            BDISPLAY::display.display();
            NeoController::getInstance().changeYellow();
        } else if (BDISPLAY::marqueeTextState.active) {
            BDISPLAY::updateFullscreenMarquee();
        } else {
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            NeoController::getInstance().changeYellow();
            BDISPLAY::display.printf("Pump this\n lil' dumbbell! Then\n  do it again...\n\n");
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F("Get that beach bod!"));
            BDISPLAY::display.display();
        }
    bannerTicker++;
    } else if (currentState == WSGameState::WS_INTERMEDIARY_BANNER) {
        if (bannerTicker % 20 == 0) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setTextSize(1);
            NeoController::getInstance().changeYellow();
            BDISPLAY::display.drawBitmap(
                0, 0, // X, Y position
                bad_dumbbell_bitmap, // Current frame
                96, 48, // Width and height
                WHITE // Color
            );            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.setCursor(0, 45);
            BDISPLAY::display.printf("\n Pumped %u irons\n", pumpCount);
            BDISPLAY::display.display();
        }
            // Detect up-and-down motion
        float currentAccelZ = BadgeAcceler::getInstance().readFloatAccelZ();

        if (!isMovingUp && currentAccelZ > upThreshold) {
            // Detected upward motion
            isMovingUp = true;
        } else if (isMovingUp && currentAccelZ < downThreshold) {
            // Detected downward motion after upward motion
            isMovingUp = false;
            pumpCount++; // Increment pump count
        }

        // End the game after 250 pumps
        if (pumpCount >= 250) {
            buttonFlag = true;
        }
        bannerTicker++;
    } else if (currentState == WSGameState::WS_DISPLAY_RESULT) {
        if (CyberPartner::getInstance().getLifePhase() == LifePhase::Egg) {
            BDISPLAY::display.clearDisplay();
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.setCursor(0, 0);
            NeoController::getInstance().changeRed();
            BDISPLAY::display.printf("EGGS CAN'T   LIFT\n\n");
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.println(F(" Come back when\n     you grown!"));
            BDISPLAY::display.display();
        } else if (cpWeightLossMax == 0) {
            if (bannerTicker < 1) {
                BDISPLAY::clearAndDisplayBatteryPercentage();
                BDISPLAY::display.setTextColor(WHITE);
                BDISPLAY::display.setTextSize(1);
                BDISPLAY::display.setCursor(0, 0);
                NeoController::getInstance().changeRed();
                BDISPLAY::display.printf("\n Your CyberPartner's\n2 tiny 4 this gym!\n\n\n");
                BDISPLAY::display.println(F(" Eat more food"));
                BDISPLAY::display.display();
            }
        } else {
            if (bannerTicker < 1) {
                // 1g per 25 pumps or 10% max weight loss
                uint16_t weightLoss = 0;
                int newWeight = 0;
                weightLoss = std::min(pumpCount / 25, cpWeightLossMax);
                newWeight = std::max(0, CyberPartner::getInstance().getWeight() - weightLoss);
                CyberPartner::getInstance().setWeight(newWeight);

                BDISPLAY::clearAndDisplayBatteryPercentage();
                BDISPLAY::display.setTextColor(WHITE);
                BDISPLAY::display.setTextSize(1);
                BDISPLAY::display.setCursor(0, 0);
                NeoController::getInstance().changeGreen();
                BDISPLAY::display.println(F(" Game Complete!"));
                BDISPLAY::display.printf("\n Total Reps: %d\n", pumpCount);
                if (weightLoss != 0) {
                    NeoController::getInstance().changeGreen();
                    BDISPLAY::display.printf("Your Cyberpartner lost %dg weight!\n", weightLoss );
                } else {
                    NeoController::getInstance().changeYellow();
                    BDISPLAY::display.printf("Your Cyberpartner lost no weight.\n      Work Harder!" );
                }
                BDISPLAY::display.display();
            }
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
void WEIGHTSHAKE::update( void ){
    if (currentState != WSGameState::WS_DISABLED) {
        if (_gameTimer.check(WS_CHECK_TIME_INTERVAL)) {
            runWeightShake();
        }
    }
}

/**
 * @brief Starts the minigame.
 * @note This function disables the menu before initializing required variables to starting the game.
 */
void WEIGHTSHAKE::doStart( void ){
    BMenu::disableMenu();
    currentState = WSGameState::WS_TITLE_BANNER;
    bannerTicker = 0;
    buttonFlag = false;
    pumpCount = 0;
    cpWeightLossMax = CyberPartner::getInstance().getWeight() / 10;  // TODO : Set to 10% of cyberpartner weight
}

/**
 * @brief Stops the minigame.
 * @note This function should clear any game-related variables and re-enables the menu.
 */
void WEIGHTSHAKE::doStop( void ){
    currentState = WSGameState::WS_DISABLED;
    bannerTicker = 0;
    buttonFlag = false;
    pumpCount = 0;
    cpWeightLossMax = 0;


    NeoController::getInstance().turnAllOff();
    // TODO optional networking things here
    BMenu::enableMenu();  // enable menu since we are done
}

/**
 * @brief Checks if the minigame is currently running.
 * @return True if the game is running, false otherwise.
 */
bool WEIGHTSHAKE::isRunning( void ){
    if (currentState == WSGameState::WS_DISABLED) {
        return false;
    } else {
        return true;
    }
}

/**
 * @brief Handles the up button click for the minigame.
 */
void WEIGHTSHAKE::clickUp( void ){
    if (currentState == WSGameState::WS_TITLE_BANNER) {
        doStop();
    } else if (currentState == WSGameState::WS_DISPLAY_RESULT) {
        doStop();
    }
}

/**
 * @brief Handles the down button click for the minigame.
 */
void WEIGHTSHAKE::clickDown( void ){
    if (currentState == WSGameState::WS_TITLE_BANNER) {
        doStop();
    } else if (currentState == WSGameState::WS_DISPLAY_RESULT) {
        doStop();
    }
}

/**
 * @brief Handles the confirm button click for the minigame.
 */
void WEIGHTSHAKE::clickConfirm( void ){
    if (currentState == WSGameState::WS_TITLE_BANNER) {
        doStop();
    } else if (currentState == WSGameState::WS_INTERMEDIARY_BANNER) {
        buttonFlag = true;
    } else if (currentState == WSGameState::WS_DISPLAY_RESULT) {
        doStop();
    }
}

/**
 * @brief Handles the cancel button click for the minigame.
 */
void WEIGHTSHAKE::clickCancel( void ){
    if (currentState == WSGameState::WS_TITLE_BANNER) {
        doStop();
    } else if (currentState == WSGameState::WS_INTERMEDIARY_BANNER) {
        buttonFlag = true;
    } else if (currentState == WSGameState::WS_DISPLAY_RESULT) {
        doStop();
    }
}