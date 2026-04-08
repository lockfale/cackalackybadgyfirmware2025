#include "badge_haptic_tasks.h"
#include "haptic.h"
#include <TaskSchedulerDeclarations.h> // Include the TaskScheduler declarations header

// Declare t_haptic as an external variable
extern Task t_haptic;

// Function to pulse the haptic motor
void BadgeHapticTask::pulse() const {
    static unsigned long pulseStartTick = 0;
    static bool pulsing = false;
    unsigned long currentTick = millis();

    if (!pulsing) {
        pulseStartTick = currentTick;
        pulsing = true;
        BadgeHaptic::getInstance().hapticOn();
    } else if (currentTick - pulseStartTick > 150) {
        BadgeHaptic::getInstance().hapticOff();
        t_haptic.disable(); // Call t_haptic.disable() after hapticOff
        pulsing = false;
    }
}

// Static wrapper for callback
void BadgeHapticTask::taskCallback() {
    static BadgeHapticTask instance;
    instance.pulse();
}