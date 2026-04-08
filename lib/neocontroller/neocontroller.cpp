/**
 * @file neocontroller.cpp
 * @brief Implementation file for the NeoController class.
 *
 * This file contains the implementation of the NeoController class, which is a singleton
 * class designed to manage NeoPixel LED operations. The class provides various methods
 * to control NeoPixel LEDs, including setting colors, patterns, and brightness. It uses
 * the FastLED library and TaskScheduler for efficient LED updates and background processing.
 *
 * The file includes the definitions of the methods declared in the neocontroller.h header file.
 * It also initializes static member variables and sets up the task scheduler for handling
 * timed events and background processing.
 */
#include "neocontroller.h"

extern Scheduler baseTaskRunner;
Scheduler& NeoController::taskRunner = baseTaskRunner;
Task NeoController::t_neoLed(10, TASK_FOREVER, &chasePatternCallback);

uint8_t NeoController::hue = 0;
uint32_t NeoController::lastUpdate = 0;

uint8_t NeoController::gHue = 0;
uint8_t NeoController::patternPosition = 0;
unsigned long NeoController::previousPatternMillis = 0;

uint8_t NeoController::brightnessLow = 64;
uint8_t NeoController::brightnessHigh = 255;

NeoController::Color NeoController::curColor = Color::None;
NeoController::Pattern NeoController::curPattern = Pattern::None;

CRGB NeoController::leds[NEO_COUNT];

// Static method to access the singleton instance
NeoController& NeoController::getInstance() {
    static NeoController instance;
    return instance;
}

bool NeoController::checkTime ( uint16_t duration )
{
    uint32_t curMillis = millis();
    if( curMillis - previousPatternMillis >= duration) {
        previousPatternMillis = curMillis;
        return true;
    }
    return false;
}

/******************************************************
 *
 * Brightness Functions
 *
 ******************************************************/
void NeoController::setBrightness( void )
{
    FastLED.setBrightness(brightnessLow);
}

void NeoController::toggleBrightness( void )
{
    if(FastLED.getBrightness() == brightnessHigh) {
        FastLED.setBrightness( brightnessLow);
    } else {
        FastLED.setBrightness( brightnessHigh);
    }
}

void NeoController::printBrightness( void )
{
    Serial.print(F("NEO: Brightness: "));
    Serial.println(FastLED.getBrightness());
}

void NeoController::fadeAll(uint8_t value) {
    for (uint8_t i = 0; i < NEO_COUNT; i++) {
        leds[i].nscale8(value);
    }
    FastLED.show();
}

void NeoController::fillAll(CRGB color) {
    for (uint8_t i = 0; i < NEO_COUNT; i++) {
        leds[i] = color;
    }
    FastLED.show();
}

void NeoController::enablePatternTask( void )
{
    #ifdef NEODEBUG
    Serial.print(F("NEO: enablingPatternTask(): "));
    #endif
    if (!t_neoLed.isEnabled()) {
        #ifdef NEODEBUG
        Serial.print(F("Enabling"));
        #endif
        taskRunner.addTask(t_neoLed);
        t_neoLed.enable();
    }
    Serial.println();
}

void NeoController::disablePatternTask( void )
{
    #ifdef NEODEBUG
    Serial.print(F("NEO: disablingPatternTask(): "));
    #endif
    if (t_neoLed.isEnabled()) {
        #ifdef NEODEBUG
        Serial.print(F("Disabling"));
        #endif
        t_neoLed.disable();
        taskRunner.deleteTask(t_neoLed);
    }

    #ifdef NEODEBUG
    if (!t_neoLed.isEnabled()) {
        Serial.println(F("Nothing to do"));
    }
    #endif
}

void NeoController::turnAllOff( void )
{
    curColor = Color::None;
    curPattern = Pattern::None;
    disablePatternTask();
    fillAll(CRGB::Black);
}


/******************************************************
 *
 * Colors
 *
 ******************************************************/
CRGB NeoController::getColorFromEnum(Color color) {
    switch (color) {
        case Color::Blue: return CRGB::Blue;
        case Color::Green: return CRGB::Green;
        case Color::Pink: return CRGB(0xff, 0x14, 0x93);
        case Color::Red: return CRGB::Red;
        case Color::Yellow: return CRGB::Yellow;
        case Color::White: return CRGB::White;
        default: return CRGB::Black;
    }
}

