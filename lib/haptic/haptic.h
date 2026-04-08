// NOLINTBEGIN(*-magic-numbers,*-avoid-c-arrays,*-pro-bounds-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
#pragma once

#include <Arduino.h>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define HAPTICEDEBUG
#endif

enum {
    HAP_GPIO = 15,
};

class BadgeHaptic {
public:
    /**
     * @brief Destructor for BadgeHaptic.
     */
    ~BadgeHaptic();

    /**
     * @brief Singleton accessor for BadgeHaptic.
     * @return Reference to the singleton instance.
     */
    static auto getInstance() -> BadgeHaptic&;

    // Delete copy constructor and assignment operator
    BadgeHaptic(const BadgeHaptic&) = delete;
    auto operator=(const BadgeHaptic&) -> BadgeHaptic& = delete;

    /**
     * @brief Turns the haptic motor on.
     * @return True if the motor is successfully turned on, false otherwise.
     * @note This function sets the pin to HIGH to activate the motor.
     */
    auto hapticOn() -> bool;

    /**
     * @brief Turns the haptic motor off.
     * @return True if the motor is successfully turned off, false otherwise.
     * @note This function sets the pin to LOW to deactivate the motor.
     */
    auto hapticOff() -> bool;

    /**
     * @brief Checks if the haptic motor is currently on.
     * @return True if the motor is on, false otherwise.
     * @note This function checks the state of the haptic motor.
     */
    bool isHapticOn() const;

private:
    /**
     * @brief Private constructor for BadgeHaptic.
     * @param pin The GPIO pin number for the haptic motor.
     * @note This constructor initializes the pin and sets it to OUTPUT mode.
     */
    explicit BadgeHaptic(uint8_t pin); // Private constructor

    uint8_t pin_;
    bool hapticMotorRunning = false;
};

// NOLINTEND(*-magic-numbers,*-avoid-c-arrays,*-pro-bounds-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
