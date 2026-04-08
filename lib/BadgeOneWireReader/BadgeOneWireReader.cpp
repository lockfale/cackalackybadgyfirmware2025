// NOLINTBEGIN(*-magic-numbers,*-avoid-c-arrays,*-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
#include "BadgeOneWireReader.h"

extern Scheduler baseTaskRunner;
extern Scheduler highPriorityTaskRunner;
Scheduler& BadgeOneWireReader::taskRunner = baseTaskRunner;
Scheduler& BadgeOneWireReader::hpTaskRunner = highPriorityTaskRunner;
std::vector<uint8_t> BadgeOneWireReader::iButtonMasterKey;

// Private Constructor: Cannot be called directly, only via `getInstance()`
// This is for the singleton pattern
BadgeOneWireReader::BadgeOneWireReader(uint8_t pin) : oneWire(pin) {
    Serial.println("OneWire Reader Initialized");
}

auto BadgeOneWireReader::getInstance() -> BadgeOneWireReader& {
    static BadgeOneWireReader instance(ONE_WIRE_GPIO);
    return instance;
}

/**
 * @brief Scans for 1-Wire devices in kiosk mode.
 *
 * This function performs a scan for 1-Wire devices on the bus in kiosk mode. It updates
 * the scan status based on the result of the operation and handles specific cases such
 * as spoofed devices or devices requiring additional initialization.
 *
 * @details
 * - If the scan count exceeds the `SCAN_COUNT_THRESHOLD`, the scan status is set to `TIMEOUT`.
 * - The function initializes the DS248x device and attempts to reset the 1-Wire bus.
 * - If a device is found, its ROM address is read and stored in the `deviceAddress` array.
 * - If the first byte of the address is `0x14`, the function checks for spoofing using `check_spoof()`.
 * - If the first byte of the address is valid, the scan status is updated to `DEVICE_FOUND`.
 * - The device address is printed to the serial monitor for debugging purposes.
 *
 * @note Ensure that the DS248x device is properly connected and initialized before calling this function.
 *       This function is typically used in kiosk environments for scanning and validating 1-Wire devices.
 *
 * @warning This function modifies the singleton instance's `scanStatus` and `deviceAddress` fields.
 *          Ensure proper synchronization if accessed from multiple threads or tasks.
 *
 * @example
 * ```cpp
 * BadgeOneWireReader::getInstance().kioskScanTask();
 * ```
 */
void BadgeOneWireReader::kioskScanTask() {
    auto& instance = BadgeOneWireReader::getInstance();

    if (instance.getScanCount() >= SCAN_COUNT_THRESHOLD) {
        instance.scanStatus = ScanStatus::TIMEOUT;
        return;
    }
    instance.currentScanCount++;
    instance.scanStatus = ScanStatus::IN_PROGRESS;

    if (!instance.ds248x.begin(&Wire, DS248X_ADDRESS)) {
        Serial.println(F("DS248x initialization failed."));
        instance.scanStatus = ScanStatus::FAILED;
        return;
    }

    uint8_t tempAddress[DEVICE_ADDRESS_SIZE];
    if (instance.ds248x.OneWireReset() && instance.ds248x.OneWireSearch(tempAddress)) {
        // Device found
        for (int i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
            instance.deviceAddress[i] = tempAddress[i];
        }

        // Call check_spoof and set scanStatus to FAILED if it returns false
        if (instance.deviceAddress[0] == 0x14 && !instance.check_spoof()) {
            instance.scanStatus = ScanStatus::SPOOFED;
            return;
        }

        instance.scanStatus = ScanStatus::DEVICE_FOUND;
        instance.printDeviceAddress(instance.deviceAddress);
        return;
    }
}

/**
 * @brief Periodically checks for a unique ID from the 1-Wire device.
 *
 * This function is executed as part of a task to periodically check for the presence
 * of a unique ID from the 1-Wire device. It updates the `uniqueIdStatus` to indicate
 * the progress or completion of the operation.
 *
 * @details
 * - If the scan count exceeds the `SCAN_COUNT_THRESHOLD`, the `uniqueIdStatus` is set to `TIMEOUT`.
 * - The function increments the current scan count and sets the `uniqueIdStatus` to `IN_PROGRESS`.
 * - If the `iButtonMasterKey` is empty, the function exits without updating the status.
 * - If a unique ID is found or generated, the `uniqueIdStatus` is set to `COMPLETE`, and a message
 *   is printed to the serial monitor.
 *
 * @note Ensure that the 1-Wire device is properly connected and initialized before using this function.
 *       This function is typically called as part of a scheduled task.
 *
 * @warning This function modifies the singleton instance's `uniqueIdStatus` field. Ensure proper
 *          synchronization if accessed from multiple threads or tasks.
 *
 * @example
 * ```cpp
 * BadgeOneWireReader::getInstance().waitForUniqueId();
 * ```
 */
void BadgeOneWireReader::waitForUniqueId() {
    auto& instance = BadgeOneWireReader::getInstance();

    if (instance.getScanCount() >= SCAN_COUNT_THRESHOLD) {
        instance.uniqueIdStatus = UniqueIdStatus::TIMEOUT;
        return;
    }
    instance.currentScanCount++;
    instance.uniqueIdStatus = UniqueIdStatus::IN_PROGRESS;

    if (instance.iButtonMasterKey.empty()) {
        return;
    }

    instance.uniqueIdStatus = UniqueIdStatus::COMPLETE;
    Serial.println("Unique ID found/generated");
    return;
}

