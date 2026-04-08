/**
 * @file CyberPartner.cpp
 * @brief Implements the CyberPartner class methods.
 *
 * This file contains the logic for handling CyberPartner's lifecycle,
 * MQTT interactions, and state management.
 */
#include "CyberPartner.h"
#include "ProgressConfig.h"

#include <algorithm>
#include <ArduinoJson.h>

#include "NetworkTaskManager.h"
#include "CryptoUtils.h"
#include "haptic.h"

namespace {
    constexpr uint8_t maxHunger = 100;
    constexpr uint8_t maxThirst = 100;
    constexpr uint8_t maxHealth = 100;
}

extern Scheduler baseTaskRunner;
extern Scheduler highPriorityTaskRunner;
Scheduler& CyberPartner::taskRunner = baseTaskRunner;
Scheduler& CyberPartner::hpTaskRunner = highPriorityTaskRunner;
int CyberPartner::status = 0;

const std::vector<std::pair<int, LifePhase>> CyberPartner::dayToLifePhaseVector = {
    {static_cast<int>(LifePhase::Egg), LifePhase::Egg},
    {static_cast<int>(LifePhase::Baby), LifePhase::Baby},
    {static_cast<int>(LifePhase::Toddler), LifePhase::Toddler},
    {static_cast<int>(LifePhase::Child), LifePhase::Child},
    {static_cast<int>(LifePhase::Teen), LifePhase::Teen},
    {static_cast<int>(LifePhase::Adult), LifePhase::Adult},
    {static_cast<int>(LifePhase::Senior), LifePhase::Senior},
    {static_cast<int>(LifePhase::Death), LifePhase::Death}
};

CyberPartner& CyberPartner::getInstance() {
    static CyberPartner instance;
    return instance;
}

/**
 * @brief Sets the state of the CyberPartner instance.
 *
 * This function updates the `state` attribute of the `CyberPartner` singleton instance
 * with the provided `State` value.
 *
 * @param state The new state to set for the CyberPartner.
 */
void CyberPartner::setState(State state) {
    CyberPartner::getInstance().attributes.state = state;
}

WaysToDie CyberPartner::getDeathCause() {
    return CyberPartner::getInstance().attributes.deathCause;
}

/**
 * @brief Enqueues an MQTT message and triggers immediate processing.
 *
 * This function adds a new MQTT message, consisting of a topic and a payload, to the `mqttQueue`.
 * After enqueuing the message, it creates a one-time task to process the queue immediately by
 * invoking the `processMQTTQueue` function. This avoids the need for a periodic task to check
 * the queue at regular intervals.
 *
 * @param topic The topic of the MQTT message (e.g., "post/age", "get/state").
 * @param payload The payload of the MQTT message as a vector of bytes.
 *
 * @note The task is created with a delay of 0 and is executed only once.
 */
void CyberPartner::enqueueMQTTMessage(std::string topic, std::vector<uint8_t> payload) {
    mqttQueue.push({topic, payload});
    // I don't love this here, but it's a quick way to ensure that we process the task immediately without
    // having a repeating task that checks the queue every 100ms or something.
    new Task(0, TASK_ONCE, []() {CyberPartner::getInstance().processMQTTQueue();}, &CyberPartner::getInstance().taskRunner, true,
        nullptr, nullptr,
    true);
}

/**
 * @brief Processes messages from the MQTT queue and updates CyberPartner attributes.
 *
 * This function continuously processes messages from the `mqttQueue` until it is empty.
 * Each message consists of a topic and a payload. Based on the topic, the function
 * performs specific actions such as updating attributes of the `CyberPartner` singleton
 * instance or printing the current state.
 *
 * Supported topics:
 * - `post/age`: Updates the age attribute.
 * - `post/weight`: Updates the weight attribute.
 * - `post/hunger`: Updates the hunger attribute.
 * - `post/thirst`: Updates the thirst attribute.
 * - `post/happiness`: Updates the happiness attribute.
 * - `get/state`: Prints the current attributes of the CyberPartner.
 * - `post/state`: Updates the state attribute after validating the value.
 *
 * If the topic is unrecognized, an error message is logged.
 *
 * @note This function assumes that the `mqttQueue` contains pairs of topics and payloads.
 *       The payload is expected to be a vector of characters that can be converted to a string.
 */
