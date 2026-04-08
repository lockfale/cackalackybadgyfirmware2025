#include "badge_display.h"
#include "badge_bitmaps.h"
#include "battery_level.h"

namespace BDISPLAY {
    // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
    #define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    ScrollingBitmapState scrollingBitmapState;
    MarqueeTextState marqueeTextState;

    int16_t rotatedDegrees = 0; // Global variable to track the rotation state for achievement

    #define LOGO_WIDTH    128
    #define LOGO_HEIGHT   64

    void DisplayLogo( void ) {
        for(uint8_t i = 60; i > 0; i--) {
            clearAndDisplayBatteryPercentage();
            display.drawBitmap(0, i,  CackalackyCon_Logo, LOGO_WIDTH, LOGO_HEIGHT, WHITE);
            display.display();
            delay(20);
        }
        delay(2000);
    }

    void rotateDisplay() {
        BDISPLAY::display.setRotation((BDISPLAY::display.getRotation() + 2) % 4);
        rotatedDegrees += 180; // Update the global variable
    }

    void drawBitmapFlippedH(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h) {
        for (int16_t row = 0; row < h; row++) {
            for (int16_t col = 0; col < w; col++) {
                int16_t srcByteIndex = row * ((w + 7) / 8) + (col / 8);
                uint8_t srcBitMask = 0x80 >> (col % 8);

                // Safely read from PROGMEM
                uint8_t byteValue = pgm_read_byte(&bitmap[srcByteIndex]);

                int16_t flippedCol = (w - 1) - col;
                if (byteValue & srcBitMask) {
                    display.drawPixel(x + flippedCol, y + row, WHITE);
                }
            }
        }
    }

    void setup( void ) {
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println(F("SSD1306 allocation failed"));
        }

        // Clear the buffer
        clearAndDisplayBatteryPercentage();

