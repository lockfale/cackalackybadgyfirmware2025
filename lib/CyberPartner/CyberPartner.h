/**
 * @file CyberPartner.h
 * @brief Defines the CyberPartner class, which manages state and attributes.
 *
 * This file declares the CyberPartner singleton, its attributes, and methods
 * for modifying and serializing its state. It also handles MQTT message processing.
 */
#pragma once

#include <Arduino.h>

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>
#include <functional>
#include <string>
#include <vector>
#include <queue>
#include <vector>
#include <CPStore.h>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define CPDEBUG // uncomment to enable badge debugging
#endif

/**
 * @enum LifePhase
 * @brief Represents the different life stages of a CyberPartner.
 */
enum class LifePhase {
    Egg = 0,        /**< Initial stage, CyberPartner is in an egg. */
    Baby = 30,
    Toddler = 200,
    Child = 500,
    Teen = 1300,
    Adult = 1800,
    Senior = 2900,
    Death = 3000    /**< End-of-life state, CyberPartner is dead. */
};

enum class State {
    ALIVE = 0,
    DEAD = 1,
    COLD = 2,
    HOT = 3,
    HUNGRY = 4,
    TIRED = 5,
    ANNOYED = 6,
    IDLE = 7,
    ACTION_ROLL = 8
};

enum class WaysToDie {
    NotDead = 0,
    AbandonedOwner = 1000,
    Starved = 1001,
    Dehydrated = 1002,
    Underweight = 1003,
    Overweight = 1004,
    Health = 1005,
    Thanos = 1100,
    Meteor = 1101,
    Earthquake = 1102,
    Tornado = 1103,
    Hurricane = 1104,
    Lightning = 1105,
    Goose = 1106,
    Spontaneous_combustion = 1107,
    NaturalCauses = 1337,
    DeathButton = 1200,
    Drop = 1201,
};

/**
 * @class CyberPartner
 * @brief Manages the attributes and state of the CyberPartner.
 *
 * This class follows a singleton pattern to ensure only one instance exists.
 * It manages CyberPartner's attributes such as age, weight, hunger, and state.
 * The class also integrates with MQTT for message processing and uses a
 * task scheduler for efficient event handling.
 *
 * Features:
 * - Tracks the CyberPartner's life phase and updates it based on age.
 * - Manages a queue for MQTT messages related to CyberPartner state updates.
 * - Serializes attributes into JSON format for external communication.
 */