/**
 * @brief Performs a naive scan for 1-Wire devices on the bus.
 *
 * This function scans for 1-Wire devices by attempting to read their ROM addresses.
 * It updates the scan status based on the result of the operation and handles specific
 * cases such as spoofed devices or devices requiring additional initialization.
 *
 * @details
 * - If the scan count exceeds the `SCAN_COUNT_THRESHOLD`, the scan status is set to `TIMEOUT`.
 * - The function initializes the DS248x device and attempts to reset the 1-Wire bus.
 * - If a device is found, its ROM address is read and stored in the `deviceAddress` array.
 * - If the first byte of the address is `0x14`, the function checks for spoofing using `check_spoof()`.
 * - If the first byte of the address is `0x01`, the function performs a "deadbeef" operation using `deadBeef()`.
 * - The scan status is updated to `DEVICE_FOUND`, `SPOOFED`, or `FAILED` based on the outcome.
 *
 * @note Ensure that the DS248x device is properly connected and initialized before calling this function.
 *
 * @warning This function modifies the singleton instance's `scanStatus` and `deviceAddress` fields.
 *
 * @example
 * ```cpp
 * BadgeOneWireReader::getInstance().niaveScanTask();
 * ```
 */
void BadgeOneWireReader::niaveScanTask() {
    auto& instance = BadgeOneWireReader::getInstance();

    if (instance.getScanCount() >= SCAN_COUNT_THRESHOLD) {
        instance.scanStatus = ScanStatus::TIMEOUT;
        return;
    }
    instance.currentScanCount++;
    instance.scanStatus = ScanStatus::IN_PROGRESS;

    if (!instance.ds248x.begin(&Wire, DS248X_ADDRESS)) {
        Serial.println(F("DS248x initialization failed."));
        instance.scanStatus = ScanStatus::FAILED;
        return;
    }

    // Use readAddRom to get the device address
    uint8_t tempAddress[DEVICE_ADDRESS_SIZE];
    if (instance.ds248x.OneWireReset() && instance.readAddRom(tempAddress)) {
        //device found
        for (int i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
            instance.deviceAddress[i] = tempAddress[i];
        }
            // Check for spoofing if the first byte of deviceAddress is 0x14
        if (instance.deviceAddress[0] == 0x14 && !instance.check_spoof()) {
            instance.scanStatus = ScanStatus::SPOOFED;
            return;
        }

        if (instance.deviceAddress[0] == 0x01) {
            Serial.println("Deadbeefing");
            instance.deadBeef();
            instance.scanStatus = ScanStatus::DEVICE_FOUND;
            instance.printDeviceAddress(instance.deviceAddress);
            return;
        }

        instance.scanStatus = ScanStatus::DEVICE_FOUND;
        instance.printDeviceAddress(instance.deviceAddress);
        return;
    }
}

/**
 * @brief Starts a scanning task to detect 1-Wire devices.
 *
 * This function initializes a task that periodically scans for 1-Wire devices on the bus.
 * The task runs at a fixed interval and updates the `scanStatus` to indicate the progress
 * or result of the scanning operation.
 *
 * @param scheduler A pointer to the `Scheduler` instance where the task will be added.
 * @return A pointer to the created `Task` object.
 *
 * @details
 * - The function resets the scan count and sets the `scanStatus` to `NOT_STARTED`.
 * - A new task is created with a delay of 500 milliseconds and is set to run indefinitely (`TASK_FOREVER`).
 * - The task executes the `BadgeOneWireReader::niaveScanTask()` method to perform the scanning operation.
 * - The task is enabled immediately after creation and added to the provided scheduler.
 *
 * @note Ensure that the `Scheduler` instance is properly initialized before calling this function.
 *       The returned task pointer must be managed and deleted appropriately to avoid memory leaks.
 *
 * @example
 * ```cpp
 * Scheduler scheduler;
 * Task* scanTask = BadgeOneWireReader::getInstance().startScanningTask(&scheduler);
 * ```
 */
auto BadgeOneWireReader::startScanningTask(Scheduler* scheduler) -> Task* {
    resetScanCount();
    scanStatus = ScanStatus::NOT_STARTED;
    Task* scanTask = new Task(DELAY_500_MILLISECONDS, TASK_FOREVER, &BadgeOneWireReader::niaveScanTask,
                            scheduler, true, nullptr, nullptr, true);
    scanTask->enable();
    return scanTask;
}

/**
 * @brief Creates a task to wait for a unique ID from the 1-Wire device.
 *
 * This function initializes a task that periodically checks for a unique ID from the
 * 1-Wire device. The task runs at a fixed interval and updates the `uniqueIdStatus`
 * to indicate the progress or completion of the operation.
 *
 * @param scheduler A pointer to the `Scheduler` instance where the task will be added.
 * @return A pointer to the created `Task` object.
 *
 * @details
 * - The function resets the scan count and sets the `uniqueIdStatus` to `NOT_STARTED`.
 * - A new task is created with a delay of 500 milliseconds and is set to run indefinitely (`TASK_FOREVER`).
 * - The task executes the `BadgeOneWireReader::waitForUniqueId()` method to check for the unique ID.
 * - The task is enabled immediately after creation and added to the provided scheduler.
 *
 * @note Ensure that the `Scheduler` instance is properly initialized before calling this function.
 *       The returned task pointer must be managed and deleted appropriately to avoid memory leaks.
 *
 * @example
 * ```cpp
 * Scheduler scheduler;
 * Task* uniqueIdTask = BadgeOneWireReader::getInstance().waitForUniqueIdTask(&scheduler);
 * ```
 */
auto BadgeOneWireReader::waitForUniqueIdTask(Scheduler* scheduler) -> Task* {
    resetScanCount();
    uniqueIdStatus = UniqueIdStatus::NOT_STARTED;
    Task* uniqueIdTask = new Task(DELAY_500_MILLISECONDS, TASK_FOREVER, &BadgeOneWireReader::waitForUniqueId,
                            scheduler, true, nullptr, nullptr, true);
    uniqueIdTask->enable();
    return uniqueIdTask;
}

