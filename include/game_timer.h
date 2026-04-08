#pragma once
#include <Arduino.h>

/**
 * @brief Interval timer for game loop tick control.
 *
 * Each instance tracks its own previousMillis independently,
 * so games can each hold a GameTimer without sharing state.
 */
class GameTimer {
public:
    bool check(uint16_t duration) {
        uint32_t curMillis = millis();
        if (curMillis - _previousMillis >= duration) {
            _previousMillis = curMillis;
            return true;
        }
        return false;
    }

private:
    uint32_t _previousMillis = 0;
};
