
// Standard Library Includes
#include <Arduino.h>

// Project-Specific Includes
#include "game_timer.h"
#include "ballhole.h"
#include "accelerometer.h"
#include "badge_display.h"
#include "badgeMenu.h"
#include "neocontroller.h"
#include "CPStore.h"

#define BH_CHECK_TIME_INTERVAL 50

BH& BH::getInstance() {
    static BH instance;
    return instance;
}

void BH::doStart()
{
    #ifdef BADGEDEBUG
        Serial.println(F("ball hole"));
    #endif //BADGEDEBUG
    // disable menu before starting
    BMenu::disableMenu();
    // Need networking things here

    NeoController::getInstance().changeGreen();
    game_state = BH_BANNER;
    game_result = GR_DISABLED;
    banner_delay_ticker = 0;
    instruct_delay_ticker = 0;
    result_delay_ticker = 0;
    ballx = 16;
    bally = 16;
    ballDeltaBias = .10;
    ballxDelta = 0;
    ballyDelta = 0;
    level=1;
    highlevel=1;
}

void BH::doStop()
{
    game_state = BH_DISABLED;
    banner_delay_ticker = 0;
    instruct_delay_ticker = 0;
    result_delay_ticker = 0;

    // enable menu since we are done
    BMenu::enableMenu();
    // Need networking things here
}

bool BH::isRunning()
{
    if(game_state == BH_DISABLED){
        return false;
    } else {
        return true;
    }
}

void BH::finishGame(bool result){
    if(result == true){
        game_result = GR_WIN;

        #ifdef BADGEDEBUG
            Serial.print(F("ball bias "));
            Serial.println(ballDeltaBias);
        #endif //BADGEDEBUG
        if(ballDeltaBias+.20<1){
            ballDeltaBias=ballDeltaBias+.20;
        }else{
            ballDeltaBias=ballDeltaBias+((ballDeltaBias-1+.20)/3);
        }
        level+=1;
    } else {
        if(level<=1){

        }else if(level<=2){
            CPStore::earnReward(CPStoreReward::REWARD_MIN);
        }else if(level<=4){
            CPStore::earnReward(CPStoreReward::REWARD_LOW);
        }else if(level<=6){
            CPStore::earnReward(CPStoreReward::REWARD_MED);
        }else if(level<=8){
            CPStore::earnReward(CPStoreReward::REWARD_HIGH);
        }else{
            CPStore::earnReward(CPStoreReward::REWARD_MAX);
        }
        game_result = GR_LOST;
        ballDeltaBias=.1;
        highlevel=level;
        level=1;
    }
    game_state = BH_RESULT;
}

void BH::playGame(){
    float x = BadgeAcceler::getInstance().readFloatAccelX();
    float y = BadgeAcceler::getInstance().readFloatAccelY();

    float change_x = (x / BDISPLAY::SCREEN_WIDTH)+(ballxDelta*ballDeltaBias);
    float change_y = (y / BDISPLAY::SCREEN_HEIGHT)+(ballyDelta*ballDeltaBias);
    ballxDelta = change_x;
    ballyDelta = change_y;

    bool fg = false;

    if (ballx > center_x-2
      && ballx < center_x+2
      && bally > center_y-2
      && bally < center_y+2){
        finishGame(true);
    }

    BDISPLAY::display.fillCircle(ballx, bally, 3, BLACK);

    if(x < -10){
        ballx = ballx + abs(change_x);
    }
    if(x > 10){
        ballx = ballx - change_x;
    }
    if(y < -10){
        bally = bally - abs(change_y);
    }
    if(y > 10){
        bally = bally + change_y;
    }

    if (ballx < 5) {
        ballx = 5;
        fg = true;
    } else if (ballx > BDISPLAY::SCREEN_WIDTH-5) {
        ballx = BDISPLAY::SCREEN_WIDTH-5;
        fg = true;
    } else if (bally < 5) {
        bally = 5;
        fg = true;
    } else if (bally > BDISPLAY::SCREEN_HEIGHT-5) {
        bally = BDISPLAY::SCREEN_HEIGHT-5;
        fg = true;
    }
    if (fg == true){
        finishGame(false);
    }

    BDISPLAY::display.drawCircle(center_x, center_y, 3, WHITE);
    BDISPLAY::display.fillCircle(ballx, bally, 3, WHITE);
    BDISPLAY::display.drawRect(0,0,BDISPLAY::SCREEN_WIDTH,BDISPLAY::SCREEN_HEIGHT,WHITE);
    BDISPLAY::display.display();
}

