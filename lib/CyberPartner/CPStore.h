/**
 * @file CPStore.h
 * @brief Defines the CPStore class, which manages the money and puchased items.
 *
 * This file declares the CPStore singleton, its attributes, and methods
 * for manages the money and puchased items.
 */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <array>
#include "CPStoreSprite.h"
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define CPSTOREDEBUG // uncomment to enable badge debugging
#endif

enum class CPStoreReward {
    REWARD_MIN,
    REWARD_LOW,
    REWARD_MED,
    REWARD_HIGH,
    REWARD_MAX,
    REWARD_COUNT
};

/**
 * @class CPStore
 * @brief Manages the attributes and state of the CPStore.
 *
 * This class follows a singleton pattern to ensure only one instance exists.
 *
 * Features:
 * - Tracks the CPStore's money and purchased items.
 */
class CPStore {
public:
    /**
     * @brief Retrieves the singleton instance of the CPStore class.
     *
     * This function provides access to the single instance of the `CPStore` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `CPStore` instance.
     *
     * @details
     * - The `CPStore` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * CPStore& storeInstance = CPStore::getInstance();
     * storeInstance.getMoney();
     * ```
     */
    static CPStore& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    CPStore(const CPStore&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    CPStore& operator=(const CPStore&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    CPStore(CPStore&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    CPStore& operator=(CPStore&&) = delete;

    /**
     * @brief Sets the amount of money that the CyberPartner has.
     * @param amount The new amount of money to set.
     */
    static bool setMoney(uint32_t amount);

    /**
     * @brief Gets the amount of money that the CyberPartner has.
     * @return The current amount of money.
     */
    static uint32_t getMoney();

    /**
     * @brief Increases the amount of money that the CyberPartner has.
     * @param amount The new amount of money to added.
     */
    static bool increaseMoney(uint32_t amount);

    /**
     * @brief Decreases the amount of money that the CyberPartner has.
     * @param amount The amount of money to decrease.
     * @param save If true, the updated state will be saved.
     * @return True if the money was successfully decreased, false if there were insufficient funds.
     * @note If the `amount` is greater than the current money balance, the function will return `false`
     * and no changes will be made to the money balance.
     * @note If `save` is true, the updated state will be saved to persistent storage.
     * @note If `save` is false, the updated state will not be saved to persistent storage.
     */
    static bool decreaseMoney(uint32_t amount, bool save);

    /**
     * @brief Returns a fixed reward amount based on the reward level.
     * @param reward The reward level
     * (REWARD_MIN, REWARD_LOW, REWARD_MED, REWARD_HIGH, REWARD_MAX).
     * @return The fixed reward amount.
     * @note The reward amount is determined by the reward level (via enum)
     */
    static uint8_t getRewardAmount(CPStoreReward reward);

    /**
     * @brief Adds a fixed reward amount based on the reward level.
     * @param reward The reward level
     * (REWARD_MIN, REWARD_LOW, REWARD_MED, REWARD_HIGH, REWARD_MAX).
     */
    static bool earnReward(CPStoreReward reward);

    /**
     * @brief Purchases a meal and decreases the user's money by the meal's price.
     * @param meal The meal to purchase.
     * @return True if the purchase was successful, false otherwise.
     */
    static bool purchaseMeal(CPStoreMeals meal);

    /**
     * @brief Eat a meal and decreses the amount of that meal by 1.
     * @param meal The meal to decrease the count for.
     */
    static bool consumeMeal(CPStoreMeals meal);

    /**
     * @brief Retrieves the count of a purchased meal.
     * @param meal The meal to retrieve the count for.
     * @return The count of the specified meal.
     */
    static uint32_t getMealCount(CPStoreMeals meal);
    static void setMealCount(CPStoreMeals meal, uint32_t count);

    static uint32_t getMealPrice(CPStoreMeals meal);
    static void setMealPrice(CPStoreMeals meal, uint32_t price);

    static uint32_t getMealCost(CPStoreMeals meal);

    static const char* mealToString(CPStoreMeals state);

    /**
     * @brief Loads the progress configuration from the file.
     * @return True if the configuration is successfully loaded, false otherwise.
     */
    static bool loadStoreFile();

    /**
     * @brief Saves the progress configuration to the file.
     * @return True if the configuration is successfully saved, false otherwise.
     */
    static bool saveStoreFile();

    static void printStoreStats();

protected:
    /**
     * @brief Default destructor for the CPStore class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~CPStore() = default;

private:
    /**
     * @brief Default constructor for the CPStore class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    CPStore() = default;

    // Money
    static uint32_t money;
    static constexpr uint32_t maxMoney = 999999;

    // Reward values for each level
    static constexpr std::array<uint32_t, static_cast<size_t>(CPStoreReward::REWARD_COUNT)> rewardValues = {
        10,  // Min
        25,  // Low
        50,  // Med
        100, // High
        200  // Max
    };

    enum class CPStoreItemType {
        FOOD,
        DRINK
    };

    static constexpr std::array<CPStoreItemType, static_cast<size_t>(CPStoreMeals::MEAL_COUNT)> itemTypes = {
        CPStoreItemType::FOOD,  // MEAL_APPLE
        CPStoreItemType::FOOD,  // MEAL_BREAD
        CPStoreItemType::FOOD,  // MEAL_CEREAL
        CPStoreItemType::FOOD,  // MEAL_CONE
        CPStoreItemType::FOOD,  // MEAL_PUDDING
        CPStoreItemType::FOOD,  // MEAL_SCONE
        CPStoreItemType::FOOD,  // MEAL_SUSHI
        CPStoreItemType::FOOD,   // MEAL_TART
        CPStoreItemType::DRINK, // DRINK_MONSTER
        CPStoreItemType::DRINK, // DRINK_MALORT
        CPStoreItemType::DRINK, // DRINK_MILK
        CPStoreItemType::DRINK  // DRINK_SODA
    };

    static constexpr std::array<uint32_t, static_cast<size_t>(CPStoreMeals::MEAL_COUNT)> thirstDecreaseByMeal = {
        0,   // MEAL_APPLE
        0,   // MEAL_BREAD
        0,   // MEAL_CEREAL
        0,   // MEAL_CONE
        0,   // MEAL_PUDDING
        0,   // MEAL_SCONE
        0,   // MEAL_SUSHI
        0,    // MEAL_TART
        10,  // DRINK_MONSTER
        20,  // DRINK_MALORT
        30,  // DRINK_MILK
        40   // DRINK_SODA
    };

    // Amount hunger decreases by eating each meal. Might need to be adjusted.
    static constexpr std::array<uint32_t, static_cast<size_t>(CPStoreMeals::MEAL_COUNT)> hungerDecreaseByMeal = {
        10,  // Hunger decrease for MEAL_APPLE
        30,  // Hunger decrease for MEAL_BREAD
        20,  // Hunger decrease for MEAL_CEREAL
        40,  // Hunger decrease for MEAL_CONE
        60,  // Hunger decrease for MEAL_PUDDING
        50,  // Hunger decrease for MEAL_SCONE
        100, // Hunger decrease for MEAL_SUSHI
        80,   // Hunger decrease for MEAL_TART
        0,  // Hunger decrease for DRINK_MONSTER
        0,  // Hunger decrease for DRINK_MALORT
        0,  // Hunger decrease for DRINK_MILK
        0   // Hunger decrease for DRINK_SODA

    };

    static std::array<uint32_t, static_cast<size_t>(CPStoreMeals::MEAL_COUNT)> mealPrices;
    static std::array<uint32_t, static_cast<size_t>(CPStoreMeals::MEAL_COUNT)> purchasedMeals; // 4 corresponds to the number of CPStoreMeals
    static constexpr const char *_progressfile = "/store.json";

    /**
     * @brief Generates a JSON document representing the current store state.
     * @return A JsonDocument containing the store's money and purchased meals.
     */
    static JsonDocument generateStoreJson();
};