// NOLINTBEGIN(*-magic-numbers,*-avoid-c-arrays,*-pro-bounds-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
#pragma once

#include <Arduino.h>

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>

#include <OneWire.h>
#include "Adafruit_DS248x.h"
#include <string>
#include <vector>
#include <queue>


enum {
    ONE_WIRE_GPIO = 12
};

enum class ScanStatus {
    NOT_STARTED,
    IN_PROGRESS,
    DEVICE_FOUND,
    FAILED,
    TIMEOUT,
    SPOOFED
};

enum class UniqueIdStatus {
    NOT_STARTED,
    IN_PROGRESS,
    COMPLETE,
    FAILED,
    TIMEOUT
};

enum class OneWireMode {
    Mode1990_1,
    Mode1990_2,
    Unknown
};

enum {
    SCAN_COUNT_THRESHOLD = 20,
    MEMORY_SIZE = 32,
    REGISTER_MEMORY_SIZE = 1,
    POWER_PULL_UP = 1,
    APP_MEMORY_SIZE = 8,
    DEVICE_ADDRESS_SIZE = 8,
    CMD_SKIP_ROM = 0xCC,
    CMD_READ_EEPROM = 0xF0,
    CMD_READ_APPLICATION_REGISTER = 0xC3,
    CMD_WRITE_SCRATCHPAD = 0x0F,
    CMD_WRITE_APPLICATION_REGISTER = 0x99,
    CMD_READ_SCRATCHPAD = 0xAA,
    CMD_COPY_SCRATCHPAD_TO_EEPROM = 0x55,
    CMD_COPY_AUTH_VALIDATION_KEY = 0xA5,
    CMD_READ_STATUS_REGISTER = 0x66,
    CMD_COPY_AND_LOCK_APPLICATION_REGISTER = 0x5A,
    CMD_READ_ROM = 0x33,
    CMD_CHANGE_SERIAL = 0xD5,
    DELAY_30_MILLISECONDS = 30,
    DELAY_500_MILLISECONDS = 500,
};

class BadgeOneWireReader {
private:
    explicit BadgeOneWireReader(uint8_t pin);
    OneWire oneWire;
    Adafruit_DS248x ds248x;
    uint8_t deviceAddress[DEVICE_ADDRESS_SIZE]{};

    static Scheduler& taskRunner;
    static Scheduler& hpTaskRunner;

    int currentScanCount = 0;
    ScanStatus scanStatus = ScanStatus::NOT_STARTED;
    UniqueIdStatus uniqueIdStatus = UniqueIdStatus::NOT_STARTED;
    static void niaveScanTask();
    static void waitForUniqueId();

    template <size_t Size>
    auto readData(uint8_t command, bool setStartAddress) -> std::array<uint8_t, Size>;

    template <size_t Size>
    auto writeData(uint8_t command, std::array<uint8_t, Size>& data) -> bool;

    auto sendCommand(uint8_t command, bool strongPullup = false) -> bool;

    std::queue<std::pair<std::string, std::vector<uint8_t>>> mqttQueue;
    static std::vector<uint8_t> iButtonMasterKey;


public:
    // Singleton instance retrieval
    static auto getInstance() -> BadgeOneWireReader&;
    static void kioskScanTask();

    // Delete copy constructor and assignment operator
    BadgeOneWireReader(const BadgeOneWireReader&) = delete;
    auto operator=(const BadgeOneWireReader&) -> BadgeOneWireReader& = delete;

    // Methods
    auto getDeviceAddress() -> std::array<uint8_t, DEVICE_ADDRESS_SIZE>;
    auto readMemory() -> std::array<uint8_t, MEMORY_SIZE>;
    auto writeMemory() -> bool;
    auto readScratchpad() -> std::array<uint8_t, MEMORY_SIZE>;
    auto writeScratchpad(std::array<uint8_t, MEMORY_SIZE>& data) -> bool;
    auto copyScratchpad() -> bool;                           // copies scratchpad to EEPROM
    auto readStatusRegister() -> std::array<uint8_t, REGISTER_MEMORY_SIZE>;
    auto writeApplicationRegister(std::array<uint8_t, APP_MEMORY_SIZE>& data) -> bool;
    auto readApplicationRegister() -> std::array<uint8_t, APP_MEMORY_SIZE>;
    auto copyLockApplicationRegister() -> bool;
    auto kioskResetIButtonMasterKey() -> void;
    static void printDeviceAddress(const uint8_t* address); // Prints the device address

    void addTaskToScheduler(Scheduler* scheduleQueue);

    auto startScanningTask(Scheduler* scheduler) -> Task*;
    auto waitForUniqueIdTask(Scheduler* scheduler) -> Task*;
    void stopTask(Task* taskToStop);
    void resetScanCount() { currentScanCount = 0; }
    void resetScanState();
    int getScanCount() const { return currentScanCount; }
    ScanStatus getScanStatus() const { return scanStatus; }
    UniqueIdStatus getUniqueIdStatus() const { return uniqueIdStatus; }
    void scanStatusToString(ScanStatus status);
    bool check_spoof();
    bool readAddRom(uint8_t* romData);
    auto detectOneWireMode() -> OneWireMode;
    bool deadBeef();

    void enqueueMQTTMessage(std::string topic, std::vector<uint8_t> payload);
    void processMQTTQueue();
    auto isMQTTQueueEmpty() -> bool;
    static void waitForUniqueIdTask();

    void setIButtonMasterKey(const std::vector<uint8_t>& ibuttonID);
    std::vector<uint8_t> getIButtonMasterKey() const;
};
// NOLINTEND(*-magic-numbers,*-avoid-c-arrays,*-pro-bounds-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
