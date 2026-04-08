// NOLINTBEGIN(*-magic-numbers,*-avoid-c-arrays,*-pro-bounds-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
#pragma once

#include <Arduino.h>

enum {
    ADC_GPIO = A0,
};

class BadgeBatteryReader {

public:
    ~BadgeBatteryReader(); // Destructor

    // Singleton instance retrieval
    static auto getInstance() -> BadgeBatteryReader&;

    // Delete copy constructor and assignment operator
    BadgeBatteryReader(const BadgeBatteryReader&) = delete;
    auto operator=(const BadgeBatteryReader&) -> BadgeBatteryReader& = delete;

    // Methods
    auto readRaw() -> uint16_t;
    auto readBatteryPercentage() -> uint8_t;

private:
    explicit BadgeBatteryReader(uint8_t pin); // Private constructor
    uint8_t pin_;
};
// NOLINTEND(*-magic-numbers,*-avoid-c-arrays,*-pro-bounds-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