/**
 * @brief Stops and deletes a task from the scheduler.
 *
 * This function stops the execution of a given task and deletes it to free up memory.
 * It is used to clean up dynamically allocated tasks that are no longer needed.
 *
 * @param taskToStop A pointer to the `Task` object to be stopped and deleted.
 *
 * @details
 * - The function deletes the task using the `delete` operator.
 * - Ensure that the task pointer passed to this function is valid and was dynamically allocated.
 *
 * @note After calling this function, the `taskToStop` pointer becomes invalid and should not be used.
 *
 * @warning Passing an invalid or already deleted task pointer to this function may result in undefined behavior.
 *
 * @example
 * ```cpp
 * Task* myTask = new Task(1000, TASK_FOREVER, []() { Serial.println("Task running"); }, &scheduler, true);
 * BadgeOneWireReader::getInstance().stopTask(myTask);
 * ```
 */
void BadgeOneWireReader::stopTask(Task* taskToStop) {
    delete taskToStop;
}

/**
 * @brief Resets the scan state of the 1-Wire reader.
 *
 * This function resets the scan state of the `BadgeOneWireReader` singleton instance.
 * It sets the scan status to `ScanStatus::NOT_STARTED`, resets the scan count, and
 * clears the device address by setting all bytes to `0`.
 *
 * @details
 * - The function retrieves the singleton instance of `BadgeOneWireReader` using `getInstance()`.
 * - The `scanStatus` is set to `ScanStatus::NOT_STARTED`.
 * - The scan count is reset by calling `resetScanCount()`.
 * - The `deviceAddress` array is cleared by setting all its elements to `0`.
 *
 * @note This function is typically used to reinitialize the scan state before starting
 *       a new scan operation.
 *
 * @example
 * ```cpp
 * BadgeOneWireReader::getInstance().resetScanState();
 * ```
 */
void BadgeOneWireReader::resetScanState() {
    auto& instance = BadgeOneWireReader::getInstance();
    instance.scanStatus = ScanStatus::NOT_STARTED;
    resetScanCount();
    for (int i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
        instance.deviceAddress[i] = 0;
    }
}

/**
 * @brief Converts a `ScanStatus` enum value to a human-readable string and prints it to the serial monitor.
 *
 * This function takes a `ScanStatus` value and prints its corresponding string representation
 * to the serial monitor. It is useful for debugging and logging the current scan status of the
 * 1-Wire device.
 *
 * @param status The `ScanStatus` enum value to be converted to a string.
 *
 * @details
 * - The function uses a `switch` statement to map each `ScanStatus` value to its string equivalent.
 * - Supported `ScanStatus` values:
 *   - `DEVICE_FOUND`: Prints "DEVICE_FOUND".
 *   - `TIMEOUT`: Prints "TIMEOUT".
 *   - `FAILED`: Prints "FAILED".
 *   - `NOT_STARTED`: Prints "NOT_STARTED".
 *   - `IN_PROGRESS`: Prints "IN_PROGRESS".
 *   - `SPOOFED`: Prints "SPOOFED".
 *   - `UNKNOWN`: Prints "UNKNOWN" for any unrecognized status.
 *
 * @note Ensure that the `ScanStatus` value passed to this function is valid. If an invalid
 *       value is provided, "UNKNOWN" will be printed.
 *
 * @example
 * ```cpp
 * BadgeOneWireReader::scanStatusToString(ScanStatus::DEVICE_FOUND);
 * // Output: DEVICE_FOUND
 * ```
 */
void BadgeOneWireReader::scanStatusToString(ScanStatus status) {
    switch (status) {
        case ScanStatus::DEVICE_FOUND:
            Serial.println("DEVICE_FOUND");
            break;
        case ScanStatus::TIMEOUT:
            Serial.println("TIMEOUT");
            break;
        case ScanStatus::FAILED:
            Serial.println("FAILED");
            break;
        case ScanStatus::NOT_STARTED:
            Serial.println("NOT_STARTED");
            break;
        case ScanStatus::IN_PROGRESS:
            Serial.println("IN_PROGRESS");
            break;
        case ScanStatus::SPOOFED:
            Serial.println("SPOOFED");
            break;
        default:
            Serial.println("UNKNOWN");
            break;
    }
}

/**
 * @brief Reads data from the 1-Wire device using a specified command.
 *
 * This function sends a command to the connected 1-Wire device and retrieves the
 * requested data. The data is read byte by byte and stored in a `std::array`. The
 * function resets the 1-Wire bus before and after the operation to ensure proper
 * communication.
 *
 * @tparam Size The size of the data array to be read.
 * @param command The command byte to be sent to the 1-Wire device.
 * @param setStartAddress A boolean flag indicating whether to send a start address
 *                        byte (`0x00`) after the command. Defaults to `false`.
 * @return A `std::array` containing the data read from the device. The size of the
 *         array matches the `Size` template parameter.
 *
 * @details
 * - The function resets the 1-Wire bus using `OneWireReset()` before sending the command.
 * - The `CMD_SKIP_ROM` command is sent first to address all devices on the bus.
 * - The specified `command` is then sent to the device.
 * - If the command is `CMD_READ_STATUS_REGISTER`, a validation key (`0x00`) is sent.
 * - If `setStartAddress` is `true`, a start address byte (`0x00`) is sent after the command.
 * - The data is read byte by byte from the device and stored in the `std::array`.
 * - After the operation, the 1-Wire bus is reset again to finalize the communication.
 *
 * @note Ensure that the 1-Wire device is properly connected and operational before
 *       calling this function.
 *
 * @warning The function does not perform additional validation to confirm the success
 *          of the read operation. Additional checks may be required to ensure data
 *          integrity.
 */