void NeoController::setColor( Color c )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: setColor()"));
    #endif
    if (curColor == c) return;

    disablePatternTask();
    turnAllOff();

    setBrightness();
    curColor = c;
    curPattern = Pattern::None;
    fillAll(getColorFromEnum(c));
}

/* *** Color: Pink *** */
void NeoController::changePink( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changePink()"));
    #endif
    setColor(Color::Pink);
}

/* *** Color: Red *** */
void NeoController::changeRed( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeRed()"));
    #endif
    setColor(Color::Red);
}

/* *** Color: Green *** */
void NeoController::changeGreen( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeGreen()"));
    #endif
    setColor(Color::Green);
}

/* *** Color: Blue *** */
void NeoController::changeBlue( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeBlue()"));
    #endif
    setColor(Color::Blue);
}

/* *** Color: Yellow *** */
void NeoController::changeYellow( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeYellow()"));
    #endif
    setColor(Color::Yellow);
}

/* *** Color: White *** */
void NeoController::changeWhite( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeWhite()"));
    #endif
    setColor(Color::White);
    setBrightness(); // Leave brightness low, this is blinding
}

/******************************************************
 *
 * Patterns
 *
 ******************************************************/

/* *** setPattern *** */
void NeoController::setPattern( Pattern p )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: setPattern()"));
    #endif
    if (curPattern == p) return;

    patternPosition = 0;
    curPattern = p;
    curColor = Color::None;
    setBrightness();

    if (p==Pattern::None) {
        curPattern = Pattern::Rainbow;
    } else if (p==Pattern::Surge) {
        fillAll(CRGB::Blue);
    }

    #ifdef NEODEBUG
    Serial.print(F("NEO: t_neoLed.getId(): "));
    Serial.println(t_neoLed.getId());
    Serial.print(F("NEO: Before, t_neoLed.isEnabled(): "));
    Serial.println(t_neoLed.isEnabled());
    #endif

    enablePatternTask();
}

/* *** Pattern: Chase *** */
void NeoController::changeChase( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeChase()"));
    #endif
    t_neoLed.setCallback(&chasePatternCallback);
    setPattern(Pattern::Chase);
}

void NeoController::chasePatternCallback( void )
{
    if (checkTime(100)) {
        leds[patternPosition] = CHSV(gHue++, 255, 255);
        if (patternPosition > 0) {
            patternPosition--;
        } else {
            patternPosition = NEO_COUNT - 1;
        }
    }
    FastLED.show();
    fadeAll(180);
}

/* *** Pattern: Confetti *** */
void NeoController::changeConfetti( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeConfetti()"));
    #endif
    t_neoLed.setCallback(&confettiPatternCallback);
    setPattern(Pattern::Confetti);
}

void NeoController::confettiPatternCallback( void )
{
    if (checkTime(50)) {
        uint8_t pos = random8(NEO_COUNT);
        leds[pos] = CHSV(gHue + random8(64), 255, 255);
    }
    FastLED.show();
    fadeAll(180);
}

/* *** Pattern: Ogrady *** */
void NeoController::changeOgrady( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeOgrady()"));
    #endif
    t_neoLed.setCallback(&ogradyPatternCallback);
    setPattern(Pattern::Ogrady);
}

void NeoController::ogradyPatternCallback( void )
{
    if (checkTime(30)) {
        if (patternPosition == 0) {
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Black;
            leds[2] = CRGB::Yellow;
            leds[3] = CRGB::Black;
        } else if (patternPosition == 16) {
            leds[0] = CRGB::Green;
            leds[1] = CRGB::Black;
            leds[2] = CRGB::Black;
            leds[3] = CRGB::Black;
        } else if (patternPosition == 32) {
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Blue;
            leds[2] = CRGB::Black;
            leds[3] = CRGB::Black;
        } else if (patternPosition == 48) {
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Black;
            leds[2] = CRGB::Black;
            leds[3] = CRGB::Red;
        }
        if (patternPosition < 64) {
            patternPosition++;
        } else {
            patternPosition = 0;
        }
    }
    FastLED.show();
}

