#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
/*
    configData.h -- part of the cackalackabadgy2023 project.
    Function declarations for interacting with the display
*/
namespace BDISPLAY {

    constexpr uint8_t SCREEN_WIDTH = 128;
    constexpr uint8_t SCREEN_HEIGHT = 64;
    extern Adafruit_SSD1306 display;

    extern int16_t rotatedDegrees; // Global variable to track the rotation state for achievement

    // State tracker for the Marquee
    struct MarqueeTextState {
        const char* text = nullptr;
        uint8_t y = 0;
        uint8_t textSize = 1;
        uint16_t speed = 1;
        int16_t x = SCREEN_WIDTH; // Start at the right edge of the screen
        bool active = false; // Whether the marquee is currently active
    };
    extern MarqueeTextState marqueeTextState;

    int startFullscreenMarquee(const char* text, uint8_t y = 32, uint8_t textSize = 4, uint16_t speed = 5);
    void updateFullscreenMarquee();

    struct ScrollingBitmapState {
        const uint8_t* bitmap = nullptr; // Pointer to the bitmap data
        int16_t x = 0; // X position of the bitmap
        int16_t y = 0; // Y position of the bitmap
        int16_t width = 0; // Width of the bitmap
        int16_t height = 0; // Height of the bitmap
        int16_t speed = 1; // Speed of scrolling (pixels per tick)
        bool active = false; // Whether the scrolling is active
        bool holding = false; // Whether the bitmap is in the hold state
        uint32_t holdTime = 2000; // Hold time in milliseconds
        uint32_t holdStart = 0; // Timestamp when the hold started
    };
    extern ScrollingBitmapState scrollingBitmapState;

    int startScrollingBitmap(const uint8_t* bitmap, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed, uint32_t holdTime);
    void updateScrollingBitmap();

    void setup();
    void rotateDisplay();
    void drawBitmapFlippedH(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h);
    void clearAndDisplayBatteryPercentage();
    void displayBatteryPercentage();
    void displayCenteredText(const char* text, uint8_t y, uint8_t textSize);
    void displayRightJustifiedText(const char* text, uint8_t rightX, uint8_t y, uint8_t textSize);



}
