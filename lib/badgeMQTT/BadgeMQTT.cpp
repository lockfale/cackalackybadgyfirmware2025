/**
 * @file BadgeMQTT.cpp
 * @brief Implements the BadgeMQTT class methods.
 *
 * This file contains the implementation of the BadgeMQTT class, which manages
 * the MQTT connection for the badge. It includes methods for initializing,
 * connecting, and disconnecting from WiFi, as well as checking the connection status.
 */
#include "BadgeMQTT.h"

#include <string>

#include "NetworkConfig.h"

bool BadgeMQTT::_loaded = false;
char BadgeMQTT::_mqtt_server[64] = {0};
int BadgeMQTT::_mqtt_port = 8883;
char BadgeMQTT::_mqtt_user[32] = {0};
char BadgeMQTT::_mqtt_password[64] = {0};
char BadgeMQTT::_mqttClientID[16] = {0};
char BadgeMQTT::_mqttRootTopic[11] = {0};

const char* BadgeMQTT::_fingerprint = "5F:EE:1F:E3:26:26:A5:49:1E:99:2E:2D:A9:D5:9F:63:9F:56:BE:D6"; // mqtt.cackalacky.ninja

/**
 * @brief Constructor for the BadgeMQTT class.
 *
 * Initializes the MQTT client with the WiFi client.
 */
BadgeMQTT::BadgeMQTT() : _pubSubClient(_wifiClient) {}

/**
 * @brief Gets the singleton instance of the BadgeMQTT class.
 * @return Reference to the singleton instance.
 */
BadgeMQTT& BadgeMQTT::getInstance() {
    static BadgeMQTT instance;
    return instance;
}

/**
 * @brief Sets up the MQTT connection.
 *
 * This function sets up the MQTT connection for the badge by loading the
 * MQTT credentials from the NetworkConfig and configuring the MQTT client.
 *
 * @param useSSL True if SSL should be used, false otherwise.
 */
void BadgeMQTT::setup(bool useSSL) {
    if (!_loaded) {
        if (!loadCredentials()) {
            Serial.println("Failed to load MQTT credentials from LittleFS!");
            return;
        }
        _loaded = true;
    }

    // Generate the Client ID once
    snprintf(_mqttClientID, sizeof(_mqttClientID), "%08X", ESP.getChipId());
    // set the root topic
    snprintf(_mqttRootTopic, sizeof(_mqttRootTopic), "cackalacky");

    if (useSSL) {
        _bearSSLClient.setFingerprint(_fingerprint);
        _bearSSLClient.setBufferSizes(512, 512);
        _pubSubClient.setClient(_bearSSLClient);
        #ifdef BADGEMQTTDEBUG
        Serial.println("Using SSL for MQTT");
        #endif
    } else {
        _pubSubClient.setClient(_wifiClient);
        #ifdef BADGEMQTTDEBUG
        Serial.println("Using plain TCP for MQTT");
        #endif
    }

    _pubSubClient.setServer(_mqtt_server, _mqtt_port);
    _pubSubClient.setCallback(staticCallback);
    _pubSubClient.setBufferSize(256, 256);
}

/**
 * @brief Checks if there are pending messages in the queue.
 * @return True if there are pending messages, false otherwise.
 */
bool BadgeMQTT::hasPendingMessages() {
    return !messageQueue.empty();
}

/**
 * @brief Pops a message from the queue.
 * @return The message as a pair of topic and payload.
 */
std::pair<std::string, std::vector<uint8_t>> BadgeMQTT::popMessage() {
    if (messageQueue.empty()) {
        return {"", {}};
    }

    auto message = messageQueue.front();
    messageQueue.pop();
    return message;
}


/**
 * @brief Static callback function for handling incoming messages.
 *
 * This function is required by the MQTT PubSubClient library. It adds the topic and payload
 * to a queue. The length parameter indicates the number of bytes in the payload.
 *
 * @param topic The topic of the incoming message.
 * @param payload The payload of the incoming message.
 * @param length The length of the payload.
 *
 * \note Required by MQTT PubSubClient: it wants a static function for callbacks.
 *
 * We add the topic and payload to a queue.
 * The length tells you how many bytes are in payload.
 *  - Prevents Buffer Overflows or Underflows
 *  - Handles binary data correctly
 *  - Ensures correct processing
 */
void BadgeMQTT::staticCallback(char* topic, byte* payload, unsigned int length) {
    std::vector<uint8_t> messagePayload(payload, payload + length);

    // Push message to queue
    BadgeMQTT::getInstance().messageQueue.push({std::string(topic), messagePayload});
}

