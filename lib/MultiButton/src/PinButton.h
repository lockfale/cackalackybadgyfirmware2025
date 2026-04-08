#pragma once

#include <Arduino.h>
#include <MultiButton.h>

class PinButton : public MultiButton {
public:
    /**
     * Construct a new PinButton using a switch connected between
     * an Arduino pin and ground.
     * The internal pull-up is automatically enabled.
     *
     * @param pin {int} Arduino pin to use
     */
    PinButton(int pin)
            : PinButton(pin, INPUT_PULLUP, false) {}

    /**
     * Construct a new PinButton using a switch.
     * Initialize the pinType according to how the button is connected:
     *  - INPUT: a switch is connected between Arduino pin and VDD with
     *      external resistor.
     *  - INPUT_PULLUP: a switch is connected between Arduino pin and ground.
     *
     * @param pin {int} Arduino pin to use
     * @param pinType {int} Set pin type (INPUT or INPUT_PULLUP)
     */
    PinButton(int pin, int pinType)
            : PinButton(pin, pinType, false) {}

    /**
     * Construct a new PinButton using a switch and specify if it is virtual.
     *
     * @param pin {int} Arduino pin to use
     * @param isVirtual {bool} True if the button is virtual
     */
    PinButton(int pin, bool isVirtual)
            : PinButton(pin, INPUT_PULLUP, isVirtual) {}

    /**
     * Construct a new PinButton using a switch, custom configuration, and
     * specify if it is virtual.
     *
     * @param pin {int} Arduino pin to use
     * @param pinType {int} Set pin type (INPUT or INPUT_PULLUP)
     * @param isVirtual {bool} True if the button is virtual
     */
    PinButton(int pin, int pinType, bool isVirtual)
            : PinButton(pin, pinType, &DEFAULT_MULTIBUTTON_CONFIG, isVirtual) {}

    /**
     * Construct a new PinButton using a switch, custom configuration, and
     * specify if it is virtual.
     *
     * @param pin {int} Arduino pin to use
     * @param configuration {const MultiButtonConfig*} Custom debounce/delay configuration
     * @param isVirtual {bool} True if the button is virtual
     */
    PinButton(int pin, const MultiButtonConfig* configuration, bool isVirtual)
            : PinButton(pin, INPUT_PULLUP, configuration, isVirtual) {}

    /**
     * Construct a new PinButton using a switch, custom configuration, and
     * specify if it is virtual.
     *
     * @param pin {int} Arduino pin to use
     * @param pinType {int} Set pin type (INPUT or INPUT_PULLUP)
     * @param configuration {const MultiButtonConfig*} Custom debounce/delay configuration
     * @param isVirtual {bool} True if the button is virtual
     */
    PinButton(int pin, int pinType, const MultiButtonConfig* configuration, bool isVirtual)
            : MultiButton(configuration), _pin(pin), _isVirtual(isVirtual) {


#ifdef ARDUINO_ARCH_STM32
        pinMode(pin, (WiringPinMode)pinType);
#else
        pinMode(pin, pinType);
#endif
        if (isVirtual) {
            // Virtual button; skip pin setup... shouldn't matter....
            _pinActiveLevel = LOW;
        } else if (INPUT == pinType) {
            _pinActiveLevel = HIGH;
        }
    }

    /**
     * Read current hardware button state and decode into
     * stable state using isClick() etc.
     *
     * It's recommended to call this method in e.g. loop().
     */
    void update() {
        MultiButton::update(digitalRead(_pin) == _pinActiveLevel);
    }

    /**
     * Advanced update method for virtual and physical buttons.
     *
     * If the button is virtual, use the provided input state.
     * If the button is physical, use the pin's digitalRead value.
     *
     * @param externalState {bool} The state of the button (true = pressed, false = released)
     */
    void update_advanced(bool externalState) {
        if (_isVirtual) {
            MultiButton::update(externalState == _pinActiveLevel);
        } else {
            // Update the button state using the physical pin
            MultiButton::update(digitalRead(_pin) == _pinActiveLevel);
        }
    }




private:
    int _pin;             // Arduino pin connected to the button
    int _pinActiveLevel;  // Active level (HIGH or LOW)
    bool _isVirtual;      // True if the button is virtual
};
