// NOLINTBEGIN(*-magic-numbers)
#include "badgeOneWireMenu.h"
#include <Arduino.h>
#include "badge_display.h"
#include "badgeMenu.h"
#include "BadgeOneWireReader.h"
#include "NetworkTaskManager.h"
#include "ProgressConfig.h"
#include "badge_eeprom.h"
//#include "registration.h"

int BadgeOneWireMenu::status = 0;
extern Scheduler baseTaskRunner;
Scheduler& BadgeOneWireMenu::taskRunner = baseTaskRunner;

auto BadgeOneWireMenu::onEnable() -> bool {
    // Task &t = taskRunner.currentTask();
    // Serial.print("Task N: "); Serial.println(t.getId());
    return true;
}

void BadgeOneWireMenu::onDisable() {
    BadgeOneWireReader::getInstance().resetScanState();
}

bool BadgeOneWireMenu::isActive() {
    return status != 0;
}

void BadgeOneWireMenu::setActive(int active) {
    status = active;
}

void BadgeOneWireMenu::displaySetup() {
    BadgeOneWireMenu::setActive(1);
    BMenu::disableMenu();
    BadgeOneWireMenu::displayConfig();
    BDISPLAY::display.println("Scanning...");
    BDISPLAY::display.display();
}

void BadgeOneWireMenu::displayConfig() {
    BDISPLAY::clearAndDisplayBatteryPercentage();
    BDISPLAY::display.setCursor(0, 10);
    BDISPLAY::display.setTextSize(1);
}

void BadgeOneWireMenu::displayText(const char* text) {
    BadgeOneWireMenu::displayConfig();
    BDISPLAY::display.println(text);
    BDISPLAY::display.display();
}


void BadgeOneWireMenu::genericTaskCallback() {
    auto* taskData = static_cast<TaskData*>(taskRunner.currentTask().getLtsPointer());
    taskData->operation();
    if (taskData->operationComplete && taskData->displayTask) {
        if (taskData->scannerTask) {
            taskData->scannerTask->disable();
            delete taskData->scannerTask;
            taskData->scannerTask = nullptr;
        }
        taskData->displayTask->disable();
        delete taskData->displayTask;
        taskData->displayTask = nullptr;
        delete taskData;
    }
}

void BadgeOneWireMenu::handleScanResult(TaskData* taskData, std::function<void()> onDeviceFound) {
    auto& instance = BadgeOneWireReader::getInstance();
    auto status = instance.getScanStatus();

    switch (status) {
        case ScanStatus::DEVICE_FOUND: {
            if (!taskData->setDisplay) {
                onDeviceFound();
                taskData->setDisplay = true;
                taskData->operationComplete = true;
            }
            return;
        }
        case ScanStatus::TIMEOUT:
        case ScanStatus::FAILED: {
            if (!taskData->setDisplay) {
                BadgeOneWireMenu::displayText(status == ScanStatus::TIMEOUT ? "Scan timeout" : "Scan failed");
                taskData->setDisplay = true;
                taskData->operationComplete = true;
            }
            return;
        }
        case ScanStatus::SPOOFED: {
            if (!taskData->setDisplay) {
                BadgeOneWireMenu::displayText("Device spoofed");
                taskData->setDisplay = true;
                taskData->operationComplete = true;
            }
            if (!ProgressConfig::isD3adb33fUnlocked()) {
                Serial.printf("Baby's first spoofed Dallas Key... Nice work, Achievement Unlocked!\n");
                ProgressConfig::unlockD3adb33f();
                ProgressConfig::saveProgressFile();
                // Add the display effect when the challenge is unlocked
                if (ProgressConfig::getInstance().isD3adb33fUnlocked()) {
                    BDISPLAY::display.invertDisplay(true); // Invert colors
                } else {
                    BDISPLAY::display.invertDisplay(false); // Normal colors
                }
            }
            return;
        }
        default:
            break;
    }
}

void BadgeOneWireMenu::handleGenerateIDResult(TaskData* taskData, std::function<void()> onDeviceFound) {
    auto& instance = BadgeOneWireReader::getInstance();
    auto status = instance.getUniqueIdStatus();
    switch (status) {
        case UniqueIdStatus::COMPLETE: {
            if (!taskData->setDisplay) {
                onDeviceFound();
                taskData->setDisplay = true;
                taskData->operationComplete = true;
            }
            return;
        }
        case UniqueIdStatus::TIMEOUT:
        case UniqueIdStatus::FAILED: {
            if (!taskData->setDisplay) {
                BadgeOneWireMenu::displayText(status == UniqueIdStatus::TIMEOUT ? "UID timeout" : "UID failed");
                taskData->setDisplay = true;
                taskData->operationComplete = true;
            }
            return;
        }
        default:
            break;
    }
}