void CyberPartner::processMQTTQueue() {
    while (!mqttQueue.empty()) {
        #ifdef CPDEBUG
        Serial.println("CyberPartner::processMQTTQueue(): queue size: " + String(mqttQueue.size()));
        #endif
        auto [topic, payload] = mqttQueue.front();
        mqttQueue.pop();

        #ifdef CPDEBUG
        Serial.print("Processing CyberPartner message: ");
        Serial.println(topic.c_str());
        #endif

        std::string messageStr(payload.begin(), payload.end());

        #ifdef CPDEBUG
        Serial.print("Payload: ");
        Serial.println(messageStr.c_str());
        #endif

        if (topic.find("state/update") != std::string::npos) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, messageStr);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }

            CyberPartner& partner = CyberPartner::getInstance();

            uint16_t lstatus = doc["status"].as<uint16_t>();
            if(lstatus >= 1000) {
                partner.die(static_cast<WaysToDie>(lstatus));
                return;
            }

            time_t lBirthTime = doc["birthday_epoch"].as<time_t>() | 0;
            partner.setBirthTime(lBirthTime);

            uint16_t lAge = doc["age"].as<uint16_t>() | 0;
            if(lAge != partner.attributes.age) {
                Serial.print("setBirthTime did not set the same age as server.");
            }

            uint16_t lWeight = doc["weight"].as<uint16_t>() | 0;
            partner.setWeight(lWeight);
            if (!ProgressConfig::getInstance().isGotHeavyUnlocked()) {
                if (partner.attributes.weight >= 195) {  // TODO Sync w/ alex on what thirst death value is
                    ProgressConfig::getInstance().unlockGotHeavy();
                    ProgressConfig::getInstance().saveProgressFile();
                }
            }
            uint8_t lHunger = doc["hunger"].as<uint8_t>() | 0;
            partner.setHunger(lHunger);
            uint16_t lThirst = doc["thirst"].as<uint16_t>() | 0;
            partner.setThirst(lThirst);
            if (!ProgressConfig::getInstance().isGotThirstyUnlocked()) {
                if (partner.attributes.thirst >= 95) {  // TODO Sync w/ alex on what thirst death value is
                    ProgressConfig::getInstance().unlockGotThirsty();
                    ProgressConfig::getInstance().saveProgressFile();
                }
            }
            uint16_t lHappiness = doc["happiness"].as<uint16_t>() | 0;
            partner.setHappiness(lHappiness);
            if (!ProgressConfig::getInstance().isGotDepressedUnlocked()) {
                if (partner.attributes.happiness <= 6) {
                    ProgressConfig::getInstance().unlockGotDepressed();
                    ProgressConfig::getInstance().saveProgressFile();
                }
            }
            partner.attributes.rock = doc["rock"].as<bool>() | false;
            partner.attributes.health = doc["health"].as<uint16_t>() | 100;
            partner.attributes.ageChangeTime = doc["ageChangeTime"].as<time_t>() | 0;
        }
        else if (topic.find("store/update") != std::string::npos) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, messageStr);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }

            for (int meal = static_cast<int>(CPStoreMeals::MEAL_APPLE);
                meal <= static_cast<int>(CPStoreMeals::MEAL_COUNT);
                ++meal) {
                CPStoreMeals mealEnum = static_cast<CPStoreMeals>(meal);

                // Convert the meal enum to a string key
                CPStore& store = CPStore::getInstance();
                const char* mealKey = store.mealToString(mealEnum);

                // Check if the key exists in the JSON
                if (doc[mealKey].is<uint32_t>()) {
                    // Get the price from the JSON
                    uint32_t mealPrice = doc[mealKey].as<uint32_t>();

                    // Set the meal price in the store
                    store.setMealPrice(mealEnum, mealPrice);

                    #ifdef CPDEBUG
                    Serial.print("Updated meal price for ");
                    Serial.print(mealKey);
                    Serial.print(": ");
                    Serial.println(mealPrice);
                    #endif
                }
            }
        }
        else if (topic.find("inventory/update") != std::string::npos) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, messageStr);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }
            CPStore& store = CPStore::getInstance();
            uint32_t money = doc["money"].as<uint32_t>() | 0;
            store.setMoney(money);

            for (int meal = static_cast<int>(CPStoreMeals::MEAL_APPLE);
                meal <= static_cast<int>(CPStoreMeals::MEAL_COUNT);
                ++meal) {
                CPStoreMeals mealEnum = static_cast<CPStoreMeals>(meal);

                // Convert the meal enum to a string key
                const char* mealKey = store.mealToString(mealEnum);

                // Check if the key exists in the JSON
                if (doc[mealKey].is<uint32_t>()) {
                    // Set the meal count in the store
                    uint32_t mealCount = doc[mealKey].as<uint32_t>();
                    store.setMealCount(mealEnum, mealCount);

                    #ifdef CPDEBUG
                    Serial.print("Updated meal count for ");
                    Serial.print(mealKey);
                    Serial.print(": ");
                    Serial.println(mealCount);
                    #endif
                }
            }
        }
        else {
            Serial.println("Unknown CyberPartner topic.");
        }
    }
}

/**
 * @brief Loads the store state from a file in the LittleFS filesystem.
 *
 * This function reads the store's state, including the amount of money and the count
 * of each purchased meal, from a JSON file stored in the LittleFS filesystem. If the
 * file does not exist or cannot be opened, the store state is reset to defaults and
 * saved to a new file.
 *
 * @return True if the store state is successfully loaded or reset to defaults, false if
 *         there is an error parsing the JSON file.
 *
 * @note If the file cannot be opened, a new file is created with default values by
 *       calling `saveStoreFile()`.
 * @note If the JSON file cannot be parsed, an error message is logged to the Serial monitor.
 */
