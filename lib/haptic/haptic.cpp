// NOLINTBEGIN(*-magic-numbers,*-avoid-c-arrays,*-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
#include "haptic.h"


/**
 * @brief Constructor for BadgeHaptic.
 * @param pin The GPIO pin number for the haptic motor.
 * @note This constructor initializes the pin and sets it to OUTPUT mode.
 */
BadgeHaptic::BadgeHaptic(uint8_t pin) : pin_(pin) {
    //Serial.println("Haptic Motor Initialized");
    pinMode(pin_, OUTPUT);
}

/**
 * @brief Destructor for BadgeHaptic.
 * @note This destructor is called when the instance of BadgeHaptic is destroyed.
 *       It is used to clean up any resources associated with the haptic motor.
 */
BadgeHaptic::~BadgeHaptic() {
    Serial.println("Haptic Motor Destroyed");
}

/**
 * @brief Singleton accessor for BadgeHaptic.
 * @return Reference to the singleton instance.
 * @note This function uses the Meyers' singleton pattern to ensure that only one instance of BadgeHaptic exists.
 *       The instance is created the first time this function is called and is destroyed when the program ends.
 */
auto BadgeHaptic::getInstance() -> BadgeHaptic& {
    static BadgeHaptic instance(HAP_GPIO);
    return instance;
}

/**
 * @brief Turns the haptic motor on.
 * @return True if the motor is successfully turned on, false otherwise.
 * @note This function sets the pin to HIGH to activate the motor.
 */
auto BadgeHaptic::hapticOn() -> bool {
    hapticMotorRunning = true;
    digitalWrite(pin_, HIGH);
    #ifdef HAPDEBUG
    Serial.println(F("Haptic Motor Turned ON"));
    #endif  // HAPDEBUG
    return true;
}

/**
 * @brief Turns the haptic motor off.
 * @return True if the motor is successfully turned off, false otherwise.
 * @note This function sets the pin to LOW to deactivate the motor.
 */
auto BadgeHaptic::hapticOff() -> bool {
    hapticMotorRunning = false;
    digitalWrite(pin_, LOW);
    #ifdef HAPDEBUG
    Serial.println(F("Haptic Motor Turned OFF"));
    #endif  // HAPDEBUG
    return true;
}

/**
 * @brief Checks if the haptic motor is currently on.
 * @return True if the motor is on, false otherwise.
 * @note This function checks the state of the haptic motor.
 */
auto BadgeHaptic::isHapticOn() const -> bool {
    return hapticMotorRunning;
}

// NOLINTEND(*-magic-numbers,*-avoid-c-arrays,*-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
