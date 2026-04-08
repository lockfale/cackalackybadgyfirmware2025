/**
 * @file CPStore.cpp
 * @brief Implements the CPPStore class methods.
 *
 * This file contains the logic for handling CPPStore's money,
 * and puchased items.
 */
#include "CPStore.h"
#include <LittleFS.h>
#include "CryptoUtils.h"
#include "NetworkTaskManager.h"
#include "CPStore_bitmaps.h"
#include "accelerometer.h"
#include "CyberPartner.h"


uint32_t CPStore::money = 0;
std::array<uint32_t, static_cast<size_t>(CPStoreMeals::MEAL_COUNT)> CPStore::purchasedMeals = {0};

std::array<uint32_t, static_cast<size_t>(CPStoreMeals::MEAL_COUNT)> CPStore::mealPrices = {
    10,  // Price for MEAL_APPLE
    30,  // Price for MEAL_BREAD
    20,  // Price for MEAL_CEREAL
    40,  // Price for MEAL_CONE
    60,  // Price for MEAL_PUDDING
    50,  // Price for MEAL_SCONE
    100, // Price for MEAL_SUSHI
    80,  // Price for MEAL_TART
    10,  // Price for DRINK_MONSTER
    20,  // Price for DRINK_MALORT
    30,  // Price for DRINK_MILK
    40   // Price for DRINK_SODA
};

CPStore& CPStore::getInstance() {
    static CPStore instance;
    return instance;
}


/**
 * @brief Sets the amount of money that the CyberPartner has.
 * @param amount The new amount of money to set.
 */
bool CPStore::setMoney(uint32_t amount)
{
    if (amount <= maxMoney) {
        money = amount;
        saveStoreFile();
        return true;
    }
    return false;
}

/**
 * @brief Gets the amount of money that the CyberPartner has.
 * @return The current amount of money.
 */
uint32_t CPStore::getMoney()
{
    return money;
}

/**
 * @brief Increases the amount of money that the CyberPartner has.
 * @param amount The new amount of money to added.
 */
bool CPStore::increaseMoney(uint32_t amount)
{
    if (amount <= 0) {
        return false;
    }
    if (money + amount <= maxMoney) {
        money += amount;
        saveStoreFile();
        return true;
    }
    return false;
}

/**
 * @brief Decreases the amount of money the CyberPartner has.
 *
 * This function deducts the specified amount from the CyberPartner's current money balance
 * if sufficient funds are available. Optionally, it can save the updated state to persistent
 * storage by calling `saveStoreFile()`.
 *
 * @param amount The amount of money to decrease.
 * @param save A boolean flag indicating whether to save the updated state to persistent storage.
 *             - `true`: Save the updated state after decreasing the money.
 *             - `false`: Do not save the updated state.
 * @return True if the money was successfully decreased, false if there were insufficient funds.
 *
 * @note If the `amount` is greater than the current money balance, the function will return `false`
 *       and no changes will be made to the money balance.
 */
bool CPStore::decreaseMoney(uint32_t amount, bool save)
{
    if (money >= amount) {
        money -= amount;
        if (save) {
            saveStoreFile();
        }
        return true;
    }
    return false;
}

/**
 * @brief Returns a fixed reward amount based on the reward level.
 * @param reward The reward level
 * (REWARD_MIN, REWARD_LOW, REWARD_MED, REWARD_HIGH, REWARD_MAX).
 */
uint8_t CPStore::getRewardAmount(CPStoreReward reward) {
    return rewardValues[static_cast<size_t>(reward)];;
}

/**
 * @brief Increases the amount of money that the CyberPartner has by a fixed value.
 * @param reward The fixed value of currency to add. See CPStoreReward enum
 * (REWARD_MIN, REWARD_LOW, REWARD_MED, REWARD_HIGH, REWARD_MAX).
 * @return True if the reward was successfully added, false otherwise.
 */
bool CPStore::earnReward(CPStoreReward reward) {
    // Map the reward level to the corresponding fixed value
    uint32_t rewardAmount = rewardValues[static_cast<size_t>(reward)];

    // Check if the reward amount is valid
    if (rewardAmount <= 0) {
        return false;
    }

    // Try to add the reward
    return increaseMoney(rewardAmount);
}

/**
 * @brief Purchases a meal and decreases the user's money by the meal's price.
 *
 * This function checks if the user has enough money to purchase the specified meal.
 * If the user has sufficient funds, the meal count is increased, and the money is
 * decreased by the price of the meal.
 *
 * @param meal The meal to purchase.
 * @return True if the purchase was successful, false otherwise.
 */
