// NOLINTBEGIN(*-magic-numbers,*-avoid-c-arrays,*-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
#include "battery_level.h"

// Private Constructor
BadgeBatteryReader::BadgeBatteryReader(uint8_t pin) : pin_(pin) {
    Serial.println("Battery Reader Initialized");
}

// Destructor
BadgeBatteryReader::~BadgeBatteryReader() {
    Serial.println("Battery Reader Destroyed");
}

auto BadgeBatteryReader::getInstance() -> BadgeBatteryReader& {
    static BadgeBatteryReader instance(ADC_GPIO);
    return instance;
}

auto BadgeBatteryReader::readRaw() -> uint16_t {
    return analogRead(pin_);
}

auto BadgeBatteryReader::readBatteryPercentage() -> uint8_t { 
    float voltage = (readRaw() * 4.2f) / 1023.0f;  // Convert ADC reading to voltage

    // Clamp voltage to range 3.3V - 4.2V
    if (voltage < 3.3f) return 0;
    if (voltage > 4.2f) return 100;

    // Define step size for percentage mapping
    const float step = (4.2f - 3.3f) / 4.0f;  
    return static_cast<uint8_t>((voltage - 3.3f) / step) * 25;
}

// NOLINTEND(*-magic-numbers,*-avoid-c-arrays,*-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
