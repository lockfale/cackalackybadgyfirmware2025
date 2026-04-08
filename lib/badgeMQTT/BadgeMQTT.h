#pragma once

#define MQTT_MAX_PACKET_SIZE 128
#define MQTT_MAX_TRANSFER_SIZE 128
#define MQTT_MAX_MESSAGE_SIZE 128

#include <queue>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define BADGEMQTTDEBUG // Comment out to disable NetConfig debugging
#endif

/**
 * TODO -> create a close connection (or just aggressively set timeout)
 * TODO -> figure out how to manage username / password
 * TODO -> what if we could store a bearssl session in flash?
      ---- https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/BearSSL_Sessions/BearSSL_Sessions.ino
 */

 /**
 * @class BadgeMQTT
 * @brief Manages the MQTT connection for the badge.
 *
 * This class handles the MQTT connection for the badge, including initialization,
 * connection, disconnection, and message handling. It follows the singleton pattern
 * to ensure only one instance of the class is created.
 */
class BadgeMQTT {
public:
    /**
     * @brief Retrieves the singleton instance of the BadgeMQTT class.
     *
     * This function provides access to the single instance of the `BadgeMQTT` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `BadgeMQTT` instance.
     *
     * @details
     * - The `BadgeMQTT` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * BadgeMQTT& mqttInstance = BadgeMQTT::getInstance();
     * mqttInstance.begin();
     * ```
     */
    static BadgeMQTT& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    BadgeMQTT(const BadgeMQTT&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    BadgeMQTT& operator=(const BadgeMQTT&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    BadgeMQTT(BadgeMQTT&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    BadgeMQTT& operator=(BadgeMQTT&&) = delete;

    /**
     * @brief Sets up the MQTT connection.
     * @param useSSL Whether to use SSL for the connection.
     */
    void setup(bool useSSL);

    /**
     * @brief Connects to the MQTT server.
     */
    void begin();

    /**
     * @brief Handles the MQTT loop, checking for incoming messages and maintaining the connection.
     */
    void loop();

    /**
     * @brief Disconnects from the MQTT server.
     */
    void disconnect();

    /**
     * @brief Checks if the badge is connected to the MQTT server.
     * @return True if connected, false otherwise.
     */
    bool isConnected();

    /**
     * @brief Publishes a message to the specified MQTT topic.
     * @param topic The MQTT topic to publish to.
     * @param message The message to publish.
     */
    void publish(const char* topic, const char* message);

    /**
     * @brief Subscribes to the specified MQTT topic.
     * @param topic The MQTT topic to subscribe to.
     */
    void subscribe(const char* topic);

    /**
     * @brief Subscribes to the specified MQTT topic with a specified QoS level.
     * @param topic The MQTT topic to subscribe to.
     * @param qos The Quality of Service level for the subscription.
     */
    void subscribe(const char* topic, unsigned int qos);

    /**
     * @brief Checks if there are any pending messages in the queue.
     * @return True if there are pending messages, false otherwise.
     */
    bool hasPendingMessages();

    /**
     * @brief Retrieves a message from the queue.
     * @return A pair containing the topic and the message payload.
     */
    std::pair<std::string, std::vector<uint8_t>> popMessage();

    protected:
    /**
     * @brief Default destructor for the BadgeMQTT class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~BadgeMQTT() = default;

private:
    /**
     * @brief Default constructor for the BadgeMQTT class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    BadgeMQTT();

    static bool _loaded; /**< Flag indicating if the MQTT credentials are loaded */
    static char _mqtt_server[64];   /**< MQTT Server address */
    static int _mqtt_port;          /**< MQTT Port number */
    static char _mqtt_user[32];     /**< MQTT Username */
    static char _mqtt_password[64]; /** MQTT Password */
    static char _mqttClientID[16];  /**< MQTT client ID */
    static char _mqttRootTopic[11]; /**< MQTT root topic: cackalacky */

    static const char* _fingerprint; /** SSL fingerprint for secure connection */

    BearSSL::WiFiClientSecure _bearSSLClient; /** Secure WiFi client for SSL connection */
    WiFiClient _wifiClient;     /** Regular WiFi client for non-SSL connection */
    PubSubClient _pubSubClient; /** PubSubClient for MQTT communication */

    /**
     * @brief Loads the MQTT credentials.
     * @return True if the credentials are loaded successfully, false otherwise.
     */
    bool loadCredentials();

    /**
     * @brief Connects to the MQTT server.
     */
    void connect();

    /**
     * @brief Resets the retry state after a successful connection.
     */
    void resetRetryState(unsigned long& startAttemptTime, unsigned int& retryCount);

    /**
     * @brief Prepares the Last Will Topic string.
     */
    void prepareLastWillTopic(char* buffer, size_t bufferSize);

    /**
     * @brief Handles connection failure and calculates the backoff delay.
     */
    void handleConnectionFailure(unsigned long currentMillis, unsigned long& startAttemptTime,
        unsigned int& retryCount, unsigned long& nextRetryTime);

    /**
     * @brief Subscribes to the necessary MQTT topics.
     */
    void subscribeToTopics();

    /**
     * @brief Callback function for handling incoming MQTT messages.
     * @param topic The topic of the incoming message.
     * @param payload The payload of the incoming message.
     * @param length The length of the payload.
     */
    static void callback(char* topic, byte* payload, unsigned int length);

    /**
     * @brief Static callback function for handling incoming MQTT messages.
     * @param topic The topic of the incoming message.
     * @param payload The payload of the incoming message.
     * @param length The length of the payload.
     */
    static void staticCallback(char* topic, byte* payload, unsigned int length);

    std::queue<std::pair<std::string, std::vector<uint8_t>>> messageQueue; /** Queue for storing incoming MQTT messages. */
};