template <size_t N>
void BadgeOneWireMenu::displayHexData(const std::array<uint8_t, N>& data) {
    BadgeOneWireMenu::displayConfig();
    BDISPLAY::display.println();

    for (uint8_t b : data) {
        if (b < 16) {
            BDISPLAY::display.print("0");
        }
        BDISPLAY::display.print(b, HEX);
        BDISPLAY::display.print(" ");
    }

    BDISPLAY::display.display();
}

void BadgeOneWireMenu::setupTask(std::function<void()> operation) {
    Task* scannerTask = BadgeOneWireReader::getInstance().startScanningTask(&taskRunner);
    TaskData* taskData = new TaskData{scannerTask, nullptr, false, false, nullptr};

    taskData->operation = [taskData, operation]() {
        BadgeOneWireMenu::handleScanResult(taskData, [taskData, operation]() {
            operation(); // Execute the specific operation
            taskData->operationComplete = true;
        });
    };

    Task* displayTask = new Task(100, TASK_FOREVER, genericTaskCallback, &taskRunner, true, BadgeOneWireMenu::onEnable, BadgeOneWireMenu::onDisable, true);
    taskData->displayTask = displayTask;
    displayTask->setLtsPointer(taskData);
}

void BadgeOneWireMenu::scanForDevice() {
    BadgeOneWireMenu::displaySetup();

    setupTask([]() {
        BadgeOneWireMenu::displayConfig();

        auto& instance = BadgeOneWireReader::getInstance();
        auto address = instance.getDeviceAddress();

        // Display the device address on the badge display
        for (int i = 0; i < DEVICE_ADDRESS_SIZE; i++) {
            if (address[i] < 16) {
                BDISPLAY::display.print("0");
            }
            BDISPLAY::display.print(address[i], HEX);
            if (i < DEVICE_ADDRESS_SIZE - 1) {
                BDISPLAY::display.print(":");
            }
        }

        BDISPLAY::display.display();
    });
}

void BadgeOneWireMenu::readMemory() {
    BadgeOneWireMenu::displaySetup();

    setupTask([]() {
        auto& instance = BadgeOneWireReader::getInstance();
        auto data = instance.readMemory();
        BadgeOneWireMenu::displayHexData(data);
    });
}

void BadgeOneWireMenu::readScratchpad() {
    BadgeOneWireMenu::displaySetup();

    setupTask([]() {
        auto& instance = BadgeOneWireReader::getInstance();
        auto data = instance.readScratchpad();
        BadgeOneWireMenu::displayHexData(data);
    });
}

void BadgeOneWireMenu::readStatusRegister() {
    BadgeOneWireMenu::displaySetup();

    setupTask([]() {
        auto& instance = BadgeOneWireReader::getInstance();
        auto data = instance.readStatusRegister();
        BadgeOneWireMenu::displayHexData(data);
    });
}

void BadgeOneWireMenu::readApplicationRegister() {
    BadgeOneWireMenu::displaySetup();

    setupTask([]() {
        auto& instance = BadgeOneWireReader::getInstance();
        auto data = instance.readApplicationRegister();
        BadgeOneWireMenu::displayHexData(data);
    });
}

void BadgeOneWireMenu::registerButtonToBadge(std::function<void(const std::array<uint8_t, 8>&, const std::array<uint8_t, 8>&)> onComplete) {
    setupTask([onComplete]() {
        auto& instance = BadgeOneWireReader::getInstance();

        // Get the button ID and master key
        auto buttonId = instance.getDeviceAddress();
        delay(100); // Allow time for the device to respond
        auto masterKey = instance.readApplicationRegister();

        // Call the callback when done
        if (onComplete) {
            onComplete(buttonId, masterKey);
        }
    });
}