bool CyberPartner::loadCPStatsFile() {
    JsonDocument doc;

    if (!CryptoUtils::loadEncryptedFile(_progressfile, doc, 0xA4F3C9D7)) {
        Serial.println("CyberPartner::loadCPStatsFile failed");
        saveCPStatsFile(); // Reset to defaults if loading fails
        return false;
    }

    // Debug: Print the contents of the JSON document
    #ifdef CPDEBUG
    Serial.print("Contents CPStats file:");
    serializeJson(doc, Serial);
    Serial.println(); // Add a newline for better readability
    #endif

    CyberPartner& partner = CyberPartner::getInstance();

    // Check if lifePhase is "Death"
    if (doc["lifePhase"] == "Death") {
        partner.attributes.lifePhase = LifePhase::Death;
        partner.setBirthTime(0); // Set birthTime to 0
        #ifdef CPDEBUG
        Serial.println("Life phase set to Death. Birth time reset to 0.");
        #endif
    } else {
        // Load birthTime correctly
        if (doc["birthTime"].is<time_t>()) {
            time_t lBirthTime = doc["birthTime"].as<time_t>();
            partner.setBirthTime(lBirthTime);
        } else {
            Serial.println("Error: birthTime not found in JSON. Setting to current time.");
            partner.setBirthTime(time(nullptr)); // Default to current time if missing
        }
    }

    // Convert ageChangeTime to human-readable format
    #ifdef CPDEBUG
    if (partner.attributes.birthTime > 0) {
        Serial.print("Birth Time (Human-Readable): ");
        Serial.println(ctime(&partner.attributes.birthTime));
    } else {
        Serial.println("Birth Time (Human-Readable): Not Set");
    }
    #endif

    partner.attributes.weight= doc["weight"].as<uint16_t>() | 0;
    partner.attributes.hunger= doc["hunger"].as<uint8_t>() | 0;
    partner.attributes.thirst= doc["thirst"].as<uint16_t>() | 0;
    partner.attributes.happiness = doc["happiness"].as<uint16_t>() | 0;
    partner.attributes.rock = doc["rock"].as<bool>() | false;
    partner.attributes.deathCause = static_cast<WaysToDie>(doc["deathCause"].as<uint16_t>() | 0);
    partner.attributes.health = doc["health"].as<uint16_t>() | 100;
    partner.attributes.ageChangeTime = doc["ageChangeTime"].as<time_t>() | 0;

    return true;
}

/**
 * @brief Saves the current CP state to a file in the LittleFS filesystem.
 *
 * This function generates a JSON representation of the CP's current state,
 * including the amount of money and the count of each purchased meal, and writes
 * it to a file in the LittleFS filesystem. The file is specified by the `_progressfile`
 * constant.
 *
 * @return True if the CP state is successfully saved to the file, false otherwise.
 *
 * @note If the file cannot be created or written to, an error message is logged to the Serial monitor.
 * @note The JSON document is generated using the `serializeAttributes` function.
 */
bool CyberPartner::saveCPStatsFile() {
    // Generate the JSON document
    JsonDocument cpStateDoc = serializeAttributes();

    return CryptoUtils::saveEncryptedFile(_progressfile, cpStateDoc, 0xA4F3C9D7);
}

/**
 * @brief Converts a LifePhase enumeration value to its string representation.
 *
 * This function takes a `LifePhase` enum value and returns a corresponding
 * string that represents the life phase in a human-readable format.
 *
 * Supported LifePhase values:
 * - `LifePhase::Egg`: Returns "Egg".
 * - `LifePhase::Baby`: Returns "Baby".
 * - `LifePhase::Child`: Returns "Child".
 * - `LifePhase::Teen`: Returns "Teen".
 * - `LifePhase::Adult`: Returns "Adult".
 * - `LifePhase::Senior`: Returns "Senior".
 * - `LifePhase::Death`: Returns "Death".
 *
 * If the provided `LifePhase` value is not recognized, the function returns "UNKNOWN".
 *
 * @param lifePhase The `LifePhase` enumeration value to convert.
 * @return A string representing the life phase.
 */
const char* CyberPartner::lifePhaseToString(LifePhase lifePhase) {
    switch (lifePhase) {
        case LifePhase::Egg: return "Egg";
        case LifePhase::Baby: return "Baby";
        case LifePhase::Toddler: return "Toddler";
        case LifePhase::Child: return "Child";
        case LifePhase::Teen: return "Teen";
        case LifePhase::Adult: return "Adult";
        case LifePhase::Senior: return "Senior";
        case LifePhase::Death: return "Death";
        default: return "UNKNOWN";
    }
}

