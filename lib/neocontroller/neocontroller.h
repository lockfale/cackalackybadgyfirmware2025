/**
 * @file neocontroller.h
 * @brief Header file for the NeoController class.
 *
 * This file contains the declaration of the NeoController class, which is a singleton
 * class designed to manage NeoPixel LED operations. The class provides various methods
 * to control NeoPixel LEDs, including setting colors, patterns, and brightness. It uses
 * the FastLED library and TaskScheduler for efficient LED updates and background processing.
 *
 * The file also includes necessary macro definitions and library inclusions required for
 * the NeoController class to function correctly.
 *
 * @note Uncomment the NEODEBUG macro to enable debugging.
 */
#pragma once

#define FASTLED_INTERNAL
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>

//#define NEODEBUG    // uncomment to enable neo debugging

#define NEO_PIN 14
#define NEO_COUNT 4

/**
 * @class NeoController
 * @brief Singleton class to manage NeoPixel LED operations.
 *
 * The NeoController class provides various methods to control NeoPixel LEDs,
 * including setting colors, patterns, and brightness. It uses the FastLED library
 * and TaskScheduler for efficient LED updates and background processing.
 */
class NeoController {
public:
    /**
     * @brief Retrieves the singleton instance of the NeoController class.
     *
     * This function provides access to the single instance of the `NeoController` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `NeoController` instance.
     *
     * @details
     * - The `NeoController` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * NeoController& neoInstance = NeoController::getInstance();
     * neoInstance.setup();
     * ```
     */
    static NeoController& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    NeoController(const NeoController&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    NeoController& operator=(const NeoController&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    NeoController(NeoController&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    NeoController& operator=(NeoController&&) = delete;

    /**
     * @brief Setup the NeoController.
     */
    static void setup( void );

    /**
     * @brief Toggle the brightness of the LEDs.
     */
    static void toggleBrightness( void );

    /**
     * @brief Print the current brightness level.
     */
    static void printBrightness( void );

    /**
     * @brief Turn all LEDs off.
     */
    static void turnAllOff( void );

    /**
     * @brief Change the LED color to blue.
     */
    static void changeBlue( void );

    /**
     * @brief Change the LED color to green.
     */
    static void changeGreen( void );

    /**
     * @brief Change the LED color to pink.
     */
    static void changePink( void );

    /**
     * @brief Change the LED color to red.
     */
    static void changeRed( void );

    /**
     * @brief Change the LED color to yellow.
     */
    static void changeYellow( void );

    /**
     * @brief Change the LED color to white.
     */
    static void changeWhite( void );

    /**
     * @brief Change the LED pattern to chase.
     */
    static void changeChase( void );

    /**
     * @brief Change the LED pattern to confetti.
     */
    static void changeConfetti( void );

    /**
     * @brief Change the LED pattern to O'Grady.
     */
    static void changeOgrady( void );

    /**
     * @brief Change the LED pattern to O'Grady (top left).
     */
    static void changeOgradyTopLeft( void );

    /**
     * @brief Change the LED pattern to O'Grady (top right).
     */
    static void changeOgradyTopRight( void );

    /**
     * @brief Change the LED pattern to O'Grady (bottom left).
     */
    static void changeOgradyBottomLeft( void );

    /**
     * @brief Change the LED pattern to O'Grady (bottom right).
     */
    static void changeOgradyBottomRight( void );

    /**
     * @brief Change the LED pattern to Roulotto.
     */
    static void changeRoulotto( void );

    /**
     * @brief Change the LED pattern to Popo.
     */
    static void changePopo( void );

    /**
     * @brief Change the LED pattern to rainbow.
     */
    static void changeRainbow( void );

    /**
     * @brief Change the LED pattern to strobe.
     */
    static void changeStrobe( void );

    /**
     * @brief Change the LED pattern to surge.
     */
    static void changeSurge( void );

    /**
     * @brief Change the LED pattern to smooth chase.
     */
    static void changeSmoothChase( void );

protected:
    /**
     * @brief Default destructor for the NeoController class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~NeoController() = default;

private:
    /**
     * @brief Default constructor for the NeoController class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    NeoController() = default;

    static CRGB leds[NEO_COUNT]; /**< Array of CRGB objects for the LEDs */
    static uint8_t hue;        /**< Current hue value for color selection */
    static uint32_t lastUpdate; /**< Timestamp of the last update */

    static uint8_t gHue;       /**< Global hue value for color selection */
    static uint8_t patternPosition; /**< Current position in the pattern */
    static unsigned long previousPatternMillis; /**< Timestamp for pattern timing */

    static uint8_t brightnessLow; /**< Low brightness level */
    static uint8_t brightnessHigh; /**< High brightness level */

    /**
     * @enum Color
     * @brief Enumeration of possible colors.
     */
    enum class Color {
        None,
        Blue,
        Green,
        Pink,
        Red,
        Yellow,
        White
    };
    static Color curColor; /**< Current color of the LEDs */

    /**
     * @enum Pattern
     * @brief Enumeration of possible patterns.
     */
    enum class Pattern {
        None,
        Chase,
        Confetti,
        Ogrady,
        OgradyTopLeft,
        OgradyTopRight,
        OgradyBottomLeft,
        OgradyBottomRight,
        Roulotto,
        Popo,
        Rainbow,
        Strobe,
        Surge,
        SmoothChase
    };
    static Pattern curPattern; /**< Current pattern of the LEDs */

    /**
     * @brief Task scheduler for handling timed events and background processing.
     *
     * The `taskRunner` is responsible for managing scheduled
     * tasks that execute neopixel-related logic, ensuring efficiency in the
     * embedded environment.
     */
    static Scheduler& taskRunner;

    static Task t_neoLed; /**< Task for updating the neopixels */

    /**
     * @brief Check if the specified duration has passed.
     * @param duration Duration to check.
     * @return True if the duration has passed, false otherwise.
     */
    static bool checkTime ( uint16_t duration );

    /**
     * @brief Set the brightness of the LEDs.
     */
    static void setBrightness( void );

    /**
     * @brief Fade all LEDs to a specified value.
     * @param value Brightness value to fade to (0-255).
     */
    static void fadeAll( uint8_t value );

    /**
     * @brief Fill all LEDs with a specified color.
     * @param color Color to fill the LEDs with.
     */
    static void fillAll( CRGB color );

    /**
     * @brief Set the color mode.
     */
    static void setColorMode( void );

    /**
     * @brief Set the pattern mode.
     */
    static void setPatternMode( void );

    /**
     * @brief Get the color corresponding to the specified enum value.
     * @param color Color enum value.
     * @return Corresponding CRGB color.
     */
    static CRGB getColorFromEnum(Color color);

    /**
     * @brief Set the current color.
     * @param c Color to set.
     */
    static void setColor( Color c );

    /**
     * @brief Set the current pattern.
     * @param p Pattern to set.
     */
    static void setPattern( Pattern p );

    /**
     * @brief Callback for chase pattern.
     */
    static void chasePatternCallback( void );

    /**
     * @brief Callback for confetti pattern.
     */
    static void confettiPatternCallback( void );

    /**
     * @brief Callback for O'Grady pattern.
     */
    static void ogradyPatternCallback( void );

    /**
     * @brief Callback for O'Grady (top left) pattern.
     */
    static void ogradyTopLeftCallback(void);

    /**
     * @brief Callback for O'Grady (top right) pattern.
     */
    static void ogradyTopRightCallback(void);

    /**
     * @brief Callback for O'Grady (bottom left) pattern.
     */
    static void ogradyBottomLeftCallback(void);

    /**
     * @brief Callback for O'Grady (bottom right) pattern.
     */
    static void ogradyBottomRightCallback(void);

    /**
     * @brief Callback for Roulotto pattern.
     */
    static void roulottoPatternCallback(void);

    /**
     * @brief Callback for Popo pattern.
     */
    static void popoPatternCallback(void);

    /**
     * @brief Callback for rainbow pattern.
     */
    static void rainbowPatternCallback(void);

    /**
     * @brief Callback for strobe pattern.
     */
    static void strobePatternCallback(void);

    /**
     * @brief Callback for surge pattern.
     */
    static void surgePatternCallback(void);

    /**
     * @brief Draw a fractional bar. Callback for smooth chase pattern.
     * @param pos16 Position.
     * @param width Width.
     * @param hue Hue.
     */
    static void drawFractionalBar( int pos16, int width, uint8_t hue );

    /**
     * @brief Callback for smooth chase pattern.
     */
    static void smoothChasePatternCallback( void );

    /**
     * @brief Enable the pattern task.
     */
    static void enablePatternTask( void );

    /**
     * @brief Disable the pattern task.
     */
    static void disablePatternTask( void );
};