bool CPStore::purchaseMeal(CPStoreMeals meal) {
    size_t mealIndex = static_cast<size_t>(meal);

    // Ensure the meal index is valid, get the price
    if (mealIndex >= static_cast<size_t>(CPStoreMeals::MEAL_COUNT)) {
        return false;
    }
    uint32_t price = mealPrices[mealIndex];

    // Check if the user has enough money
    if (decreaseMoney(price, false)) {
        purchasedMeals[mealIndex]++;

        // Generate the JSON document being sent to the MQTT broker
        JsonDocument queueDoc;
        queueDoc["ts"] = time(nullptr);    // Add a timestamp to the JSON document
        char cpEvent[64]; // Adjust size as needed
        snprintf(cpEvent, sizeof(cpEvent), "purchase.%s", mealToString(meal));
        queueDoc["cp_event"] = cpEvent;

        // Serialize the JSON document to a character buffer
        static char payload[512]; // Adjust size as needed
        serializeJson(queueDoc, payload, sizeof(payload));
        NetworkTaskManager::getInstance().queueReceivedEventTask("cp/store/state", payload);

        saveStoreFile(); // Save the updated meal count
        return true;
    }

    // Not enough money
    return false;
}

/**
 * @brief Eat a meal and decreses the amount of that meal by 1.
 * @param meal The meal to decrease the count for.
 *
 * @note This function needs to decrease hunger based on the meal eaten.
 */
bool CPStore::consumeMeal(CPStoreMeals meal) {
    CPStore& store = CPStore::getInstance();
    size_t mealIndex = static_cast<size_t>(meal);

    if (store.purchasedMeals[mealIndex] > 0) {
        store.purchasedMeals[mealIndex]--;

        // Generate the JSON document being sent to the MQTT broker
        JsonDocument queueDoc;
        queueDoc["ts"] = time(nullptr);    // Add a timestamp to the JSON document
        char cpEvent[64]; // Adjust size as needed

        // Adjust hunger or thirst based on the item type
        if (itemTypes[static_cast<size_t>(meal)] == CPStoreItemType::FOOD) {
            CyberPartner::getInstance().decreaseHunger(hungerDecreaseByMeal[mealIndex]);
            snprintf(cpEvent, sizeof(cpEvent), "eat.%s", mealToString(meal));
        } else if (itemTypes[static_cast<size_t>(meal)] == CPStoreItemType::DRINK) {
            CyberPartner::getInstance().decreaseThirst(thirstDecreaseByMeal[mealIndex]);
            snprintf(cpEvent, sizeof(cpEvent), "drink.%s", mealToString(meal));
        }

        queueDoc["cp_event"] = cpEvent;

        // Serialize the JSON document to a character buffer
        static char payload[512]; // Adjust size as needed
        serializeJson(queueDoc, payload, sizeof(payload));
        NetworkTaskManager::getInstance().queueReceivedEventTask("cp/state/update", payload);

        saveStoreFile(); // Save the updated meal count

        return true;
    }

    // Not enough meals
    return false;
}

/**
 * @brief Retrieves the count of a purchased meal.
 * @param meal The meal to retrieve the count for.
 * @return The count of the specified meal.
 */
uint32_t CPStore::getMealCount(CPStoreMeals meal) {
    return purchasedMeals[static_cast<size_t>(meal)];
}

uint32_t CPStore::getMealPrice(CPStoreMeals meal) {
    return mealPrices[static_cast<size_t>(meal)];
}

// Setter for meal prices
void CPStore::setMealPrice(CPStoreMeals meal, uint32_t price) {
    mealPrices[static_cast<size_t>(meal)] = price;
}

void CPStore::setMealCount(CPStoreMeals meal, uint32_t count) {
    size_t mealIndex = static_cast<size_t>(meal);

    // Ensure the meal index is valid
    if (mealIndex < static_cast<size_t>(CPStoreMeals::MEAL_COUNT)) {
        purchasedMeals[mealIndex] = count;
    }
}

/**
 * @brief Retrieves the cost of a specific meal.
 *
 * This function returns the price of the specified meal by looking up the
 * corresponding value in the `mealPrices` array.
 *
 * @param meal The meal for which the cost is to be retrieved.
 * @return The cost of the specified meal as a `uint32_t`.
 *
 * @note Ensure that the `meal` parameter is a valid `CPStoreMeals` enum value.
 *       Passing an invalid value may result in undefined behavior.
 */
