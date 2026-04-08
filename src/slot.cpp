
// Standard Library Includes
#include <Arduino.h>

// Project-Specific Includes
#include "game_timer.h"
#include "slot.h"
#include "accelerometer.h"
#include "badge_display.h"
#include "badgeMenu.h"
#include "neocontroller.h"
#include "CPStore.h"
#include "haptic.h"

#define SLOT_CHECK_TIME_INTERVAL 25

SLOT& SLOT::getInstance() {
    static SLOT instance;
    return instance;
}

void SLOT::doStart()
{
    #ifdef BADGEDEBUG
        Serial.println(F("slot"));
    #endif //BADGEDEBUG
    // disable menu before starting
    BMenu::disableMenu();
    // Need networking things here

    NeoController::getInstance().changeGreen();
    game_state = SLOT_BANNER;
    game_result = GR_DISABLED;
    banner_delay_ticker = 0;
    instruct_delay_ticker = 0;
    result_delay_ticker = 0;

    topx=0;
    middlex=0;
    bottomx=0;
    setLevel(1);

    target=-1;
}

void SLOT::doStop()
{
    game_state = SLOT_DISABLED;
    banner_delay_ticker = 0;
    instruct_delay_ticker = 0;
    result_delay_ticker = 0;

    // enable menu since we are done
    BMenu::enableMenu();
    // Need networking things here
}

void SLOT::doClick()
{
    // Serial.println("click");
    if(game_state==SLOT_RUNNING){
        game_state=SLOT_COOLDOWN;
    }
}

bool SLOT::isRunning()
{
    if(game_state == SLOT_DISABLED){
        return false;
    } else {
        return true;
    }
}

void SLOT::setLevel(int16_t l)
{
    level=l;
    if(level<=4)
        speed=l*2;
    else if(level<=8)
        speed=l*1.5;
    else
        speed=l*1.25;

    topxDelta=-speed;
    middlexDelta=speed;
    bottomxDelta=-speed;
}

void SLOT::incLevel()
{
    setLevel(level+1);
}

void SLOT::finishGame(bool result){
    if(result == true){
        game_result = GR_WIN;

        if(level>0 && level<=2){
            reward = CPStore::getRewardAmount(CPStoreReward::REWARD_MIN);
            CPStore::earnReward(CPStoreReward::REWARD_MIN);
        }else if(level>2 && level<=4){
            reward = CPStore::getRewardAmount(CPStoreReward::REWARD_LOW);
            CPStore::earnReward(CPStoreReward::REWARD_LOW);
        }else if(level>4 && level<=6){
            reward = CPStore::getRewardAmount(CPStoreReward::REWARD_MED);
            CPStore::earnReward(CPStoreReward::REWARD_MED);
        }else if(level>6 && level<=8){
            reward = CPStore::getRewardAmount(CPStoreReward::REWARD_HIGH);
            CPStore::earnReward(CPStoreReward::REWARD_HIGH);
        }else if(level>8){
            reward = CPStore::getRewardAmount(CPStoreReward::REWARD_MAX);
            CPStore::earnReward(CPStoreReward::REWARD_MAX);
        }
        incLevel();
    } else {
        setLevel(1);
        game_result = GR_LOST;

    }
    game_state = SLOT_RESULT;
}

void SLOT::playGame(){
    int slowDownRamp = 50;
    if(game_state==SLOT_COOLDOWN){
        BadgeHaptic::getInstance().hapticOn();
        if(topxDelta!=0){

            if(target==-1){
                if(topx>203){
                    target=203;//50-mushroom, 125-star, 203-flower
                    topResult=FLOWER;
                }else if(topx>125){
                    target=125;
                    topResult=STAR;
                }else if(topx>50){
                    target=50;
                    topResult=MUSHROOM;
                }else{
                    target=203;
                    topResult=FLOWER;
                }
            }

            if(target<topx && topx-slowDownRamp<=target){
                topxDelta=(target-topx)/2;
            }

            if(topxDelta==0){
                game_state=SLOT_RUNNING;
                target=-1;
            }
        }else if(middlexDelta!=0){
            if(target==-1){
                if(middlex<50){
                    target=50;//50-mushroom, 125-star, 203-flower
                    middleResult=MUSHROOM;
                }else if(middlex<125){
                    target=125;
                    middleResult=STAR;
                }else if(middlex<203){
                    target=203;
                    middleResult=FLOWER;
                }else{
                    target=50;
                    middleResult=MUSHROOM;
                }
            }

            if(target>middlex && middlex+slowDownRamp>=target){
                middlexDelta=(target-middlex)/2;
            }

            if(middlexDelta==0){
                game_state=SLOT_RUNNING;
                target=-1;
            }
        }else if(bottomxDelta!=0){
            if(target==-1){
                if(bottomx>203){
                    target=203;//50-mushroom, 125-star, 203-flower
                    bottomResult=FLOWER;
                }else if(bottomx>125){
                    target=125;
                    bottomResult=STAR;
                }else if(bottomx>50){
                    target=50;
                    bottomResult=MUSHROOM;
                }else{
                    target=203;
                    bottomResult=FLOWER;
                }
            }

            if(target<bottomx && bottomx-slowDownRamp<=target){
                bottomxDelta=(target-bottomx)/2;
            }


        }else{
            BadgeHaptic::getInstance().hapticOff();
            #ifdef BADGEDEBUG
            Serial.print("result: ");
            Serial.print(topResult);
            Serial.print(", ");
            Serial.print(middleResult);
            Serial.print(", ");
            Serial.println(bottomResult);
            #endif //BADGEDEBUG

            if(topResult==middleResult && middleResult==bottomResult){
                finishGame(true);
            }
            // else if(topResult==middleResult){
            //     finishGame(true);
            // }else if(middleResult==bottomResult){
            //     finishGame(true);
            // }else if(topResult==bottomResult){
            //     finishGame(true);
            // }
            else{
                finishGame(false);
            }

            target=-1;
        }
    }else
        BadgeHaptic::getInstance().hapticOff();

    int16_t topWidth = 229;
    int16_t topHeight = 22;
    drawXbm(0,0,topWidth,topHeight,topx,0,epd_bitmap_Mario_3_sprites_top);
    topx+=topxDelta;
    if(topx<0)
        topx=topWidth;

    int16_t middleWidth = 229;
    int16_t middleHeight = 31;
    drawXbm(0,22,middleWidth,middleHeight,middlex,0,epd_bitmap_Mario_3_sprites_middle);
    middlex+=middlexDelta;
    if(middlex>=middleWidth)
        middlex=0;

    int16_t bottomWidth = 229;
    int16_t bottomHeight = 11;
    drawXbm(0,53,bottomWidth,bottomHeight,bottomx,0,epd_bitmap_Mario_3_sprites_bottom);
    bottomx+=bottomxDelta;
    if(bottomx<0)
        bottomx=bottomWidth;


    BDISPLAY::display.display();
}