const char* CyberPartner::WaysToDieToString (WaysToDie theWay) {
    switch (theWay) {
        case WaysToDie::NotDead: return "NotDead";
        case WaysToDie::AbandonedOwner: return "AbandonedOwner";
        case WaysToDie::Starved: return "Starved";
        case WaysToDie::Dehydrated: return "Dehydrated";
        case WaysToDie::Underweight: return "Underweight";
        case WaysToDie::Overweight: return "Overweight";
        case WaysToDie::Health: return "Health";
        case WaysToDie::Thanos: return "Thanos";
        case WaysToDie::Meteor: return "Meteor";
        case WaysToDie::Earthquake: return "Earthquake";
        case WaysToDie::Tornado: return "Tornado";
        case WaysToDie::Hurricane: return "Hurricane";
        case WaysToDie::Lightning: return "Lightning";
        case WaysToDie::Goose: return "Goose";
        case WaysToDie::Spontaneous_combustion: return "Spontaneous_combustion";
        case WaysToDie::NaturalCauses: return "NaturalCauses";
        case WaysToDie::DeathButton: return "DeathButton";
        case WaysToDie::Drop: return "Drop";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Converts a State enumeration value to its string representation.
 *
 * This function takes a `State` enum value and returns a corresponding
 * string that represents the state in a human-readable format.
 *
 * Supported State values:
 * - `State::ALIVE`: Returns "ALIVE".
 * - `State::DEAD`: Returns "DEAD".
 * - `State::COLD`: Returns "COLD".
 * - `State::HOT`: Returns "HOT".
 * - `State::HUNGRY`: Returns "HUNGRY".
 * - `State::TIRED`: Returns "TIRED".
 * - `State::ANNOYED`: Returns "ANNOYED".
 * - `State::IDLE`: Returns "IDLE".
 * - `State::ACTION_ROLL`: Returns "ACTION_ROLL".
 *
 * If the provided `State` value is not recognized, the function returns "UNKNOWN".
 *
 * @param state The `State` enumeration value to convert.
 * @return A string representing the state.
 */
const char* CyberPartner::stateToString(State state) {
    switch (state) {
        case State::ALIVE: return "ALIVE";
        case State::DEAD: return "DEAD";
        case State::COLD: return "COLD";
        case State::HOT: return "HOT";
        case State::HUNGRY: return "HUNGRY";
        case State::TIRED: return "TIRED";
        case State::ANNOYED: return "ANNOYED";
        case State::IDLE: return "IDLE";
        case State::ACTION_ROLL: return "ACTION_ROLL";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Serializes the attributes of the CyberPartner instance into a JSON string.
 *
 * This function retrieves the attributes of the `CyberPartner` singleton instance,
 * including numerical values (e.g., age, weight, hunger, thirst, happiness) and
 * enumerations (e.g., state, lifePhase). It converts the enumerations into their
 * string representations using helper functions (`stateToString` and `lifePhaseToString`)
 * and serializes the data into a JSON string.
 *
 * @return A JSON string representing the serialized attributes of the CyberPartner instance.
 */
JsonDocument CyberPartner::serializeAttributes()
{
    CyberPartner& partner = CyberPartner::getInstance();  // Get singleton instance

    // Create a JSON document
    JsonDocument doc;

    // Populate JSON with attributes
    doc["rock"] = partner.attributes.rock;
    doc["age"] = partner.attributes.age;
    doc["weight"] = partner.attributes.weight;
    doc["hunger"] = partner.attributes.hunger;
    doc["thirst"] = partner.attributes.thirst;
    doc["happiness"] = partner.attributes.happiness;
    doc["health"] = partner.attributes.health;
    doc["ageChangeTime"] = partner.attributes.ageChangeTime;
    doc["lifePhase"] = lifePhaseToString(partner.attributes.lifePhase);
    doc["deathCause"] = WaysToDieToString(partner.attributes.deathCause);
    doc["birthTime"] = partner.attributes.birthTime;

    return doc;
}

/**
 * @brief Prints the attributes of the CyberPartner instance to the serial output.
 *
 * This function retrieves the attributes of the `CyberPartner` singleton instance
 * and prints them to the serial output in a human-readable format. It includes
 * numerical attributes (e.g., age, weight, hunger, thirst, happiness) and
 * enumerations (e.g., state, life phase). The enumerations are converted to
 * their string representations using helper functions (`stateToString` and
 * `lifePhaseToString`).
 *
 * The output includes:
 * - Age
 * - Weight
 * - Hunger
 * - Thirst
 * - Happiness
 * - State
 * - Life Phase
 * - Last Age Change Time
 *
 * @note This function is useful for debugging or monitoring the state of the
 *       CyberPartner instance.
 */
void CyberPartner::printAttributes() {
    CyberPartner& partner = CyberPartner::getInstance();  // Get singleton instance

    Serial.println("---- CyberPartner Stats ----");
    Serial.print("Age: (years) "); Serial.println(partner.virtualYearsSinceBirth());
    Serial.print("Age: (days) "); Serial.println(partner.virtualDaysSinceBirth());
    Serial.print("Weight: "); Serial.println(partner.attributes.weight);
    Serial.print("Hunger: "); Serial.println(partner.attributes.hunger);
    Serial.print("Thirst: "); Serial.println(partner.attributes.thirst);
    Serial.print("Happiness: "); Serial.println(partner.attributes.happiness);
    Serial.print("Health: "); Serial.println(partner.attributes.health);

    Serial.print("State: ");
    Serial.println(stateToString(partner.attributes.state));

    Serial.print("Life Phase: ");
    Serial.println(lifePhaseToString(partner.attributes.lifePhase));

    Serial.print("Last Age Change Time (Epoch): ");
    Serial.println(partner.attributes.ageChangeTime);

    // Convert ageChangeTime to human-readable format
    if (partner.attributes.ageChangeTime > 0) {
        Serial.print("Last Age Change Time (Human-Readable): ");
        Serial.println(ctime(&partner.attributes.ageChangeTime));
    } else {
        Serial.println("Last Age Change Time (Human-Readable): Not Set");
    }

    Serial.print("Birth Time (Epoch): ");
    Serial.println(partner.attributes.birthTime);

    // Convert ageChangeTime to human-readable format
    if (partner.attributes.birthTime > 0) {
        Serial.print("Birth Time (Human-Readable): ");
        Serial.println(ctime(&partner.attributes.birthTime));
    } else {
        Serial.println("Birth Time (Human-Readable): Not Set");
    }
    Serial.println("--------------------------");
}

void CyberPartner::setBirthTime(time_t birthTime) {
    CyberPartner& partner = CyberPartner::getInstance();
    partner.attributes.birthTime = birthTime;

    // Debug: Print the updated birth time
    #ifdef CPDEBUG
    Serial.print("Birth Time Set To: ");
    Serial.println(birthTime);
    #endif

    partner.determineAge(); // Recalculate age based on the new birth time
}

uint16_t CyberPartner::virtualDaysSinceBirth() {
    // TODO: Need to add the aging modifyer
    CyberPartner& partner = CyberPartner::getInstance();
    time_t elapsedTime = time(nullptr) - partner.attributes.birthTime;
    return elapsedTime / 6; // 6 seconds per virtual day
}

uint16_t CyberPartner::virtualYearsSinceBirth() {
    return virtualDaysSinceBirth() / 100; // 100 virtual days per virtual year
}

uint16_t CyberPartner::getAge() {
    CyberPartner& partner = CyberPartner::getInstance();
    return partner.attributes.age;
}

/**
 * @brief Retrieves the age of the CyberPartner instance in virtual years.
 *
 * This function calculates the age of the CyberPartner in virtual years based on the
 * timestamp stored in the `birthTime` attribute. In this virtual world:
 * - Each virtual day is equivalent to 6 real seconds.
 * - A virtual year consists of 100 virtual days, making it 600 real seconds.
 *
 * The function computes the age by subtracting the `birthTime` from the current time
 * and dividing the result by the number of real seconds in a virtual year.
 *
 * @return The age of the CyberPartner instance in virtual years as an unsigned 16-bit integer.
 *
 * @note If the `birthTime` is not set or is in the future, the function may return 0.
 * @note The calculated age is also stored in the `age` attribute of the CyberPartner instance.
 */
uint16_t CyberPartner::determineAge() {
    CyberPartner& partner = CyberPartner::getInstance();

    // Calculate the age in virtual years
    uint16_t virtualYears = virtualYearsSinceBirth();

    // Ensure age does not exceed the Death phase
    if (virtualYears > static_cast<int>(LifePhase::Death)) {
        // If it's your first old age death, have an unlock
        if (!ProgressConfig::getInstance().isGotOldUnlocked()) {
            Serial.printf("GotOld Achievement Unlocked!\n");
            ProgressConfig::getInstance().unlockGotOld();
            ProgressConfig::getInstance().saveProgressFile();
        }
        partner.die(WaysToDie::NaturalCauses);
        Serial.println("CyberPartner has died.");
        return static_cast<int>(LifePhase::Death);
    }

    partner.attributes.age = virtualYears;

    // Update the life phase
    partner.setLifePhase();

    // Queue an update to notify the system
    //queueCPState(QUEUE_STATE_t::QUEUE_STATE_UPDATE);

    return partner.attributes.age;
}

void CyberPartner::bornAgain() {
    if (BadgeHaptic::getInstance().isHapticOn()) {
        BadgeHaptic::getInstance().hapticOff();
    }

    CyberPartner& partner = CyberPartner::getInstance();

    // Return immediately if the life phase is not Death
    if (partner.attributes.lifePhase != LifePhase::Death) {
        return;
    }

    // Set birth time to the current time
    time_t currentTime = time(nullptr);
    partner.setBirthTime(currentTime);

    // Debug: Print the new birth time
    #ifdef CPDEBUG
    Serial.print("New Birth Time (Epoch): ");
    Serial.println(currentTime);
    #endif

    // Reset attributes
    partner.setWeight(50);
    partner.setHunger(0);
    partner.setThirst(0);
    partner.setHappiness(100);
    partner.attributes.ageChangeTime = currentTime;

    // Reset life phase to Egg
    partner.attributes.lifePhase = LifePhase::Egg;

    // Notify the system of the rebirth
    queueCPState(QUEUE_STATE_t::QUEUE_STATE_CREATE);
    saveCPStatsFile();
    Serial.println("CyberPartner has been reborn!");
}

/**
 * @brief Sets the CyberPartner instance to the Death state.
 *
 * This function updates the `lifePhase` attribute of the `CyberPartner` singleton
 * instance to indicate that it has died. It also sets the `birthTime` and `ageChangeTime`
 * attributes to the minimum possible value, effectively marking the CyberPartner as dead.
 * Additionally, it queues a state update to notify the system of the death event if the
 * cause of death meets the specified criteria.
 *
 * @param cause The reason for death, passed as a `WaysToDie` enumeration value.
 *
 * @note The function only queues a state update if the cause of death has a value
 *       greater than or equal to 1200.
 * @note The function ensures that no further actions are taken if the CyberPartner
 *       is already in the Death state.
 */
void CyberPartner::die(WaysToDie cause) {
    CyberPartner& partner = CyberPartner::getInstance();

    // Return immediately if the life phase is already Death
    if (partner.attributes.lifePhase == LifePhase::Death) {
        #ifdef CPDEBUG
        Serial.println("Debug: CyberPartner is already in the Death state. Exiting die() early.");
        #endif
        return;
    }

    partner.attributes.deathCause = cause;

    partner.setBirthTime(0); // Set birth time to the start of time.
    //partner.attributes.lifePhase = LifePhase::Death;
    partner.attributes.ageChangeTime = 0; // Set age change time to the start of time.

    // Only queueCPState if the cause is >= 1200
    if (static_cast<int>(cause) >= 1200) {
        queueCPState(QUEUE_STATE_t::QUEUE_STATE_DEATH, WaysToDieToString(cause));
    }

    Serial.print("Here lies your CyberPartner. RIP. \nReason: ");
    saveCPStatsFile();
    Serial.println(WaysToDieToString(cause));
}

/**
 * @brief Checks if badge has synced
 *
 * @warning The time synchronization check (`time(nullptr) < 1742444909`) is hardcoded
 *          and may need to be revisited depending on the behavior that we want when
 *          the badge restarts and the owner is not within range of BadgeNet.
 *
 * @note The time synchronization check (`time(nullptr) < 1742444909`) is hardcoded and
 *       may need to be revisited for future updates.
 */
bool CyberPartner::hasSynced() {
    // when badge hasn't synced it's time
    if (time(nullptr) < 1742444909 && !CyberPartner::getInstance().attributes.debug) {
        return false;
    }
    return true;
}

void CyberPartner::queueCPState(QUEUE_STATE_t state, const char* reason) {
    const char* queueName = nullptr;

    // Generate the JSON document being sent to the MQTT broker
    JsonDocument queueDoc; // Use a smaller buffer for efficiency
    queueDoc["ts"] = time(nullptr);   // Add a timestamp to the JSON document

    switch (state) {
        case QUEUE_STATE_t::QUEUE_STATE_CREATE:
            queueName = "cp/create";
            break;
        case QUEUE_STATE_t::QUEUE_STATE_UPDATE:
            queueName = "cp/state/get";
            break;
        case QUEUE_STATE_t::QUEUE_STATE_DEATH:
            queueName = "cp/death";
            if (reason) {
                queueDoc["death_event"] = reason; // Add the reason for death
            }
            break;
        default:
            Serial.println("Unknown state, cannot queue CP state.");
            return;
    }

    // Serialize the JSON document to a character buffer
    static char payload[512]; // Adjust size as needed
    serializeJson(queueDoc, payload, sizeof(payload));
    NetworkTaskManager::getInstance().queueReceivedEventTask(queueName, payload);
}


/**
 * @brief Queues a general update for the CyberPartner instance.
 *
 * This function takes a topic and a JSON document, adds a timestamp to the document,
 * serializes it, and sends it to the server using the `NetworkTaskManager`.
 *
 * @param topic The topic to which the update is sent.
 * @param doc The JSON document containing the update data.
 */
void CyberPartner::queueGeneralUpdate(const char* topic, JsonDocument& doc) {
    // Add a timestamp to the JSON document
    doc["ts"] = time(nullptr);

    // Serialize the JSON document to a character buffer
    static char payload[512]; // Adjust size as needed
    serializeJson(doc, payload, sizeof(payload));

    // Send the serialized JSON to the server
    NetworkTaskManager::getInstance().queueReceivedEventTask(topic, payload);
}

/**
 * @brief Updates the life phase of the CyberPartner instance based on its age.
 *
 * This function determines the appropriate life phase for the `CyberPartner` singleton
 * instance by comparing its `age` attribute against predefined age thresholds stored
 * in the `ageToLifePhaseVector`. Once the correct life phase is identified, the function:
 * - Updates the `lifePhase` attribute of the `CyberPartner`.
 * - Sets the `ageChangeTime` attribute to the current time.
 *
 * @note The function uses a loop to iterate through the `ageToLifePhaseVector` and stops
 *       once the appropriate life phase is found.
 */
void CyberPartner::setLifePhase() {
    CyberPartner& partner = CyberPartner::getInstance();

    // Calculate the number of virtual days since birth
    uint16_t virtualDays = virtualDaysSinceBirth();

    // Store the current life phase
    LifePhase currentLifePhase = partner.attributes.lifePhase;

    // Determine the new life phase based on virtual days
    LifePhase newLifePhase = LifePhase::Death; // Default to Death if no match is found
    for (const auto& [minDays, lifePhase] : dayToLifePhaseVector) {
        if (virtualDays >= minDays) {
            newLifePhase = lifePhase; // Update to the most appropriate life phase
        } else {
            break; // Stop checking once we exceed the current virtualDays
        }
    }

    // Only update and save if the life phase has changed
    if (currentLifePhase != newLifePhase) {
        partner.attributes.lifePhase = newLifePhase;

        // Save the updated state since the life phase has changed
        saveCPStatsFile();

        #ifdef CPDEBUG
        Serial.print("Life phase changed to: ");
        Serial.println(lifePhaseToString(newLifePhase));
        #endif
    }
}

/**
 * @brief Retrieves the current life phase of the CyberPartner instance.
 *
 * This function accesses the `CyberPartner` singleton instance and returns
 * the value of the `lifePhase` attribute from its `attributes`.
 *
 * @return The current life phase of the CyberPartner instance as a `LifePhase` enumeration value.
 */
LifePhase CyberPartner::getLifePhase() {
    return CyberPartner::getInstance().attributes.lifePhase;
}

/**
 * @brief Sets the weight of the CyberPartner instance.
 *
 * This function updates the `weight` attribute of the `CyberPartner` singleton instance
 * with the provided value.
 *
 * @param weight The new weight to set for the CyberPartner.
 */
void CyberPartner::setWeight(int weight) {
    // If the weight is being set to a natural number, else kill the cyber partner
    if (weight <= 0) {
        CyberPartner::getInstance().die(WaysToDie::Underweight);
        Serial.println("CyberPartner's bodyweight dropped too low. RIP.");
        return;
    } else {
        CyberPartner::getInstance().attributes.weight = weight;
    }
}

/**
 * @brief Retrieves the weight of the CyberPartner instance.
 *
 * This function accesses the `CyberPartner` singleton instance and returns
 * the value of the `weight` attribute from its `attributes`.
 *
 * @return The weight of the CyberPartner instance as an integer.
 */
int CyberPartner::getWeight() {
    return CyberPartner::getInstance().attributes.weight;
}

/**
 * @brief Burns weight off of the CyberPartner (from exercise).
 * @param weight The session weight burned.
 * @note The weight is reduced by the burned amount, but not below 0.
 *      This function makes it the responsibility of the caller to ensure
 *     minimum CyberPartner wight values BEYOND keeping the value non-negative
 */
void burnWeight(int burnedWeight) {
    CyberPartner& partner = CyberPartner::getInstance();
    partner.attributes.burnedWeight += burnedWeight;

    // 20 Is an arbitrary value for the achievement and can be adjusted
    if (partner.attributes.burnedWeight >= 20) {
        if (!ProgressConfig::getInstance().isGotSkinnyUnlocked()) {
            Serial.printf("GotBurned Achievement Unlocked!\n");
            ProgressConfig::getInstance().unlockGotSkinny();
            ProgressConfig::getInstance().saveProgressFile();
        }
    }

    // This might kill the CyberPartner, so we do it last
    partner.setWeight(partner.attributes.weight - burnedWeight);
}

/**
 * @brief Sets the hunger level of the CyberPartner instance.
 *
 * This function updates the `hunger` attribute of the `CyberPartner` singleton instance
 * with the provided value.
 *
 * @param hunger The new hunger level to set for the CyberPartner.
 */
void CyberPartner::setHunger(int hunger) {
    CyberPartner::getInstance().attributes.hunger = hunger;
}

/**
 * @brief Increases the hunger level of the CyberPartner instance by 1.
 *
 * This function increments the `hunger` attribute of the `CyberPartner` singleton instance
 * by 1, ensuring that it does not exceed a predefined maximum value.
 *
 * @param maxHunger The maximum allowable hunger level.
 */
void CyberPartner::increaseHunger() {
    if (CyberPartner::getInstance().attributes.lifePhase == LifePhase::Death){
        return;
    }

    CyberPartner& partner = CyberPartner::getInstance();

    if (partner.attributes.hunger < maxHunger) {
        partner.attributes.hunger += 1;
        Serial.println("Hunger increased, now "+String(partner.attributes.hunger)+" of "+String(maxHunger));
    } else {
        // If it's your first hunger death, have an unlock
        if (!ProgressConfig::getInstance().isGotStarvedUnlocked()) {
            Serial.printf("GotStarved Achievement Unlocked!\n");
            ProgressConfig::getInstance().unlockGotStarved();
            ProgressConfig::getInstance().saveProgressFile();
        }
        CyberPartner::getInstance().die(WaysToDie::Starved);
        Serial.println("CyberPartner died of hunger.");
    }
}

void CyberPartner::decreaseHunger(int amount) {
    if (CyberPartner::getInstance().attributes.lifePhase == LifePhase::Death){
        return;
    }
    CyberPartner& partner = CyberPartner::getInstance();
    if (partner.attributes.hunger > 0) {
        partner.attributes.hunger -= amount;
        Serial.println("Hunger decreased, now "+String(partner.attributes.hunger)+" of "+String(maxHunger));
    } else {
        Serial.println("Hunger is already at minimum.");
    }
    if (partner.attributes.hunger < 0) {
        partner.attributes.hunger = 0;
    }
}

void CyberPartner::decreaseThirst(int amount) {
    if (CyberPartner::getInstance().attributes.lifePhase == LifePhase::Death){
        return;
    }
    CyberPartner& partner = CyberPartner::getInstance();
    if (partner.attributes.thirst > 0) {
        partner.attributes.thirst -= amount;
        Serial.println("Thirst decreased, now "+String(partner.attributes.thirst)+" of "+String(maxThirst));
    } else {
        Serial.println("Thirst is already at minimum.");
    }
    if (partner.attributes.thirst < 0) {
        partner.attributes.thirst = 0;
    }
}

/**
 * @brief Makes the CyberPartner consume a meal, reducing hunger/thirst if the meal is available.
 *
 * This function checks if the specified meal is available in the `CPStore`. If the meal
 * is available, it decreases the meal count in the store and reduces the CyberPartner's
 * hunger level by a fixed amount.
 *
 * @param meal The meal to consume (from `CPStoreMeals`).
 * @return True if the meal was successfully consumed, false otherwise.
 */
bool CyberPartner::consumeMeal(CPStoreMeals meal) {
    CPStore& store = CPStore::getInstance();

    // Check if the meal is available in the store
    if (store.getMealCount(meal) > 0) {
        // Reduce the meal count in the store
        store.consumeMeal(meal);

        // Reduce hunger level
        CyberPartner& partner = CyberPartner::getInstance();
        partner.attributes.hunger = max(0, partner.attributes.hunger - 10); // Reduce hunger by 10, but not below 0

        Serial.println("CyberPartner ate a meal and reduced hunger.");
        return true;
    }

    Serial.println("No meal available to eat.");
    return false;
}

/**
 * @brief Sets the thirst level of the CyberPartner instance.
 *
 * This function updates the `thirst` attribute of the `CyberPartner` singleton instance
 * with the provided value.
 *
 * @param thirst The new thirst level to set for the CyberPartner.
 */
void CyberPartner::setThirst(int thirst) {
    CyberPartner::getInstance().attributes.thirst = thirst;
}

/**
 * @brief Sets the happiness level of the CyberPartner instance.
 *
 * This function updates the `happiness` attribute of the `CyberPartner` singleton instance
 * with the provided value.
 *
 * @param happiness The new happiness level to set for the CyberPartner.
 */
void CyberPartner::setHappiness(int happiness) {
    CyberPartner::getInstance().attributes.happiness = happiness;
}

/**
 * @brief Increases the happiness level of the CyberPartner instance.
 *
 * This function increments the `happiness` attribute of the `CyberPartner` singleton instance
 * by a specified amount, ensuring that it does not exceed a predefined maximum value (150).
 *
 * @param happiness The amount to increase the happiness level by.
 */
void CyberPartner::increaseHappiness(int happiness) {
    int16_t currHappiness = CyberPartner::getInstance().attributes.happiness;
    CyberPartner::getInstance().setHappiness(std::min(currHappiness + happiness, 150));
}

/**
 * @brief Sets debug on the CyberPartner instance.
 *
 * This function sets the `debug` attribute of the `CyberPartner` singleton instance
 * with the provided value.
 *
 * @param dbug The debug bool value for the CyberPartner.
 */
void CyberPartner::setDebug(bool debug) {
    CyberPartner::getInstance().attributes.debug = debug;
}