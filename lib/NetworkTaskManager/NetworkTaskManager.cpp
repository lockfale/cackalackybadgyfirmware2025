/**
 * @file NetworkTaskManager.cpp
 * @brief Implementation of the Network Task Manager class.
 *
 * This file implements the `NetworkTaskManager` class, which is responsible for
 * handling network-related tasks such as managing WiFi connectivity and MQTT message
 * processing. The class is implemented as a singleton to ensure only one instance exists
 * and provides methods for scheduling tasks, queuing events, and processing incoming
 * MQTT messages.
 *
 * The implementation includes methods to handle WiFi connectivity, MQTT event publishing,
 * and managing network event queues. It also provides callbacks for enabling and disabling
 * tasks, as well as for processing messages from the MQTT broker.
 *
 * The class interacts with the `BadgeWiFi`, `BadgeMQTT`, and `BadgeOneWireReader` classes
 * to manage WiFi and MQTT functionality and device-specific tasks.
 */
#include "NetworkTaskManager.h"
#include "BadgeWiFi.h"
#include "BadgeMQTT.h"
#include "BadgeOneWireReader.h"
#include "CyberPartner.h"

extern Scheduler baseTaskRunner;
extern Scheduler highPriorityTaskRunner;
Scheduler& NetworkTaskManager::taskRunner = baseTaskRunner;
Scheduler& NetworkTaskManager::highPrioTaskRunner = highPriorityTaskRunner;

/**
 * @brief Retrieves the singleton instance of the NetworkTaskManager.
 *
 * This ensures that only one instance of the NetworkTaskManager exists throughout the program.
 *
 * @return The singleton instance of NetworkTaskManager.
 */
NetworkTaskManager& NetworkTaskManager::getInstance() {
    static NetworkTaskManager instance;
    return instance;
}

/**
 * @brief Private constructor for the NetworkTaskManager singleton.
 *
 * This constructor is private to prevent direct instantiation from outside the class.
 */
NetworkTaskManager::NetworkTaskManager() = default;

/**
 * @brief Queue an event for processing by the network.
 *
 * Adds an MQTT event to the receivedEventQueue to be processed later.
 *
 * @param topic The MQTT topic.
 * @param message The MQTT message to be processed.
 */
void NetworkTaskManager::queueReceivedEventTask(const String& topic, const String& message) {
    receivedEventQueue.push({topic, message});
}

/**
 * @brief Queue a network submission task for later processing.
 *
 * Adds a network submission task to the eventQueueForNetSubmission.
 *
 * @param topic The MQTT topic.
 * @param message The message to be submitted.
 */
void NetworkTaskManager::queueNetworkSubmissionTask(const String& topic, const String& message) {
    eventQueueForNetSubmission.push({topic, message});
}

/**
 * @brief Setup tasks related to network operations, including WiFi and MQTT.
 */
void NetworkTaskManager::setupNetworkTasks() {
    BadgeWiFi::getInstance().setup();
    BadgeMQTT::getInstance().setup(false);
    queueReceivedEventTask("status", "online");
}

/**
 * @brief WiFi task that checks WiFi connectivity and schedules the MQTT task.
 */
void NetworkTaskManager::taskWiFi() {
    if (BadgeWiFi::getInstance().isConnected()) {
        Task *t2 = new Task(0, TASK_ONCE, &NetworkTaskManager::getInstance().taskMQTT, &NetworkTaskManager::getInstance().highPrioTaskRunner, true,
            NetworkTaskManager::getInstance().onEnable,
            NetworkTaskManager::getInstance().onDisable,
            true);
    }
    BadgeWiFi::getInstance().loop();
}

/**
 * @brief MQTT task that processes incoming messages from the MQTT broker.
 *
 * This task reads messages from the receivedEventQueue and schedules the task to publish the event to the network.
 */
void NetworkTaskManager::taskMQTT() {
    BadgeMQTT::getInstance().loop();  // Handle MQTT connection

    if (!NetworkTaskManager::getInstance().receivedEventQueue.empty()) {
        MQTTPublishData netTask = NetworkTaskManager::getInstance().receivedEventQueue.front();
        NetworkTaskManager::getInstance().receivedEventQueue.pop();
        NetworkTaskManager::getInstance().queueNetworkSubmissionTask(netTask.topic, netTask.message);

        Task *t2 = new Task(0, TASK_ONCE, &taskPublishEvent, &NetworkTaskManager::getInstance().taskRunner, true,
            NetworkTaskManager::getInstance().onEnable,
            NetworkTaskManager::getInstance().onDisable,
        true);
    }
}

/**
 * @brief Publish event task that submits events to the network.
 *
 * This task retrieves an event from the submission queue and attempts to publish it over MQTT.
 */
void NetworkTaskManager::taskPublishEvent() {
    if (NetworkTaskManager::getInstance().eventQueueForNetSubmission.empty()) return;

    MQTTPublishData netTask = NetworkTaskManager::getInstance().eventQueueForNetSubmission.front();
    NetworkTaskManager::getInstance().eventQueueForNetSubmission.pop();

    if (BadgeMQTT::getInstance().isConnected()) {
        BadgeMQTT::getInstance().publish(netTask.topic.c_str(), netTask.message.c_str());
    } else {
        NetworkTaskManager::getInstance().queueReceivedEventTask(netTask.topic, netTask.message);
    }
}

/**
 * @brief Task to check the network event queues.
 *
 * This task checks the WiFi connection and schedules the WiFi and MQTT tasks accordingly.
 */
void NetworkTaskManager::taskNetworkQueueCheck() {
    Task *t2 = new Task(0, TASK_ONCE, &NetworkTaskManager::getInstance().taskWiFi, &NetworkTaskManager::getInstance().highPrioTaskRunner, true,
        NetworkTaskManager::getInstance().onEnable,
        NetworkTaskManager::getInstance().onDisable,
    true);
    // todo => determine when to D/C
    // if(BadgeMQTT::getInstance().isConnected()) BadgeMQTT::getInstance().disconnect();
    // if(BadgeWiFi::getInstance().isConnected()) BadgeWiFi::getInstance().disconnect();
}

/**
 * @brief Callback function to enable tasks.
 *
 * This function is invoked when enabling a task.
 *
 * @return true if the task can be enabled.
 */
bool NetworkTaskManager::onEnable() {
    return true;
}

/**
 * @brief Callback function to disable tasks.
 *
 * This function is invoked when disabling a task.
 */
void NetworkTaskManager::onDisable() {}

/**
 * @brief Process MQTT messages that are ingressed from the network.
 *
 * Handles incoming MQTT messages based on their topics, and routes them to appropriate handlers.
 */
void NetworkTaskManager::processMQTTQueueIngress() {
    if (!BadgeMQTT::getInstance().isConnected()) return;
    if (!BadgeMQTT::getInstance().hasPendingMessages()) return;

    Serial.println("Processing MQTT queue ingress...");

    auto [topic, payload] = BadgeMQTT::getInstance().popMessage();

    if (topic.find("cp/") != std::string::npos) {
        CyberPartner::getInstance().enqueueMQTTMessage(topic, payload);
    } else if (topic.find("ibutton/appmem/generate") != std::string::npos) {
        BadgeOneWireReader::getInstance().enqueueMQTTMessage(topic, payload);
    } else {
        Serial.print("No handler for topic.");
        Serial.println(topic.c_str());
        //Serial.println(payload.c_str());
    }
}