template <size_t Size>
auto BadgeOneWireReader::readData(uint8_t command, bool setStartAddress) -> std::array<uint8_t, Size> {
    std::array<uint8_t, Size> data{};
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(CMD_SKIP_ROM);
    ds248x.OneWireWriteByte(command);

    if (command == CMD_READ_STATUS_REGISTER) {
        // validation key only for status register
        ds248x.OneWireWriteByte(0x00);
    }

    if (setStartAddress) {
        ds248x.OneWireWriteByte(0x00); // default to zero for now
    }

    for (uint8_t i = 0; i < Size; i++) {
        ds248x.OneWireReadByte(&data.at(i));
    }
    ds248x.OneWireReset();
    return data;
}

/**
 * @brief Writes data to the 1-Wire device using a specified command.
 *
 * This function sends a command to the connected 1-Wire device and writes the provided
 * data to the device. The data is written byte by byte after the command is sent. The
 * function resets the 1-Wire bus before and after the operation to ensure proper communication.
 *
 * @tparam Size The size of the data array to be written.
 * @param command The command byte to be sent to the 1-Wire device.
 * @param data A reference to a `std::array` containing the data to be written. The size
 *             of the array must match the `Size` template parameter.
 * @return `true` if the data was successfully written to the device, `false` otherwise.
 *
 * @details
 * - The function resets the 1-Wire bus using `OneWireReset()` before sending the command.
 * - The `CMD_SKIP_ROM` command is sent first to address all devices on the bus.
 * - The specified `command` is then sent to the device, followed by a default address byte (`0x00`).
 * - The data is written byte by byte to the device.
 * - After the operation, the 1-Wire bus is reset again to finalize the communication.
 *
 * @note Ensure that the 1-Wire device is properly connected and operational before calling
 *       this function. The data array must contain valid data to be written to the device.
 *
 * @warning The function does not perform additional validation to confirm the success of
 *          the write operation. Additional checks may be required to ensure data integrity.
 */
template <size_t Size>
auto BadgeOneWireReader::writeData(uint8_t command, std::array<uint8_t, Size>& data) -> bool {
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(CMD_SKIP_ROM);
    ds248x.OneWireWriteByte(command);
    ds248x.OneWireWriteByte(0x00);
    for (uint8_t i = 0; i < Size; i++) {
        ds248x.OneWireWriteByte(data.at(i));
    }
    ds248x.OneWireReset();
    return true;
}

/**
 * @brief Sends a command to the connected 1-Wire device.
 *
 * This function sends a specified command to the connected 1-Wire device. If the
 * `strongPullup` parameter is set to `true`, a strong pull-up is applied after
 * sending the command to ensure sufficient power for certain operations, such as
 * writing to non-volatile memory.
 *
 * @param command The command byte to be sent to the 1-Wire device.
 * @param strongPullup A boolean flag indicating whether to apply a strong pull-up
 *                     after sending the command. Defaults to `false`.
 * @return `true` if the command was successfully sent, `false` otherwise.
 *
 * @details
 * - The function resets the 1-Wire bus before sending the command.
 * - The `CMD_SKIP_ROM` command is sent first to address all devices on the bus.
 * - The specified `command` is then sent to the device.
 * - If `strongPullup` is `true`, the `CMD_COPY_AUTH_VALIDATION_KEY` command is sent
 *   before enabling the strong pull-up.
 * - The function assumes that the 1-Wire device is properly connected and operational.
 *
 * @note Ensure that the 1-Wire device is ready to accept the command before calling
 *       this function. The strong pull-up is typically required for operations like
 *       writing to EEPROM or locking registers.
 *
 * @warning The function does not verify the success of the operation after sending
 *          the command. Additional validation may be required to ensure the command
 *          was executed successfully.
 */
auto BadgeOneWireReader::sendCommand(uint8_t command, bool strongPullup) -> bool {
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(CMD_SKIP_ROM);
    ds248x.OneWireWriteByte(command);
    if (strongPullup) {
        // we're making the assumption that if we're pulling up...
        // we're copying and need a validation key first
        ds248x.strongPullup(true);
        ds248x.OneWireWriteByte(CMD_COPY_AUTH_VALIDATION_KEY);
        ds248x.strongPullup(true);
    }
    Serial.println("DONE");
    return true;
}

/**
 * @brief Retrieves the device address of the connected 1-Wire device.
 *
 * This function returns the device address stored in the `deviceAddress` member
 * variable as a `std::array`. The device address uniquely identifies the 1-Wire
 * device on the bus.
 *
 * @return A `std::array` containing the device address. The size of the array
 *         matches `DEVICE_ADDRESS_SIZE`.
 *
 * @details
 * - The function copies the contents of the `deviceAddress` member variable
 *   into a new `std::array` and returns it.
 * - The `deviceAddress` is populated during the scanning process, such as in
 *   `kioskScanTask()` or `niaveScanTask()`.
 *
 * @note Ensure that the device address has been populated by a successful scan
 *       before calling this function. If the address is uninitialized, the
 *       returned array may contain invalid data.
 */
auto BadgeOneWireReader::getDeviceAddress() -> std::array<uint8_t, DEVICE_ADDRESS_SIZE> {
    std::array<uint8_t, DEVICE_ADDRESS_SIZE> data;
    std::copy(std::begin(deviceAddress), std::end(deviceAddress), data.begin());
    return data;
}