/* *** Pattern: Ogrady TopLeft*** */
void NeoController::changeOgradyTopLeft( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeOgradyTopLeft()"));
    #endif
    t_neoLed.setCallback(&ogradyTopLeftCallback);
    setPattern(Pattern::OgradyTopLeft);
}

void NeoController::ogradyTopLeftCallback( void )
{
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Yellow;
    leds[3] = CRGB::Black;
    FastLED.show();
}

/* *** Pattern: Ogrady TopRight*** */
void NeoController::changeOgradyTopRight( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeOgradyTopRight()"));
    #endif
    t_neoLed.setCallback(&ogradyTopRightCallback);
    setPattern(Pattern::OgradyTopRight);
}

void NeoController::ogradyTopRightCallback( void )
{
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Blue;
    leds[2] = CRGB::Black;
    leds[3] = CRGB::Black;
    FastLED.show();
}

/* *** Pattern: Ogrady BottomLeft*** */
void NeoController::changeOgradyBottomLeft( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeOgradyBottomLeft()"));
    #endif
    t_neoLed.setCallback(&ogradyBottomLeftCallback);
    setPattern(Pattern::OgradyBottomLeft);
}

void NeoController::ogradyBottomLeftCallback( void )
{
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Black;
    leds[3] = CRGB::Red;
    FastLED.show();
}

/* *** Pattern: Ogrady BottomRight*** */
void NeoController::changeOgradyBottomRight( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeOgradyBottomRight()"));
    #endif
    t_neoLed.setCallback(&ogradyBottomRightCallback);
    setPattern(Pattern::OgradyBottomRight);
}

void NeoController::ogradyBottomRightCallback( void )
{
    leds[0] = CRGB::Green;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Black;
    leds[3] = CRGB::Black;
    FastLED.show();
}

/* *** Pattern: Roulotto *** */
void NeoController::changeRoulotto( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeRoulotto()"));
    #endif
    t_neoLed.setCallback(&roulottoPatternCallback);
    setPattern(Pattern::Roulotto);
}

void NeoController::roulottoPatternCallback( void )
{
    if (checkTime(10)) {
        if (patternPosition == 0) {
            leds[0] = CRGB::Green;
            leds[1] = CRGB::White;
            leds[2] = CRGB::Green;
            leds[3] = CRGB::Red;
        } else if (patternPosition == 16) {
            leds[0] = CRGB::White;
            leds[1] = CRGB::Green;
            leds[2] = CRGB::Red;
            leds[3] = CRGB::Green;
        } else if (patternPosition == 32) {
            leds[0] = CRGB::Green;
            leds[1] = CRGB::Red;
            leds[2] = CRGB::Green;
            leds[3] = CRGB::White;
        } else if (patternPosition == 48) {
            leds[0] = CRGB::Red;
            leds[1] = CRGB::Green;
            leds[2] = CRGB::White;
            leds[3] = CRGB::Green;
        }
        if (patternPosition < 64) {
            patternPosition++;
        } else {
            patternPosition = 0;
        }
    }
    FastLED.show();
}

/* *** Pattern: Popo *** */
void NeoController::changePopo( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changePopo()"));
    #endif
    t_neoLed.setCallback(&popoPatternCallback);
    setPattern(Pattern::Popo);
}

void NeoController::popoPatternCallback( void )
{
    if (checkTime(30)) {
        if (patternPosition == 0 || patternPosition == 3) {
            leds[0] = CRGB::Red;
            leds[1] = CRGB::Red;
            leds[2] = CRGB::Blue;
            leds[3] = CRGB::Blue;
        } else if (patternPosition == 10 || patternPosition == 13) {
            leds[0] = CRGB::Blue;
            leds[1] = CRGB::Blue;
            leds[2] = CRGB::Red;
            leds[3] = CRGB::Red;
        }
        if (patternPosition < 20) {
            patternPosition++;
        } else {
            patternPosition = 0;
        }
    }
    FastLED.show();
    fadeAll(180);
}

/* *** Pattern: Rainbow *** */
void NeoController::changeRainbow( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeRainbow()"));
    #endif
    t_neoLed.setCallback(&rainbowPatternCallback);
    setPattern(Pattern::Rainbow);
}

