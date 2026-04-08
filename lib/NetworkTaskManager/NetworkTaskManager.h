/**
 * @file NetworkTaskManager.h
 * @brief Network Task Manager for managing WiFi and MQTT tasks.
 *
 * This file defines the `NetworkTaskManager` class which is responsible for managing
 * various network-related tasks such as WiFi connectivity and MQTT message processing.
 * The class follows the Singleton design pattern to ensure only one instance exists.
 * It includes the setup and management of event queues, as well as task scheduling for
 * network-related operations.
 *
 * It also defines the `MQTTPublishData` structure to hold MQTT topic and message data.
 *
 * Key functionalities:
 * - Manage WiFi and MQTT tasks
 * - Queue incoming MQTT events
 * - Publish events to the network
 * - Singleton pattern to ensure a single instance
 */
#pragma once

#include <Arduino.h>
#include <queue>

#define _TASK_WDT_IDS // To enable task unique IDs
#define _TASK_SLEEP_ON_IDLE_RUN // Compile with support for entering IDLE SLEEP state for 1 ms if not tasks are scheduled to run
#define _TASK_LTS_POINTER       // Compile with support for Local Task Storage pointer
#define _TASK_SELF_DESTRUCT     // Enable tasks to "self-destruct" after disable
#define _TASK_PRIORITY

#include <TaskSchedulerDeclarations.h>
#include <functional>

/**
 * @brief Data structure to hold MQTT topic and message.
 */
struct MQTTPublishData {
    String topic;   ///< MQTT topic to publish to.
    String message; ///< Message to be published.
};

/**
 * @brief Singleton class responsible for managing network-related tasks like WiFi and MQTT.
 *
 * Handles network task management, including scheduling tasks for WiFi connectivity,
 * MQTT messaging, and publishing events to a network.
 */
class NetworkTaskManager {
public:
    /**
     * @brief Retrieves the singleton instance of the NetworkTaskManager.
     *
     * @return The singleton instance of NetworkTaskManager.
     */
    static auto getInstance() -> NetworkTaskManager&;

    // Delete copy constructor and assignment operator
    NetworkTaskManager(const NetworkTaskManager&) = delete;
    auto operator=(const NetworkTaskManager&) -> NetworkTaskManager& = delete;

    /**
     * @brief Setup the network-related tasks including WiFi and MQTT.
     */
    void setupNetworkTasks();

    /**
     * @brief Enqueues a received event for processing.
     *
     * @param topic The MQTT topic of the received event.
     * @param message The message associated with the topic.
     */
    void queueReceivedEventTask(const String& topic, const String& message);

    /**
     * @brief Queue an MQTT submission task to be processed later.
     *
     * @param topic The MQTT topic for the submission.
     * @param message The message to be submitted.
     */
    void queueNetworkSubmissionTask(const String& topic, const String& message);

    /*
        * @brief Checks the network queue for any pending tasks and processes them.
        */
    void taskNetworkQueueCheck();

    /**
     * @brief Processes the MQTT queue to handle any pending messages.
     */
    void processMQTTQueueIngress();

    /*
        * @brief Processes the MQTT queue to handle any pending messages.
        *
        * @return true if successful.
        */
    static bool onEnable();

    /**
     * @brief Disables the network task manager.
     */
    static void onDisable();

private:
    /**
     * @brief Private constructor to prevent multiple instances of the singleton.
     */
    explicit NetworkTaskManager();

    static Scheduler& taskRunner;           ///< Scheduler for managing network tasks.
    static Scheduler& highPrioTaskRunner;   ///< High-priority scheduler for critical tasks.

    // Event Queues
    std::queue<MQTTPublishData> receivedEventQueue; ///< Queue to store received events.
    std::queue<MQTTPublishData> eventQueueForNetSubmission; ///< Queue for events to be submitted to the network.

    /**
     * @brief WiFi task to handle WiFi-related operations.
     */
    static void taskWiFi();

    /**
     * @brief MQTT task to handle MQTT-related operations.
     */
    static void taskMQTT();

    /**
     * @brief Task to publish events from the event queue.
     */
    static void taskPublishEvent();
};