void SLOT::runSlot()
{

    if(banner_delay_ticker >= 120){
        BDISPLAY::display.clearDisplay();
        BDISPLAY::display.display();
        game_state = SLOT_LEVEL;
    }
    if(instruct_delay_ticker >= 120){
        BDISPLAY::display.clearDisplay();
        BDISPLAY::display.display();
        game_state = SLOT_RUNNING;
    }
    if(result_delay_ticker >= 80){
        // doStop();
        game_state = SLOT_LEVEL;
        banner_delay_ticker = 0;
        instruct_delay_ticker = 0;
        result_delay_ticker = 0;
    }
    if(game_state == SLOT_BANNER){
        banner_delay_ticker++;
        if (banner_delay_ticker < 2) {
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.clearDisplay();
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.println();
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.println();
            BDISPLAY::display.println(F("      Super Cyber"));
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.println(F("   SLOT"));
            BDISPLAY::display.display();
        }
    } else if(game_state == SLOT_LEVEL){
        NeoController::getInstance().changeGreen();
        banner_delay_ticker = 0;
        instruct_delay_ticker++;
        if (instruct_delay_ticker < 2) {
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.clearDisplay();
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.print(F("       Level "));
            BDISPLAY::display.println(level);

            BDISPLAY::display.display();
        }
    } else if(game_state == SLOT_RUNNING || game_state==SLOT_COOLDOWN){
        banner_delay_ticker = 0;
        instruct_delay_ticker = 0;
        result_delay_ticker = 0;
        playGame();
    } else if(game_state == SLOT_RESULT){
        result_delay_ticker++;
        BDISPLAY::display.setTextSize(1);
        BDISPLAY::display.writeFillRect(24,28,85,16,0);
        BDISPLAY::display.setTextColor(WHITE);
        // BDISPLAY::display.clearDisplay();
        BDISPLAY::display.setCursor(0, 0);
        if(game_result == GR_WIN){
            NeoController::getInstance().changeConfetti();
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.print(F("     Winning +"));
            BDISPLAY::display.print(reward);
            BDISPLAY::display.println(F("c"));
        } else {
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F(""));
            BDISPLAY::display.println(F("      Try again"));
        }
        BDISPLAY::display.display();
        BDISPLAY::display.display();
    }
}

void SLOT::update()
{
    if (game_state != SLOT_DISABLED) {
        if (gameTimer.check(SLOT_CHECK_TIME_INTERVAL)) {
            runSlot();
        }
    }
}

void SLOT::setup()
{
    game_state = SLOT_DISABLED;
}

void SLOT::drawXbm(int16_t xMove, int16_t yMove, int16_t width, int16_t height, int16_t insetx, int16_t insety, const uint8_t *xbm){
    int16_t widthInXbm = (width + 7) / 8;
    uint8_t data = 0;

    for(int16_t y = 0; y < height; y++) {
      for(int16_t x = 0; x < width; x++ ) {
        if(xMove+x<=BDISPLAY::SCREEN_WIDTH && yMove+y<=BDISPLAY::SCREEN_HEIGHT){
            if (x & 7) {
                data >>= 1; // Move a bit
            } else {  // Read new data every 8 bit
                int16_t readx = x+(insetx);
                if(readx>=width-(width%8)){
                    readx = readx-width+(width%8);
                }
                int16_t ready = y+(insety);
                if(ready>=height)
                    ready = ready-height;
                data = pgm_read_byte(xbm + (readx / 8) + ready * widthInXbm);
            }
            // if there is a bit draw it
            if (data & 0x01) {
                BDISPLAY::display.drawPixel(xMove + x, yMove + y,WHITE);
            }else{
                BDISPLAY::display.drawPixel(xMove + x, yMove + y, BLACK);
            }
        }
      }
    }
}