void NeoController::rainbowPatternCallback( void )
{
    if (checkTime(50)) {
        fill_rainbow( leds, NEO_COUNT, gHue++, 7);
        FastLED.show();
    }
}

/* *** Pattern: Strobe *** */
void NeoController::changeStrobe( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeStrobe()"));
    #endif
    t_neoLed.setCallback(&strobePatternCallback);
    setPattern(Pattern::Strobe);
}

void NeoController::strobePatternCallback( void )
{
    if (checkTime(50)) {
        if (patternPosition < 20 && patternPosition % 2 == 0) {
            for( uint8_t i = 0; i<NEO_COUNT; i++){
                leds[i] = CRGB(0xff, 0xff, 0xff);
            }
        }
        if (patternPosition + 1 < 50) {
            patternPosition++;
        } else {
            patternPosition = 0;
        }
    }
    FastLED.show();
    fadeAll(120);
}

/* *** Pattern: Surge *** */
void NeoController::changeSurge( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeSurge()"));
    #endif
    t_neoLed.setCallback(&surgePatternCallback);
    setPattern(Pattern::Surge);
}

void NeoController::surgePatternCallback( void )
{
    if (checkTime(100)) {
        for( uint8_t j = 0; j < NEO_COUNT; j++) {
            if (patternPosition < 10) {
                leds[j].nscale8(200);
            } else {
                leds[j] += leds[j].scale8(250);
            }
        }
        FastLED.show();
        if (patternPosition  < 20) {
            patternPosition++;
        } else {
            patternPosition = 0;
        }
    }
}

/* *** Pattern: Smooth Chase *** */
void NeoController::changeSmoothChase( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: changeSmoothChase()"));
    #endif
    t_neoLed.setCallback(&smoothChasePatternCallback);
    setPattern(Pattern::SmoothChase);
}

void NeoController::drawFractionalBar( int pos16, int width, uint8_t hue )
{
    int i = pos16 / 16; // convert from pos to raw pixel number
    uint8_t frac = pos16 & 0x0F; // extract the 'fractional' part of the position

    uint8_t firstpixelbrightness = 255 - (frac * 16);
    uint8_t lastpixelbrightness  = 255 - firstpixelbrightness;
    uint8_t bright;
    for( int n = 0; n <= width; n++) {
        if( n == 0) {
            bright = firstpixelbrightness;
        } else if( n == width ) {
            bright = lastpixelbrightness;
        } else {
            bright = 255;
        }
        leds[i] += CHSV( hue, 255, bright);
        i++;
        if( i == NEO_COUNT) i = 0; // wrap around
    }
}

struct SmoothChasePatternState {
    uint16_t lb16pos = 0;
    uint16_t lb16delta = 8;
    uint8_t lbhue16 = 0;
    uint8_t lbWidth = 3;
};
SmoothChasePatternState smoothChaseState;

void NeoController::smoothChasePatternCallback( void )
{
    if (checkTime(40)) {
        // Update the "Fraction Bar" by 1/16th pixel every time
        smoothChaseState.lb16pos += smoothChaseState.lb16delta;

        // wrap around at end
        // // remember that lb16pos contains position in "16ths of a pixel"
        // so the 'end of the strip' is (NEO_COUNT * 16)
        if( smoothChaseState.lb16pos >= (NEO_COUNT * 16)) {
            smoothChaseState.lb16pos -= (NEO_COUNT * 16);
        }

        // Draw everything:
        // clear the pixel buffer
        memset8( leds, 0, NEO_COUNT * sizeof(CRGB));

        // advance to the next hue
        smoothChaseState.lbhue16 = smoothChaseState.lbhue16 + 29;

        // draw the Fractional Bar, length=4px
        drawFractionalBar( smoothChaseState.lb16pos, smoothChaseState.lbWidth, smoothChaseState.lbhue16 / 256);

        FastLED.show();
    }
}

void NeoController::setup( void )
{
    #ifdef NEODEBUG
    Serial.println(F("NEO: Setting up NeoPixels"));
    #endif
    FastLED.addLeds<NEOPIXEL, NEO_PIN>(leds, NEO_COUNT);

    turnAllOff();
    setBrightness();
}