class CyberPartner {
public:
    /**
     * @brief Retrieves the singleton instance of the CyberPartner class.
     *
     * This function provides access to the single instance of the `CyberPartner` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `CyberPartner` instance.
     *
     * @details
     * - The `CyberPartner` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * CyberPartner& partner = CyberPartner::getInstance();
     * partner.die();
     * ```
     */
    static CyberPartner& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    CyberPartner(const CyberPartner&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    CyberPartner& operator=(const CyberPartner&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    CyberPartner(CyberPartner&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    CyberPartner& operator=(CyberPartner&&) = delete;

    /**
     * @brief Task scheduler for handling timed events and background processing.
     *
     * The `taskRunner` and `hpTaskRunner` are responsible for managing scheduled
     * tasks that execute CyberPartner-related logic, ensuring efficiency in the
     * embedded environment.
     */
    static Scheduler& taskRunner;
    static Scheduler& hpTaskRunner;

    static int status; /**< Global status indicator. */

    /**
     * @brief Sets the birthTime of the CyberPartner.
     * @param birthTime The new birth time to set.
     */
    static void setBirthTime(time_t birthTime);

    /**
     * @brief Gets the age of the CyberPartner.
     * @return The current age of the CyberPartner.
     */
    static uint16_t getAge();
    static uint16_t determineAge();

    /**
     * @brief Sets the life phase of the CyberPartner based on its age.
     */
    static void setLifePhase();

    /**
     * @brief Gets the current life phase of the CyberPartner.
     * @return The current life phase of the CyberPartner.
     */
    static LifePhase getLifePhase();

    /**
     * @brief Converts a LifePhase enum to its corresponding string representation.
     * @param lifePhase The LifePhase enum value.
     * @return The string representation of the LifePhase.
     */
    static const char* lifePhaseToString(LifePhase lifePhase);

    /**
     * @brief Converts a WaysToDie enum to its corresponding string representation.
     * @param theWays The WaysToDie enum value.
     * @return The string representation of the WaysToDie.
     */
    static const char* WaysToDieToString(WaysToDie theWays);

    /**
     * @brief Sets the weight of the CyberPartner.
     * @param weight The new weight to set.
     */
    static void setWeight(int weight);

    /**
     * @brief Gets the weight of the CyberPartner.
     * @return The current weight of the CyberPartner.
     */
    static int getWeight();

    /**
     * @brief Burns weight off of the CyberPartner (from exercise).
     * @param weight The session weight burned.
     */
    static void burnWeight(int weight);

    /**
     * @brief Sets the hunger level of the CyberPartner.
     * @param amount The new hunger level to set.
     */
    static void setHunger(int amount);

    /**
     * @brief Increases the hunger level of the CyberPartner instance by 1.
     * @param maxHunger The maximum allowable hunger level.
     */
    static void increaseHunger();

    static void decreaseHunger(int amount);
    static void decreaseThirst(int amount);

    /**
     * @brief Makes the CyberPartner consume a meal, reducing hunger/thirst if the meal is available.
     * @param meal The meal to consume (from `CPStoreMeals`).
     * @return True if the meal was successfully consumed, false otherwise.
     */
    bool consumeMeal(CPStoreMeals meal);

    /**
     * @brief Sets the thirst level of the CyberPartner.
     * @param thirst The new thirst level to set.
     */
    static void setThirst(int thirst);

    /**
     * @brief Sets the happiness level of the CyberPartner.
     * @param happiness The new happiness level to set.
     */
    static void setHappiness(int happiness);

    /**
     * @brief Increase the happiness level of the CyberPartner.
     * @param happiness The new happiness level to set.
     */
    static void increaseHappiness(int happiness);

    /**
     * @brief Sets the state of the CyberPartner.
     * @param state The new state to set.
     */
    static void setState(State state);

    static WaysToDie getDeathCause();

    /**
     * @brief Converts a State enum to its corresponding string representation.
     * @param state The State enum value.
     * @return The string representation of the State.
     */
    static const char* stateToString(State state);

    /**
     * @brief Generate a new CyberPartner
     */
    static void bornAgain();

    /**
    * @brief Kills the current CyberPartner
    */
    static void die(WaysToDie cause = WaysToDie::NotDead);

    /**
     * @brief Sets the debug of the CyberPartner.
     * @param debug The new debug state to set.
     */
    static void setDebug(bool debug);

    /**
     * @brief Prints the current attributes of the CyberPartner to the Serial console.
     */
    static void printAttributes();

    /**
     * @brief Serializes the attributes of the CyberPartner to a JSON string.
     * @return A JSON string representing the attributes of the CyberPartner.
     */
    static JsonDocument serializeAttributes();

    /**
     * @brief Enqueues an MQTT message for processing.
     * @param topic The topic of the MQTT message.
     * @param payload The payload of the MQTT message.
     */
    void enqueueMQTTMessage(std::string topic, std::vector<uint8_t> payload);


    /**
     * @brief Processes the MQTT message queue.
     */
    void processMQTTQueue();

    static void queueGeneralUpdate(const char* topic, JsonDocument& doc);

    /**
     * @brief Loads the progress configuration from the file.
     * @return True if the configuration is successfully loaded, false otherwise.
     */
    static bool loadCPStatsFile();

    /**
     * @brief Saves the progress configuration to the file.
     * @return True if the configuration is successfully saved, false otherwise.
     */
    static bool saveCPStatsFile();

    /**
     * @struct Attributes
     * @brief Holds the core attributes of the CyberPartner.
     *
     * This struct contains essential properties that define the CyberPartner's
     * state, including its age, hunger level, mood, and life phase.
     */
    struct Attributes {
        bool rock = false;         /**< Indicates if the CyberPartner is in a rock state. */
        uint16_t age = 0;               /**< The age of the CyberPartner. */
        uint16_t weight = 5;            /**< The weight of the CyberPartner. */
        uint8_t hunger = 0;             /**< The hunger level of the CyberPartner. */
        uint16_t thirst = 0;            /**< The thirst level of the CyberPartner. */
        uint16_t happiness = 100;       /**< The happiness level of the CyberPartner. */
        uint16_t health = 100;         /**< The health level of the CyberPartner. */
        uint16_t burnedWeight = 0;          /**< The weight worked off at the gym by the CyberPartner. */
        State state = State::IDLE;      /**< The current state of the CyberPartner. */
        LifePhase lifePhase = LifePhase::Egg;  /**< The current life phase of the CyberPartner. */
        time_t ageChangeTime = 0;       /**< The last recorded life phase change timestamp. */
        time_t birthTime = 0;           /**< The timestamp when the CyberPartner was born. */
        WaysToDie deathCause = WaysToDie::NotDead; /**< The cause of death for the CyberPartner. */
        bool debug = false;             /**< Used for debugging of the CyberPartner. */
    };

    Attributes attributes; /**< Stores the attributes of the CyberPartner. */

protected:
    /**
     * @brief Default destructor for the CyberPartner class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~CyberPartner() = default;

private:
    /**
     * @brief Default constructor for the CyberPartner class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    CyberPartner() = default;

    /**
     * @enum CPSTORE_STATE_t
     * @brief Enumeration of QR code generation states.
     */
    enum class QUEUE_STATE_t {
        QUEUE_STATE_CREATE,
        QUEUE_STATE_UPDATE,
        QUEUE_STATE_DEATH
    };

    static const std::vector<std::pair<int, LifePhase>> dayToLifePhaseVector; /**< Vector mapping age to life phase. */
    std::queue<std::pair<std::string, std::vector<uint8_t>>> mqttQueue; /**< Queue to handle MQTT messages. */

    static constexpr const char *_progressfile = "/partner.json";

    /**
     * @brief Queues the CyberPartner state for processing.
     * @param state The state to queue.
     */
    static void queueCPState(QUEUE_STATE_t state, const char* reason = nullptr);

    static uint16_t virtualDaysSinceBirth();
    static uint16_t virtualYearsSinceBirth();

    /**
     * @brief Has the badged synced
     */
    bool hasSynced();
};
