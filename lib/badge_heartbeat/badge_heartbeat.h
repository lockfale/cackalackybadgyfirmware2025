#pragma once

#include <Arduino.h>

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>
#include <functional>

class BadgeHeartbeat {
public:
    // Constructor
    BadgeHeartbeat() = default;

    // Function to print free heap memory
    void print_free_mem() const;

    // Static callback for the task
    static void taskCallback();
    
    static void startHeartbeat();
    static void stopHeartbeat();

    static Scheduler& taskRunner;
    static int status;
};