/**
 * @brief Initializes the MQTT connection.
 */
void BadgeMQTT::begin() {
    connect();
}

/**
 * @brief Checks the MQTT connection status and processes incoming messages.
 *
 * This function should be called in the main loop to maintain the MQTT connection
 * and check for incoming messages.
 */
void BadgeMQTT::loop() {
    if (!_pubSubClient.connected()) {
        connect();
    }
    _pubSubClient.loop();
}

/**
 * @brief Disconnects from the MQTT server.
 */
void BadgeMQTT::disconnect() {
    Serial.println("\nDisconnecting from MQTT...");
    _pubSubClient.disconnect();
}

/**
 * @brief Checks if the badge is connected to the MQTT server.
 * @return True if connected, false otherwise.
 */
bool BadgeMQTT::isConnected() {
    return _pubSubClient.connected();
}

/**
 * @brief Publishes a message to the specified MQTT topic.
 *
 * This function publishes a message to the specified MQTT topic. It constructs the full topic
 * using the root topic and client ID, and then publishes the message.
 *
 * @param topic The MQTT topic to publish to.
 * @param message The message to publish.
 */
void BadgeMQTT::publish(const char* topic, const char* message) {
    if (_pubSubClient.connected()) {
        char fullTopic[256];  // Ensure enough space for prefix + topic
        snprintf(fullTopic, sizeof(fullTopic), "%s/badge/egress/%s/%s", _mqttRootTopic, _mqttClientID, topic);
        #ifdef BADGEMQTTDEBUG
        Serial.print("Publishing: ");
        Serial.print(fullTopic);
        Serial.print(" -> ");
        Serial.println(message);
        #endif
        _pubSubClient.publish(fullTopic, message, false);
    } else {
        Serial.println("MQTT not connected. Message not sent.");
    }
}

/**
 * @brief Subscribes to the specified MQTT topic.
 *
 * This function subscribes to the specified MQTT topic with a default QoS of 0.
 *
 * @param topic The MQTT topic to subscribe to.
 */
void BadgeMQTT::subscribe(const char* topic) {
    subscribe(topic, 0);
}

/**
 * @brief Subscribes to the specified MQTT topic with a specified QoS level.
 *
 * This function subscribes to the specified MQTT topic with the given QoS level.
 *
 * @param topic The MQTT topic to subscribe to.
 * @param qos The Quality of Service level for the subscription.
 */
void BadgeMQTT::subscribe(const char* topic, unsigned int qos) {
    if (_pubSubClient.connected()) {
        _pubSubClient.subscribe(topic, qos);
    }
}

/**
 * @brief Connects to the MQTT server with retry and backoff logic.
 *
 * This function attempts to establish a connection to the MQTT server using the
 * specified credentials and Last Will and Testament (LWT) message. If the connection
 * fails, it implements an exponential backoff strategy to retry the connection after
 * increasing delays. The retry state is reset upon a successful connection.
 *
 * The function performs the following steps:
 * - Checks if the client is already connected. If so, it resets the retry state and exits.
 * - Verifies if the current time has passed the next retry time before attempting a connection.
 * - Prepares the Last Will Topic string for the connection.
 * - Attempts to connect to the MQTT server. If successful:
 *   - Subscribes to relevant topics.
 *   - Resets the retry state.
 * - If the connection fails:
 *   - Logs the failure reason.
 *   - Calculates the next retry time using exponential backoff.
 *   - Resets the retry timer if the connection attempt times out.
 *
 * @note The exponential backoff delay is calculated as `min((1 << retryCount) * baseDelay, maxDelay)`,
 *       where `baseDelay` is 1 second and `maxDelay` is 30 seconds.
 *
 * @note Always subscribe to topics after a successful connection to ensure QoS is obeyed.
 *
 * @param None
 */