void BH::runBallHole()
{
    if(banner_delay_ticker >= 60){
        BDISPLAY::display.clearDisplay();
        BDISPLAY::display.display();
        game_state = BH_INSTRUCT;
    }
    if(instruct_delay_ticker >= 60){
        BDISPLAY::display.clearDisplay();
        BDISPLAY::display.display();
        game_state = BH_RUNNING;
    }
    if(result_delay_ticker >= 60){
        // doStop();
        game_state = BH_INSTRUCT;
        banner_delay_ticker = 0;
        instruct_delay_ticker = 0;
        result_delay_ticker = 0;

        ballx = 16;
        bally = 16;
    }
    if(game_state == BH_BANNER){
        banner_delay_ticker++;
        if (banner_delay_ticker < 2) {
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.clearDisplay();
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.println();
            BDISPLAY::display.println(F(" BallHole"));
            BDISPLAY::display.display();
        }
    } else if(game_state == BH_INSTRUCT){
        banner_delay_ticker = 0;
        instruct_delay_ticker++;
        if (instruct_delay_ticker < 2) {
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.clearDisplay();
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.print(F("Level "));
            BDISPLAY::display.println(level);
            BDISPLAY::display.println(F("1) Lay badge flat"));
            BDISPLAY::display.println(F("2) Try to get ball"));
            BDISPLAY::display.println(F("     in the hole"));
            BDISPLAY::display.println(F("3) If screen flashes"));
            BDISPLAY::display.println(F("     and you lose.."));
            BDISPLAY::display.println(F("     Go to step 1"));
            BDISPLAY::display.display();
        }
    } else if(game_state == BH_RUNNING){
        banner_delay_ticker = 0;
        instruct_delay_ticker = 0;
        result_delay_ticker = 0;
        playGame();
    } else if(game_state == BH_RESULT){
        result_delay_ticker++;
        BDISPLAY::display.setTextSize(2);
        BDISPLAY::display.setTextColor(WHITE);
        BDISPLAY::display.clearDisplay();
        BDISPLAY::display.setCursor(0, 0);
        if(game_result == GR_WIN){
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F("  Winning"));
        } else {
            BDISPLAY::display.println(F(" Try again"));
            BDISPLAY::display.setTextSize(1);
            if(highlevel<=1){
                BDISPLAY::display.printf("No Award");
            }else if(highlevel<=2){
                BDISPLAY::display.printf("Level %d Awarded $%d\n", highlevel-1, CPStore::getRewardAmount(CPStoreReward::REWARD_MIN));
            }else if(highlevel<=4){
                BDISPLAY::display.printf("Level %d Awarded $%d\n", highlevel-1,CPStore::getRewardAmount(CPStoreReward::REWARD_LOW));
            }else if(highlevel<=6){
                BDISPLAY::display.printf("Level %d Awarded $%d\n", highlevel-1,CPStore::getRewardAmount(CPStoreReward::REWARD_MED));
            }else if(highlevel<=8){
                BDISPLAY::display.printf("Level %d Awarded $%d\n", highlevel-1,CPStore::getRewardAmount(CPStoreReward::REWARD_HIGH));
            }else{
                BDISPLAY::display.printf("Level %d Awarded $%d\n", highlevel-1,CPStore::getRewardAmount(CPStoreReward::REWARD_MAX));
            }

        }
        BDISPLAY::display.display();
    }
}

void BH::update()
{
    if (game_state != BH_DISABLED) {
        if (gameTimer.check(BH_CHECK_TIME_INTERVAL)) {
            runBallHole();
        }
    }
}

void BH::setup()
{
    game_state = BH_DISABLED;
}
