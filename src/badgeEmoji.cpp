#include "badge.h"
#include "badge_display.h"
#include "badgeEmoji_bitmaps.h"
#include "badgeMenu.h"

namespace EMOJI {
    #define EMOJI_START 250

    typedef enum {
        EMOJI_DISABLED = 0,
        EMOJI_DISPLAY_NOFRETCITIZEN = 1,
        EMOJI_DISPLAY_MIDDLEFINGER = 2,
        EMOJI_DISPLAY_TACO = 3,
        EMOJI_DISPLAY_WAVEHELLO = 4
    } EMOJI_STATE_t;
    static EMOJI_STATE_t image_display_state;
    uint8_t emoji_y = 255;
    uint8_t emoji_delay_ticker = 0;


    bool imageCheckTime( uint16_t duration )
    {
        static uint32_t previousImageMillis = 0;
        uint32_t curMillis = millis();
        if( curMillis - previousImageMillis >= duration) {
            previousImageMillis = curMillis;
            return true;
        } else {
            return false;
        }
    }

    void drawEmoji()
    {
        BDISPLAY::display.clearDisplay();
        if( image_display_state == EMOJI_DISPLAY_NOFRETCITIZEN ) {
            uint8_t width = 50;
            uint8_t height = 50;
            uint8_t x = BDISPLAY::SCREEN_WIDTH / 2 - width / 2;
            BDISPLAY::display.drawBitmap(x, emoji_y,  NoFretCitizen, width, height, WHITE);
        } else if ( image_display_state == EMOJI_DISPLAY_MIDDLEFINGER ) {
            uint8_t width = 64;
            uint8_t height = 64;
            uint8_t x = BDISPLAY::SCREEN_WIDTH / 2 - width / 2;
            BDISPLAY::display.drawBitmap(x, emoji_y,  MiddleFinger, width, height, WHITE);
        } else if ( image_display_state == EMOJI_DISPLAY_TACO ) {
            uint8_t width = 64;
            uint8_t height = 64;
            uint8_t x = BDISPLAY::SCREEN_WIDTH / 2 - width / 2;
            BDISPLAY::display.drawBitmap(x, emoji_y,  Taco, width, height, WHITE);
        }  else if ( image_display_state == EMOJI_DISPLAY_WAVEHELLO ) {
            uint8_t width = 64;
            uint8_t height = 64;
            uint8_t x = BDISPLAY::SCREEN_WIDTH / 2 - width / 2;
            BDISPLAY::display.drawBitmap(x, emoji_y,  WaveHello, width, height, WHITE);
        }
        BDISPLAY::display.display();
    }

    void drawNoFretCitizenImage( uint16_t x, uint16_t y )
    {
        BDISPLAY::display.drawBitmap(x, y,  NoFretCitizen, 50, 50, WHITE);
    }

    void startEmoji( void )
    {
        Serial.println("startEmoji()");
        BMenu::disableMenu();
        emoji_y = BDISPLAY::SCREEN_HEIGHT;
        emoji_delay_ticker = 255;
    }

    void drawNoFretCitizenEmoji( void )
    {
        startEmoji();
        image_display_state = EMOJI_DISPLAY_NOFRETCITIZEN;
    }

    void drawMiddleFingerEmoji( void )
    {
        startEmoji();
        image_display_state = EMOJI_DISPLAY_MIDDLEFINGER;
    }

    void drawTacoEmoji( void )
    {
        startEmoji();
        image_display_state = EMOJI_DISPLAY_TACO;
    }

    void drawWaveEmoji( void )
    {
        startEmoji();
        image_display_state = EMOJI_DISPLAY_WAVEHELLO;
    }

    void setup( void )
    {
        image_display_state = EMOJI_DISABLED;
        emoji_y = 255;
        emoji_delay_ticker = 255;
    }

    void updateMenu( void )
    {
        if( image_display_state ) {
            drawEmoji();
            if( emoji_delay_ticker == 0 ) {
                emoji_y = 255;
                emoji_delay_ticker = 255;
                image_display_state = EMOJI_DISABLED;
                BMenu::enableMenu();
            } else if( emoji_delay_ticker <= EMOJI_START ) {
                emoji_delay_ticker--;
            } else if( emoji_y == 0 ) {
                emoji_delay_ticker = EMOJI_START;
            } else if( emoji_y <= BDISPLAY::SCREEN_HEIGHT ) {
                emoji_y--;
            }
        }
}

}