        // Start scrolling the logo
        BDISPLAY::startScrollingBitmap(CackalackyCon_Logo, BDISPLAY::SCREEN_WIDTH, 0, LOGO_WIDTH, LOGO_HEIGHT, 1, 3000); // Hold for 3 seconds
    }

    void clearAndDisplayBatteryPercentage() {
        uint8_t batteryPercentage = BadgeBatteryReader::getInstance().readBatteryPercentage();
        display.clearDisplay();
        BDISPLAY::display.clearDisplay();
        int fillWidth = map(batteryPercentage, 0, 100, 0, 16);  // Adjust fill size
        BDISPLAY::display.fillRect(112, 1, fillWidth, 2, 1);  // Fill the inside
        BDISPLAY::display.drawRect(112, 0, 16, 4, 1);
    }

    void displayBatteryPercentage() {
        uint8_t batteryPercentage = BadgeBatteryReader::getInstance().readBatteryPercentage();
        int fillWidth = map(batteryPercentage, 0, 100, 0, 16);  // Adjust fill size
        BDISPLAY::display.fillRect(112, 1, fillWidth, 2, 1);  // Fill the inside
        BDISPLAY::display.drawRect(112, 0, 16, 4, 1);
    }

    void displayCenteredText(const char* text, uint8_t y, uint8_t textSize) {
        constexpr uint8_t charWidth = 6; // Width of a single character for default font
        uint8_t textLength = strlen_P(text); // Get the length of the text in PROGMEM
        uint8_t textWidth = textLength * charWidth * textSize;
        uint8_t x = (BDISPLAY::SCREEN_WIDTH - textWidth) / 2;

        BDISPLAY::display.setTextSize(textSize);
        BDISPLAY::display.setCursor(x, y);
        BDISPLAY::display.println(FPSTR(text));
    }

    void displayRightJustifiedText(const char* text, uint8_t rightX, uint8_t y, uint8_t textSize) {
        constexpr uint8_t charWidth = 6; // Width of a single character for default font
        uint8_t textLength = strlen_P(text); // Get the length of the text in PROGMEM
        uint8_t textWidth = textLength * charWidth * textSize;
        uint8_t x = rightX - textWidth; // Calculate the x-coordinate for right justification

        BDISPLAY::display.setTextSize(textSize);
        BDISPLAY::display.setCursor(x, y);
        BDISPLAY::display.println(FPSTR(text));
    }

    int startFullscreenMarquee(const char* text, uint8_t y, uint8_t textSize, uint16_t speed) {
        // Initialize the marquee state
        marqueeTextState.text = text;
        marqueeTextState.y = y;
        marqueeTextState.textSize = textSize;
        marqueeTextState.speed = speed > 0 ? speed : 1; // Ensure speed is at least 1
        marqueeTextState.x = SCREEN_WIDTH; // Start at the right edge
        marqueeTextState.active = true;

        // Calculate the total width of the text
        constexpr uint8_t charWidth = 6; // Width of a single character for the default font
        uint8_t textLength = strlen_P(text); // Get the length of the text in PROGMEM
        uint16_t textWidth = textLength * charWidth * textSize; // Calculate the total width of the text

        // Calculate the number of ticks required for the marquee to finish
        int ticksRequired = (SCREEN_WIDTH + textWidth) / speed;

        return ticksRequired;
    }

    void updateFullscreenMarquee() {
        if (!marqueeTextState.active || !marqueeTextState.text) {
            return; // No active marquee
        }

        constexpr uint8_t charWidth = 6; // Width of a single character for the default font
        uint8_t textLength = strlen_P(marqueeTextState.text); // Get the length of the text in PROGMEM
        uint16_t textWidth = textLength * charWidth * marqueeTextState.textSize; // Calculate the total width of the text

        // Clear the display
        BDISPLAY::display.clearDisplay();

        // Set text size and position
        BDISPLAY::display.setTextSize(marqueeTextState.textSize);
        BDISPLAY::display.setCursor(marqueeTextState.x, marqueeTextState.y);

        // Print the text
        BDISPLAY::display.print(FPSTR(marqueeTextState.text));

        // Update the display
        BDISPLAY::display.display();

        // Move the text to the left
        marqueeTextState.x -= marqueeTextState.speed;

        // Reset the marquee if it has completely exited the screen
        if (marqueeTextState.x + textWidth <= 0) {
            marqueeTextState.active = false; // Stop the marquee
        }
    }

    /**
     * @brief Starts scrolling a bitmap across the display.
     * @param bitmap Pointer to the bitmap data.
     * @param x Initial X position of the bitmap.
     * @param y Initial Y position of the bitmap.
     * @param width Width of the bitmap.
     * @param height Height of the bitmap.
     * @param speed Speed of scrolling (pixels per tick).
     * @param holdTime Time to hold the bitmap before clearing (in milliseconds).
     * @note The speed must be greater than 0. If a negative or zero speed is provided,
     *       it will default to 1 pixel per tick.
     *       The bitmap will scroll from right to left across the display.
     */
    int startScrollingBitmap(const uint8_t* bitmap, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed, uint32_t holdTime) {
        scrollingBitmapState.bitmap = bitmap;
        scrollingBitmapState.x = x;
        scrollingBitmapState.y = y;
        scrollingBitmapState.width = width;
        scrollingBitmapState.height = height;
        scrollingBitmapState.speed = speed > 0 ? speed : 1; // Ensure speed is at least 1
        scrollingBitmapState.active = true;
        scrollingBitmapState.holding = false;
        scrollingBitmapState.holdTime = holdTime;
        scrollingBitmapState.holdStart = 0;

    // Calculate the total distance the bitmap needs to scroll
    int totalDistance = x + width;

    // Calculate the number of scroll updates required
    int scrollUpdates = (totalDistance + speed - 1) / speed; // Round up to ensure full scroll

    // Calculate the number of hold updates (assuming 60 updates per second)
    int holdUpdates = (holdTime + 16) / 17; // Approximate 60Hz update frequency (16.67ms per frame)

    // Return the total number of updates
    return scrollUpdates + holdUpdates;
    }

    /**
     * @brief Updates the scrolling bitmap on the display.
     * @note This function should be called periodically to update the bitmap's position.
     *       It handles the scrolling and hold logic.
     *       If the bitmap is in the hold state, it will wait for the specified hold time
     *       before clearing the display and stopping the scrolling.
     */
    void updateScrollingBitmap() {
        if (!scrollingBitmapState.active) {
            return; // No active scrolling
        }

        if (scrollingBitmapState.holding) {
            // Check if the hold period has elapsed
            if (millis() - scrollingBitmapState.holdStart >= scrollingBitmapState.holdTime) {
                // End the hold state and clear the display
                scrollingBitmapState.active = false;
                scrollingBitmapState.holding = false;
                BDISPLAY::display.clearDisplay();
                BDISPLAY::display.display();
            }
            return; // Exit early while holding
        }

        // Clear the display
        BDISPLAY::display.clearDisplay();

        // Draw the bitmap at the current position
        BDISPLAY::display.drawBitmap(scrollingBitmapState.x, scrollingBitmapState.y, scrollingBitmapState.bitmap, scrollingBitmapState.width, scrollingBitmapState.height, WHITE);

        // Update the display
        BDISPLAY::display.display();

        // Move the bitmap to the left
        scrollingBitmapState.x -= scrollingBitmapState.speed;

        // Transition to the hold state when the bitmap exits the screen
        if (scrollingBitmapState.x+1 + scrollingBitmapState.width <= 0) {
            scrollingBitmapState.holding = true;
            scrollingBitmapState.holdStart = millis(); // Record the start time of the hold period
        }
    }

}
