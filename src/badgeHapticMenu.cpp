// NOLINTBEGIN(*-magic-numbers)
#include "badgeHapticMenu.h"
#include <Arduino.h>
#include "haptic.h"


void BadgeHapticMenu::hapticOn() {
    auto data = BadgeHaptic::getInstance().hapticOn();
}

void BadgeHapticMenu::hapticOff() {
    auto data = BadgeHaptic::getInstance().hapticOff();
}

// NOLINTEND(*-magic-numbers)