/**
 * @brief Reads data from the status register of the 1-Wire device.
 *
 * This function sends the `CMD_READ_STATUS_REGISTER` command to the connected
 * 1-Wire device and retrieves the data stored in its status register. The status
 * register contains information about the current state or configuration of the device.
 *
 * @return A `std::array` containing the data read from the status register. The size
 *         of the array matches `REGISTER_MEMORY_SIZE`.
 *
 * @details
 * - The function uses the `readData()` template method to send the read command and
 *   retrieve the data from the status register.
 * - The `CMD_READ_STATUS_REGISTER` command is used to initiate the read operation.
 * - The function assumes that the 1-Wire device is properly connected and operational.
 *
 * @note Ensure that the 1-Wire device is ready to provide status information before
 *       calling this function. The data returned by this function reflects the current
 *       state of the status register.
 *
 * @warning The function does not perform additional validation on the retrieved data.
 *          Ensure that the data integrity is verified if required.
 */
auto BadgeOneWireReader::readStatusRegister() -> std::array<uint8_t, REGISTER_MEMORY_SIZE> {
    return readData<REGISTER_MEMORY_SIZE>(CMD_READ_STATUS_REGISTER, false);
}

/**
 * @brief Reads data from the application register of the 1-Wire device.
 *
 * This function sends the `CMD_READ_APPLICATION_REGISTER` command to the connected
 * 1-Wire device and retrieves the data stored in its application register. The
 * application register is used to store specific configuration or operational data
 * for the device.
 *
 * @return A `std::array` containing the data read from the application register. The
 *         size of the array matches `APP_MEMORY_SIZE`.
 *
 * @details
 * - The function uses the `readData()` template method to send the read command and
 *   retrieve the data from the application register.
 * - The `CMD_READ_APPLICATION_REGISTER` command is used to initiate the read operation.
 * - The function assumes that the 1-Wire device is properly connected and operational.
 *
 * @note Ensure that the application register contains valid data before calling this
 *       function. The data returned by this function reflects the current state of
 *       the application register.
 *
 * @warning The function does not perform additional validation on the retrieved data.
 *          Ensure that the data integrity is verified if required.
 */
auto BadgeOneWireReader::readApplicationRegister() -> std::array<uint8_t, APP_MEMORY_SIZE> {
    return readData<APP_MEMORY_SIZE>(CMD_READ_APPLICATION_REGISTER, true);
}

/**
 * @brief Reads data from the EEPROM memory of the 1-Wire device.
 *
 * This function sends the `CMD_READ_EEPROM` command to the connected 1-Wire device
 * and retrieves the data stored in its EEPROM memory. The EEPROM is a non-volatile
 * memory area used for storing persistent data.
 *
 * @return A `std::array` containing the data read from the EEPROM. The size of the
 *         array matches `MEMORY_SIZE`.
 *
 * @details
 * - The function uses the `readData()` template method to send the read command and
 *   retrieve the data from the EEPROM.
 * - The `CMD_READ_EEPROM` command is used to initiate the read operation.
 * - The function assumes that the 1-Wire device is properly connected and operational.
 *
 * @note Ensure that the EEPROM contains valid data before calling this function.
 *       The data returned by this function reflects the current state of the EEPROM.
 *
 * @warning The function does not perform additional validation on the retrieved data.
 *          Ensure that the data integrity is verified if required.
 */
auto BadgeOneWireReader::readMemory() -> std::array<uint8_t, MEMORY_SIZE> {
    return readData<MEMORY_SIZE>(CMD_READ_EEPROM, true);
}

/**
 * @brief Reads data from the scratchpad memory of the 1-Wire device.
 *
 * This function sends the `CMD_READ_SCRATCHPAD` command to the connected 1-Wire device
 * and retrieves the data stored in its scratchpad memory. The scratchpad is a temporary
 * memory area used for staging data before it is written to the device's EEPROM.
 *
 * @return A `std::array` containing the data read from the scratchpad. The size of the
 *         array matches `MEMORY_SIZE`.
 *
 * @details
 * - The function uses the `readData()` template method to send the read command and
 *   retrieve the data from the scratchpad.
 * - The `CMD_READ_SCRATCHPAD` command is used to initiate the read operation.
 * - The function assumes that the 1-Wire device is properly connected and operational.
 *
 * @note Ensure that the scratchpad contains valid data before calling this function.
 *       The data returned by this function reflects the current state of the scratchpad.
 *
 * @warning The function does not perform additional validation on the retrieved data.
 *          Ensure that the data integrity is verified if required.
 */
auto BadgeOneWireReader::readScratchpad() -> std::array<uint8_t, MEMORY_SIZE> {
    return readData<MEMORY_SIZE>(CMD_READ_SCRATCHPAD, true);
}

/**
 * @brief Writes data to the scratchpad memory of the 1-Wire device.
 *
 * This function sends the `CMD_WRITE_SCRATCHPAD` command to the connected 1-Wire device
 * and writes the provided data to its scratchpad memory. The scratchpad is a temporary
 * memory area used for staging data before it is written to the device's EEPROM.
 *
 * @param data A reference to a `std::array` containing the data to be written to the
 *             scratchpad. The size of the array must match `MEMORY_SIZE`.
 * @return `true` if the data was successfully written to the scratchpad, `false` otherwise.
 *
 * @details
 * - The function uses the `writeData()` template method to send the write command and
 *   transfer the data to the scratchpad.
 * - The `CMD_WRITE_SCRATCHPAD` command is used to initiate the write operation.
 * - The function assumes that the 1-Wire device is properly connected and operational.
 *
 * @note Ensure that the provided data array contains valid data and that the 1-Wire
 *       device is ready to accept the write operation before calling this function.
 *
 * @warning The function does not perform additional validation to confirm the success
 *          of the write operation. Additional checks may be required to ensure data
 *          integrity.
 */
auto BadgeOneWireReader::writeScratchpad(std::array<uint8_t, MEMORY_SIZE>& data) -> bool {
    return writeData<MEMORY_SIZE>(CMD_WRITE_SCRATCHPAD, data);
}