/** KIOSK ONLY **/
void BadgeOneWireMenu::kioskSetupIButton() {
    BadgeOneWireMenu::displaySetup();
    auto& instance = BadgeOneWireReader::getInstance();
    // reset master key - the kiosk is ephemeral
    instance.kioskResetIButtonMasterKey();
    int counter = 0;

    // BLOCKING - get the button ID
    while(counter < 20) {
        instance.kioskScanTask();
        auto status = instance.getScanStatus();
        switch (status) {
            case ScanStatus::DEVICE_FOUND: {
                Serial.println("Device found!");
                break;
            }
            case ScanStatus::TIMEOUT:
            case ScanStatus::FAILED: {
                Serial.println("Device search FAILED!");
                BadgeOneWireMenu::displayText(status == ScanStatus::TIMEOUT ? "Scan timeout" : "Scan failed");
                return;
            }
            default:
                delay(500);
                counter++;
                continue;
        }
        break;
    }

    const auto& deviceAddress = instance.getDeviceAddress();
    std::string deviceAddressStr;
    deviceAddressStr.reserve(DEVICE_ADDRESS_SIZE * 2);  // Reserve space for hex characters

    for (uint8_t byte : deviceAddress) {
        char buf[3];  // Two hex characters + null terminator
        snprintf(buf, sizeof(buf), "%02X", byte);
        deviceAddressStr.append(buf);
    }

    auto updatedStatus = instance.readStatusRegister();
    // check if the button is already locked
    // if so, post an update to ensure that creds on the backend are up to date
    // we will not get a response back as the button master key cannot change
    if (std::all_of(updatedStatus.begin(), updatedStatus.end(), [](uint8_t byte) { return byte == 0xFC; })) {
        // button already locked
        Serial.println("Button is already locked...");
        auto data = instance.readApplicationRegister();
        String dataHexStr;
        for (uint8_t byte : data) {
            if (byte < 16) {
                dataHexStr += "0";
            }
            dataHexStr += String(byte, HEX);
        }
        String deviceAddressArduinoStr = String(deviceAddressStr.c_str());
        String payload = deviceAddressArduinoStr + "|" + dataHexStr;

        NetworkTaskManager::getInstance().queueReceivedEventTask("ibutton/appmem/update", payload);
        BadgeOneWireMenu::displayHexData(data);
    } else {

        String deviceAddressArduinoStr = String(deviceAddressStr.c_str());
        NetworkTaskManager::getInstance().queueReceivedEventTask("ibutton/appmem/generate", deviceAddressArduinoStr);

        Task* scannerTask = BadgeOneWireReader::getInstance().waitForUniqueIdTask(&taskRunner);
        TaskData* taskData = new TaskData{scannerTask, nullptr, false, false, nullptr};

        taskData->operation = [taskData]() {
            BadgeOneWireMenu::handleGenerateIDResult(taskData, [taskData]() {
                auto& instance = BadgeOneWireReader::getInstance();

                // Read the initial status register
                auto initialStatus = instance.readStatusRegister();

                // Write the master key to the application register
                std::vector<uint8_t> masterKey = instance.getIButtonMasterKey();
                std::array<uint8_t, APP_MEMORY_SIZE> masterKeyArray;
                std::copy(masterKey.begin(), masterKey.end(), masterKeyArray.begin());

                bool statusUpdated = false;
                for (int attempt = 0; attempt < 5; ++attempt) { // Retry up to 5 times
                    instance.writeApplicationRegister(masterKeyArray);
                    delay(200);
                    instance.copyLockApplicationRegister();

                    // Read the status register again after writing
                    auto updatedStatus = instance.readStatusRegister();
                    Serial.print("Updated Status Register: ");
                    for (uint8_t byte : updatedStatus) {
                        if (byte < 16) {
                            Serial.print("0");
                        }
                        Serial.print(byte, HEX);
                        Serial.print(" ");
                    }
                    Serial.println();

                    // Check if the status register is 0xFC
                    if (std::all_of(updatedStatus.begin(), updatedStatus.end(), [](uint8_t byte) { return byte == 0xFC; })) {
                        Serial.println("Status Register Check: PASSED (0xFC)");
                        statusUpdated = true;
                        break;
                    } else {
                        Serial.println("Status Register Check: FAILED (Retrying...)");
                        instance.readStatusRegister();
                        delay(500);
                    }
                }

                if (!statusUpdated) {
                    Serial.println("Error: Status Register did not reach 0xFC after retries.");
                    BadgeOneWireMenu::displayText("Error: Status Register not 0xFC");
                    return;
                }

                // Read back the application register and display the data
                auto data = instance.readApplicationRegister();
                // reset master key - the kiosk is ephemeral
                instance.kioskResetIButtonMasterKey();
                BadgeOneWireMenu::displayHexData(data);
            });
        };

        Task* displayTask = new Task(100, TASK_FOREVER, genericTaskCallback, &taskRunner, true, BadgeOneWireMenu::onEnable, BadgeOneWireMenu::onDisable, true);
        taskData->displayTask = displayTask;
        displayTask->setLtsPointer(taskData);
    }
}

// NOLINTEND(*-magic-numbers)