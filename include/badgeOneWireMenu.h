// NOLINTBEGIN(*-magic-numbers)
#pragma once

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>
#include <functional>

class BadgeOneWireMenu {
private:
    static Scheduler& taskRunner;
    static int status;

    struct TaskData {
        Task* scannerTask;
        Task* displayTask;
        bool operationComplete;
        bool setDisplay;
        std::function<void()> operation;
    };
    static void genericTaskCallback();
    static void displaySetup();
    static void displayConfig();
    static void displayTeardown();
    static void displayText(const char* text);
    static void handleScanResult(TaskData* taskData, std::function<void()> onDeviceFound);
    static void handleGenerateIDResult(TaskData* taskData, std::function<void()> onDeviceFound);

    template <size_t N>
    static void displayHexData(const std::array<uint8_t, N>& data);


public:
    static void setupTask(std::function<void()> operation);
    static void scanForDevice();
    static void readMemory();
    static void readScratchpad();
    static void readStatusRegister();
    static void readApplicationRegister();
    static void registerButtonToBadge(std::function<void(const std::array<uint8_t, 8>&, const std::array<uint8_t, 8>&)> onComplete);

    static void kioskSetupIButton();

    static auto onEnable() -> bool;
    static void onDisable();
    static auto isActive() -> bool;
    static void setActive(int value);
};
// NOLINTEND(*-magic-numbers)