/**
 * @brief Writes the scratchpad data to the EEPROM of the 1-Wire device.
 *
 * This function invokes the `copyScratchpad()` method to copy the data from the
 * scratchpad memory of the 1-Wire device to its EEPROM. It ensures that the
 * scratchpad data is persisted in the device's non-volatile memory.
 *
 * @return `true` if the data was successfully copied to the EEPROM, `false` otherwise.
 *
 * @details
 * - The function internally calls `copyScratchpad()` to perform the operation.
 * - The `copyScratchpad()` method sends the `CMD_COPY_SCRATCHPAD_TO_EEPROM` command
 *   to the device and applies a strong pull-up to ensure sufficient power for the
 *   EEPROM write process.
 *
 * @note Ensure that the scratchpad contains valid data and that the 1-Wire device
 *       is properly connected and initialized before calling this function.
 *
 * @warning The function does not perform additional validation to confirm the success
 *          of the EEPROM write operation. Additional checks may be required to ensure
 *          data integrity.
 */
auto BadgeOneWireReader::writeMemory() -> bool {
    return copyScratchpad();
}

/**
 * @brief Copies the scratchpad data to the EEPROM of the 1-Wire device.
 *
 * This function sends the `CMD_COPY_SCRATCHPAD_TO_EEPROM` command to the connected
 * 1-Wire device, instructing it to copy the data from its scratchpad memory to its
 * EEPROM. A strong pull-up is applied during the operation to ensure proper execution.
 *
 * @return `true` if the command was successfully sent and executed, `false` otherwise.
 *
 * @details
 * - The function uses the `sendCommand()` method to send the copy command to the device.
 * - A strong pull-up is applied during the operation to provide sufficient power for
 *   the EEPROM write process.
 * - The function assumes that the scratchpad contains valid data to be copied.
 *
 * @note Ensure that the 1-Wire device is properly connected and initialized before
 *       calling this function.
 *
 * @warning The function does not verify the success of the EEPROM write operation.
 *          Additional validation may be required to ensure data integrity.
 */
auto BadgeOneWireReader::copyScratchpad() -> bool {
    return sendCommand(CMD_COPY_SCRATCHPAD_TO_EEPROM, true);
}

/**
 * @brief Prints the address of a 1-Wire device to the serial monitor.
 *
 * This function takes a device address (an array of bytes) and prints it in a
 * human-readable hexadecimal format to the serial monitor. Each byte is separated
 * by a colon (`:`) for clarity.
 *
 * @param address A pointer to the device address array. The array must contain
 *                `DEVICE_ADDRESS_SIZE` bytes.
 *
 * @details
 * - The function iterates through the `address` array and prints each byte in
 *   hexadecimal format.
 * - If a byte is less than 16 (0x10), a leading zero is added for proper formatting.
 * - Colons (`:`) are added between bytes, except after the last byte.
 * - The function appends a newline at the end of the printed address.
 *
 * @note Ensure that the `address` pointer is valid and points to an array of
 *       `DEVICE_ADDRESS_SIZE` bytes before calling this function.
 *
 * @example
 * Given an address array `{0x28, 0xFF, 0x1C, 0xA3, 0x64, 0x15, 0x03, 0x9C}`,
 * the function will print:
 * `Found device at: 28:FF:1C:A3:64:15:03:9C`
 */
void BadgeOneWireReader::printDeviceAddress(const uint8_t* address) {
    Serial.print("Found device at: ");
    for (int i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
        if (address[i] < 16) { Serial.print("0"); }
        Serial.print(address[i], HEX);
        if (i < 7) { Serial.print(":"); }
    }
    Serial.println();
}

/**
 * @brief Adds a new task to the provided scheduler.
 *
 * This function creates a new task that executes a simple operation (printing "OneWire Task Executing")
 * at a fixed interval of 1000 milliseconds. The task is added to the provided scheduler and enabled immediately.
 *
 * @param scheduleQueue A pointer to the `Scheduler` instance where the task will be added.
 *
 * @details
 * - The task is created with a period of 1000 milliseconds and is set to run indefinitely (`TASK_FOREVER`).
 * - The task executes a lambda function that prints "OneWire Task Executing" to the serial monitor.
 * - The task is enabled immediately after being added to the scheduler.
 *
 * @note Ensure that the provided `Scheduler` instance is properly initialized before calling this function.
 *
 * @warning The task is dynamically allocated using `new`. Ensure proper memory management to avoid memory leaks.
 */
void BadgeOneWireReader::addTaskToScheduler(Scheduler* scheduleQueue) {
    Serial.println("Adding OneWire Task to Scheduler");
    Task* newTask = new Task(1000, TASK_FOREVER, []() {
        Serial.println("OneWire Task Executing");
    }, scheduleQueue, true);
    newTask->enable();
}

/**
 * @brief Checks for spoofing by verifying data written to and read from the scratchpad.
 *
 * This function generates a random byte, writes it to the scratchpad of the 1-Wire device,
 * and then reads the scratchpad to verify if the written byte matches the read byte. If the
 * values match, the device is considered valid; otherwise, it is flagged as spoofed.
 *
 * @return `true` if the written byte matches the read byte, indicating no spoofing.
 *         `false` if the values do not match or if the write operation fails.
 *
 * @details
 * - A random byte is generated using the `random()` function.
 * - The random byte is written to the scratchpad using `writeScratchpad()`.
 * - The scratchpad is read back using `readScratchpad()`.
 * - The function compares the first byte of the read data with the generated random byte.
 * - If the values match, the function returns `true`. Otherwise, it returns `false`.
 *
 * @note Ensure that the 1-Wire device is properly connected and operational before calling this function.
 *
 * @warning If the write operation to the scratchpad fails, the function will return `false`.
 */
