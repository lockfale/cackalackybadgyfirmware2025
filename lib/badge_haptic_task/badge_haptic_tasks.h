#pragma once

#include <Arduino.h>

class BadgeHapticTask {
public:
    // Constructor
    BadgeHapticTask() = default;

    // Function to print free heap memory
    void pulse() const;

    // Static callback for the task
    static void taskCallback();
};