uint32_t CPStore::getMealCost(CPStoreMeals meal) {
    return mealPrices[static_cast<size_t>(meal)];
}

/**
 * @brief Converts a CPStoreMeals enumeration value to its string representation.
 *
 * This function takes a `CPStoreMeals` enum value and returns a corresponding
 * string that represents the meal in a human-readable format.
 *
 * If the provided `CPStoreMeals` value is not recognized, the function returns "UNKNOWN".
 *
 * @param meal The `CPStoreMeals` enumeration value to convert.
 * @return A string representing the meal.
 */
const char* CPStore::mealToString(CPStoreMeals meal) {
    switch (meal) {
        case CPStoreMeals::MEAL_APPLE: return "apple";
        case CPStoreMeals::MEAL_BREAD: return "bread";
        case CPStoreMeals::MEAL_CEREAL: return "cereal";
        case CPStoreMeals::MEAL_CONE: return "cone";
        case CPStoreMeals::MEAL_PUDDING: return "pudding";
        case CPStoreMeals::MEAL_SCONE: return "scone";
        case CPStoreMeals::MEAL_SUSHI: return "sushi";
        case CPStoreMeals::MEAL_TART: return "tart";
        case CPStoreMeals::DRINK_MONSTER: return "monster";
        case CPStoreMeals::DRINK_MALORT: return "malort";
        case CPStoreMeals::DRINK_MILK: return "milk";
        case CPStoreMeals::DRINK_SODA: return "soda";
        default: return "UNKNOWN";
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
bool CPStore::loadStoreFile() {
    JsonDocument doc;

    if (!CryptoUtils::loadEncryptedFile(_progressfile, doc, 0xB3C7D8E9)) {
        Serial.println("CPStore::loadStoreFile failed");
        saveStoreFile(); // Reset to defaults if loading fails
        return false;
    }

    money = doc["money"].as<uint32_t>() | 0;
    for(size_t i = 0; i < static_cast<size_t>(CPStoreMeals::MEAL_COUNT); i++) {
        const char* mealName = mealToString(static_cast<CPStoreMeals>(i));
        purchasedMeals[i] = doc[mealName].as<uint32_t>() | 0;
    }

    return true;
}

/**
 * @brief Saves the current store state to a file in the LittleFS filesystem.
 *
 * This function generates a JSON representation of the store's current state,
 * including the amount of money and the count of each purchased meal, and writes
 * it to a file in the LittleFS filesystem. The file is specified by the `_progressfile`
 * constant.
 *
 * @return True if the store state is successfully saved to the file, false otherwise.
 *
 * @note If the file cannot be created or written to, an error message is logged to the Serial monitor.
 * @note The JSON document is generated using the `generateStoreJson` function.
 */
bool CPStore::saveStoreFile() {
    #ifdef CPSTOREDEBUG
    Serial.println("CPStore::saveStoreFile()");
    #endif
    // Generate the JSON document
    JsonDocument storeStateDoc = generateStoreJson();

    return CryptoUtils::saveEncryptedFile(_progressfile, storeStateDoc, 0xB3C7D8E9);
}

/**
 * @brief Generates a JSON document representing the current store state.
 *
 * This function creates a JSON document containing the current state of the store,
 * including the amount of money and the count of each purchased meal. The meal names
 * are used as keys, and their respective counts are used as values in the JSON object.
 *
 * @return A `JsonDocument` containing the store's money and purchased meals.
 *
 * @note The JSON document is dynamically allocated. Ensure that the caller handles
 *       memory usage appropriately to avoid memory leaks or fragmentation.
 */
JsonDocument CPStore::generateStoreJson() {
    JsonDocument doc;

    doc["money"] = money;

    for (size_t i = 0; i < static_cast<size_t>(CPStoreMeals::MEAL_COUNT); i++) {
        const char* mealName = mealToString(static_cast<CPStoreMeals>(i));
        if (purchasedMeals[i] > 0) {
            doc[mealName] = purchasedMeals[i];
        }
    }

    return doc;
}

/**
 * @brief Prints the current store state to the Serial monitor.
 *
 * This function generates a JSON representation of the store's current state,
 * including the amount of money and the count of each purchased meal. The JSON
 * string is then printed to the Serial monitor for debugging or informational purposes.
 *
 * @note The JSON is serialized into a `String` before being printed, which may
 *       cause heap fragmentation on memory-constrained devices like the ESP8266.
 *       Consider optimizing this if memory usage becomes an issue.
 */
void CPStore::printStoreStats() {
    Serial.println(generateStoreJson().as<String>().c_str());
}