auto BadgeOneWireReader::check_spoof() -> bool {
    int attempts = 0; // Counter for the number of attempts

    while (attempts < 3) { // Limit to 5 attempts
        attempts++;

        // Generate a random byte
        uint8_t randomByte = random(1, 255);

        // Prepare data to write to scratchpad
        std::array<uint8_t, MEMORY_SIZE> writeData{};
        writeData[0] = randomByte;

        // Write the random byte to the scratchpad
        if (!writeScratchpad(writeData)) {
            return false; // If the write fails, return false
        }

        // Read the scratchpad for the first verification
        auto firstReadData = readScratchpad();
        bool firstMatch = (firstReadData[0] == randomByte);
        if(firstMatch) {
            return true; // If the first read does not match, return false
        }

        // Write the random byte again to the scratchpad
        if (!writeScratchpad(writeData)) {
            return false; // If the write fails, return false
        }

        // Read the scratchpad for the second verification
        auto secondReadData = readScratchpad();
        bool secondMatch = (secondReadData[0] == randomByte);

        // Check if both results match
        if (firstMatch || secondMatch) {
            return true; // Two consecutive matches, return true
        }
    }
    return false; // If all attempts fail, return false
}

/**
 * @brief Reads the ROM address of a 1-Wire device.
 *
 * This function sends the `READ_ROM` command to the connected 1-Wire device and reads
 * its ROM address into the provided buffer. The ROM address is used to uniquely identify
 * the device on the 1-Wire bus.
 *
 * @param romData A pointer to a buffer where the ROM address will be stored. The buffer
 *                must be large enough to hold `DEVICE_ADDRESS_SIZE` bytes.
 * @return `true` if the ROM address was successfully read and is valid, `false` otherwise.
 *
 * @details
 * - The function first resets the 1-Wire bus using `OneWireReset()`.
 * - It sends the `READ_ROM` command to the device.
 * - The ROM address is read byte by byte into the provided buffer.
 * - After reading, the function checks if all bytes in the ROM address are `0xFF`.
 *   If any byte is not `0xFF`, the ROM address is considered valid, and the function
 *   returns `true`.
 * - If all bytes are `0xFF`, the function returns `false`, indicating an invalid ROM address.
 *
 * @note Ensure that the `romData` buffer is properly allocated and that the 1-Wire device
 *       is connected and operational before calling this function.
 *
 * @warning If the 1-Wire bus is not properly initialized or the device does not respond,
 *          the function will return `false`.
 */