void BadgeMQTT::connect() {
    static unsigned long startAttemptTime = 0;
    static unsigned int retryCount = 0;                   // Retry counter
    static unsigned long nextRetryTime = 0;               // Time for the next retry

    constexpr size_t lastWillTopicSize = 128;

    // Check if already connected
    if (_pubSubClient.connected()) {
        resetRetryState(startAttemptTime, retryCount);
        return;
    }

    // Check if it's time for the next retry
    unsigned long currentMillis = millis();
    if (currentMillis < nextRetryTime) {
        return; // Wait until the backoff delay has passed
    }

    // Initialize the start attempt time
    if (startAttemptTime == 0) {
        startAttemptTime = currentMillis;
    }

    // Prepare the Last Will Topic
    char lastWillTopic[lastWillTopicSize];
    prepareLastWillTopic(lastWillTopic, lastWillTopicSize);

    // Attempt to connect
    Serial.print("Connecting to MQTT broker... ");
    if (_pubSubClient.connect(_mqttClientID, _mqtt_user, _mqtt_password, lastWillTopic, 0, 1, "offline", 0)) {
        Serial.println("MQTT Connected!");
        subscribeToTopics();
        resetRetryState(startAttemptTime, retryCount);
    } else {
        handleConnectionFailure(currentMillis, startAttemptTime, retryCount, nextRetryTime);
    }
}

/**
 * @brief Resets the retry state after a successful connection.
 */
void BadgeMQTT::resetRetryState(unsigned long& startAttemptTime, unsigned int& retryCount) {
    startAttemptTime = 0; // Reset the timer
    retryCount = 0;       // Reset retry counter
}

/**
 * @brief Prepares the Last Will Topic string.
 */
void BadgeMQTT::prepareLastWillTopic(char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%s/badge/egress/%s/status", _mqttRootTopic, _mqttClientID);
}

/**
 * @brief Handles connection failure and calculates the backoff delay.
 */
void BadgeMQTT::handleConnectionFailure(unsigned long currentMillis, unsigned long& startAttemptTime,
                                        unsigned int& retryCount, unsigned long& nextRetryTime) {

    constexpr unsigned long connectionTimeout = 100000;    // 100 seconds timeout
    constexpr unsigned long baseDelay = 10000;             // 10 seconds base delay
    constexpr unsigned long maxDelay = 300000;             // 300 seconds (5 minutes) max delay

    Serial.print("Failed, rc=");
    Serial.println(_pubSubClient.state());

    // Increment retry counter and calculate backoff delay
    retryCount++;
    unsigned long backoffDelay = min((1 << retryCount) * baseDelay, maxDelay);
    nextRetryTime = currentMillis + backoffDelay;

    Serial.print("Retrying in ");
    Serial.print(backoffDelay / 1000);
    Serial.println(" seconds...");

    // Reset the timer if the connection attempt timed out
    if (currentMillis - startAttemptTime >= connectionTimeout) {
        Serial.println("Connection attempt timed out.");
        startAttemptTime = 0; // Reset the timer
    }
}

/**
 * @brief Subscribes to the relevant MQTT topics.
 *
 * This function subscribes to the topics related to the badge and the world ingress.
 */
void BadgeMQTT::subscribeToTopics() {
    // Serial.println("Unsubscribing from old topics...");
    // _pubSubClient.unsubscribe("#");  // Unsubscribe from all previous topics

    Serial.println("Subscribing to topics...");
    char topic[128];
    snprintf(topic, sizeof(topic), "%s/badge/ingress/%s/#", _mqttRootTopic, _mqttClientID);
    Serial.println(topic);
    _pubSubClient.subscribe(topic, 1);

    //snprintf(topic, sizeof(topic), "%s/world/ingress/#", _mqttRootTopic);
    //Serial.println(topic);
    //_pubSubClient.subscribe(topic, 1);
}

/**
 * @brief Loads the MQTT credentials from the NetworkConfig.
 *
 * This function loads the MQTT server, port, username, and password from the NetworkConfig.
 *
 * @return True if the credentials are successfully loaded, false otherwise.
 */
bool BadgeMQTT::loadCredentials() {
    strncpy(_mqtt_server, NetworkConfig::getInstance().getMQTTServer().c_str(), sizeof(_mqtt_server) - 1);
    _mqtt_port = NetworkConfig::getInstance().getMQTTPort();
    strncpy(_mqtt_user, NetworkConfig::getInstance().getMQTTUser().c_str(), sizeof(_mqtt_user) - 1);
    strncpy(_mqtt_password, NetworkConfig::getInstance().getMQTTPassword().c_str(), sizeof(_mqtt_password) - 1);

    Serial.println("MQTT credentials loaded.");
    #ifdef BADGEMQTTDEBUG
        Serial.print(F("MQTT Server: mqtt://"));
        Serial.print(_mqtt_server);
        Serial.print(F(":"));
        Serial.println(_mqtt_port);
        Serial.print(F("MQTT User: "));
        Serial.print(_mqtt_user);
        Serial.print(F(" | Pass: "));
        Serial.println(_mqtt_password);
    #endif // BADGEMQTTDEBUG
    return true;
}
