// NOLINTBEGIN(*-magic-numbers)
#include "badgeBatteryMenu.h"
#include <Arduino.h>
#include "badgeMenu.h"
#include "badge_display.h"
#include "battery_level.h"


void BadgeBatteryMenu::readRaw() {
    // TODO -> display on screen
    // TODO -> look into button events to clear the screen
    auto data = BadgeBatteryReader::getInstance().readRaw();
    Serial.println("Memory Read:");
    Serial.print(data);
    Serial.println();
}

void BadgeBatteryMenu::readBatteryPercentage() {
    // TODO -> display on screen
    // TODO -> look into button events to clear the screen
    auto data = BadgeBatteryReader::getInstance().readBatteryPercentage();
    Serial.println("Memory Read:");
    Serial.print(data);
    Serial.println();
}

void BadgeBatteryMenu::drawBatteryIcon() {
    auto level = BadgeBatteryReader::getInstance().readBatteryPercentage();
    int fillWidth = map(level, 0, 100, 0, 16);  // Adjust fill size
    BDISPLAY::display.fillRect(112, 1, fillWidth, 2, 1);  // Fill the inside
    BDISPLAY::display.drawRect(112, 0, 16, 4, 1);

    BDISPLAY::display.display();
}
// NOLINTEND(*-magic-numbers)