bool BadgeOneWireReader::readAddRom(uint8_t* romData) {
    if (ds248x.OneWireReset()) {
        ds248x.OneWireWriteByte(CMD_READ_ROM);
        for (uint8_t i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
            ds248x.OneWireReadByte(&romData[i]);
        }

        // Check if all bytes are 0xFF
        for (uint8_t i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
            if (romData[i] != 0xFF) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Detects the operating mode of the 1-Wire device.
 *
 * This function attempts to determine the operating mode of the connected 1-Wire device
 * by sending specific command sequences and analyzing the device's response. The detected
 * mode is used to configure subsequent communication with the device.
 *
 * @return The detected operating mode as a `OneWireMode` enum value:
 *         - `OneWireMode::Mode1990_1`: Indicates the device is operating in Mode 1990.1.
 *         - `OneWireMode::Mode1990_2`: Indicates the device is operating in Mode 1990.2.
 *         - `OneWireMode::Unknown`: Indicates that the operating mode could not be determined.
 *
 * @details
 * - The function first tries the Mode 1990.1 sequence:
 *   - Sends the command `0xD1` followed by a `1` bit.
 *   - Sends the command `0xB5` and reads a byte from the device.
 *   - If the read byte is `0xFE`, the mode is determined to be `Mode1990_1`.
 * - If Mode 1990.1 is not detected, the function tries the Mode 1990.2 sequence:
 *   - Sends the command `0x1D` followed by a `1` bit.
 *   - Sends the command `0x1E` and reads a byte from the device.
 *   - If the read byte is `0xFE`, the mode is determined to be `Mode1990_2`.
 *   - Finalizes the Mode 1990.2 initialization by sending the command `0x1D` followed by a `0` bit.
 * - If neither mode is detected, the function returns `OneWireMode::Unknown`.
 *
 * @note Ensure that the 1-Wire device is properly connected and initialized before calling this function.
 *
 * @warning If the device does not respond as expected, the function will return `OneWireMode::Unknown`.
 */
auto BadgeOneWireReader::detectOneWireMode() -> OneWireMode {
    uint8_t readByte = 0;

    // Try mode 1990.1 sequence
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(0xD1);
    ds248x.OneWireWriteBit(1);
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(0xB5);
    ds248x.OneWireReadByte(&readByte);
    if (readByte == 0xFE) {
        Serial.println("is 1990.1");
        return OneWireMode::Mode1990_1;
    }

    // Try mode 1990.2 sequence
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(0x1D);
    ds248x.OneWireWriteBit(1);
    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(0x1E);
    ds248x.OneWireReadByte(&readByte);
    if (readByte == 0xFE) {
        Serial.println("is 1990.2");
        // Finalize mode 1990.2 initialization:
        ds248x.OneWireReset();
        ds248x.OneWireWriteByte(0x1D);
        ds248x.OneWireWriteBit(0);
        return OneWireMode::Mode1990_2;
    }

    return OneWireMode::Unknown;
}

/**
 * @brief Sends the "deadbeef" sequence to the 1-Wire device.
 *
 * This function detects the operating mode of the 1-Wire device and sends a specific
 * sequence of bytes (commonly referred to as "deadbeef") to the device. The sequence
 * is used for initialization, configuration, or validation purposes.
 *
 * @return `true` if the sequence was successfully sent, `false` if the operating mode
 *         could not be determined or an error occurred.
 *
 * @details
 * - The function first detects the operating mode using `detectOneWireMode()`.
 * - Depending on the mode (`Mode1990_1` or `Mode1990_2`), it adjusts the behavior
 *   for sending the sequence.
 * - The sequence `0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x8B` is sent bit by bit.
 * - After sending the sequence, the function finalizes the command and resets the
 *   1-Wire bus.
 *
 * @note
 * - In `Mode1990_1`, the bits are inverted before being sent.
 * - In `Mode1990_2`, the bits are sent as-is.
 * - A delay of 10 milliseconds is introduced after sending the sequence to allow
 *   the device to process the command.
 *
 * @warning Ensure that the 1-Wire device is properly connected and initialized
 *          before calling this function.
 */
bool BadgeOneWireReader::deadBeef() {
    // Detect operating mode
    OneWireMode mode = detectOneWireMode();
    if (mode == OneWireMode::Unknown) {
        return false;
    }

    // Write the initial command based on the detected mode
    ds248x.OneWireReset();
    if (mode == OneWireMode::Mode1990_1) {
        ds248x.OneWireWriteByte(0xD1);
        ds248x.OneWireWriteBit(0);
    } else {  // Mode1990_2
        ds248x.OneWireWriteByte(0x1D);
        ds248x.OneWireWriteBit(1);
    }

    ds248x.OneWireReset();
    ds248x.OneWireWriteByte(0xD5);

    // Write out the deadbeef sequence bit by bit
    uint8_t bytesToWrite[] = {0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x8B};
    for (uint8_t byte : bytesToWrite) {
        for (uint8_t i = 0; i < 8; i++) {
            if (mode == OneWireMode::Mode1990_1) {
                // In Mode1990_1, write the inverted bit
                ds248x.OneWireWriteBit(~byte & 0x01);
            } else {
                // In Mode1990_2, write the bit as is
                ds248x.OneWireWriteBit(byte & 0x01);
            }
            byte >>= 1;
        }
    }

    // Finalize the command based on the mode
    if (mode == OneWireMode::Mode1990_1) {
        ds248x.OneWireWriteByte(0xD1);
        ds248x.OneWireWriteBit(1);
    } else {  // Mode1990_2
        ds248x.OneWireWriteByte(0x1D);
        ds248x.OneWireWriteBit(0);
    }

    delayMicroseconds(10000);
    ds248x.OneWireReset();
    return true;
}


/*** KIOSK ONLY ***/
auto BadgeOneWireReader::writeApplicationRegister(std::array<uint8_t, APP_MEMORY_SIZE>& data) -> bool {
    return writeData<APP_MEMORY_SIZE>(CMD_WRITE_APPLICATION_REGISTER, data);
}

auto BadgeOneWireReader::copyLockApplicationRegister() -> bool {
    ds248x.strongPullup(true);
    bool x = sendCommand(CMD_COPY_AND_LOCK_APPLICATION_REGISTER, true);
    delay(2000); // bigger delay (~1s) for cheap china clones
    ds248x.OneWireReset();
    
    delay(1000); // bigger delay (~1s) for cheap china clones
    return x;
}

void BadgeOneWireReader::setIButtonMasterKey(const std::vector<uint8_t>& masterKey) {
    if (masterKey.size() == 8) {  // Ensure it's 8 bytes
        iButtonMasterKey = masterKey;
        Serial.println("masterKey stored successfully.");
    } else {
        Serial.println("Error: masterKey must be 8 bytes.");
    }
}

void BadgeOneWireReader::kioskResetIButtonMasterKey() {
    iButtonMasterKey.clear();
}

std::vector<uint8_t> BadgeOneWireReader::getIButtonMasterKey() const {
    return iButtonMasterKey;
}

void BadgeOneWireReader::enqueueMQTTMessage(std::string topic, std::vector<uint8_t> payload) {
    mqttQueue.push({topic, payload});
    // I don't love this here, but it's a quick way to ensure that we process the task immediately without
    // having a repeating task that checks the queue every 100ms or something.
    new Task(0, TASK_ONCE, []() {BadgeOneWireReader::getInstance().processMQTTQueue();}, &BadgeOneWireReader::getInstance().taskRunner, true,
        nullptr, nullptr,
    true);
}


auto BadgeOneWireReader::isMQTTQueueEmpty() -> bool {
    return mqttQueue.empty();
}


void BadgeOneWireReader::processMQTTQueue() {
    if (mqttQueue.empty()) {
        Serial.println("MQTT queue is empty.");
        return;
    }

    // Extract the front item **before popping**
    auto item = mqttQueue.front();
    mqttQueue.pop();

    const auto& topic = item.first;
    const auto& payload = item.second;

    Serial.print("Processing BadgeOneWireReader message: ");
    Serial.println(topic.c_str());

    if (topic.find("ibutton/appmem/generate") != std::string::npos) {
        // Convert vector<uint8_t> to string
        std::vector<uint8_t> convertedPayload;

        if (payload.size() == 16) {  // Ensure we have exactly 16 hex characters (8 bytes)
            for (size_t i = 0; i < payload.size(); i += 2) {
                std::string byteStr(payload.begin() + i, payload.begin() + i + 2);
                convertedPayload.push_back(static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16)));
            }
        } else {
            Serial.println("Error: Payload size is incorrect for iButton Master Key.");
            return;
        }

        BadgeOneWireReader::getInstance().setIButtonMasterKey(convertedPayload);
    } else {
        Serial.println("Unknown BadgeOneWireReader topic.");
    }
    return;
}


// NOLINTEND(*-magic-numbers,*-avoid-c-arrays,*-array-to-pointer-decay,*-pro-bounds-pointer-arithmetic,*-pro-bounds-constant-array-index)
