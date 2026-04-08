/**
 * @file NetworkConfig.h
 * @brief Manages the network configuration for the badge.
 *
 * This file contains the declaration of the NetworkConfig class, which follows
 * a singleton pattern to ensure only one instance exists. It manages the WiFi
 * and MQTT configuration, including loading and saving the configuration to a file.
 */
#pragma once
#include <Arduino.h>
#include <string>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define NETCONFIGDEBUG // Comment out to disable NetConfig debugging
#endif

/**
 * @class NetworkConfig
 * @brief Manages the network configuration for the badge.
 *
 * This class follows a singleton pattern to ensure only one instance exists.
 * It manages the WiFi and MQTT configuration, including loading and saving
 * the configuration to a file.
 */
class NetworkConfig
{
public:
    /**
     * @brief Retrieves the singleton instance of the NetworkConfig class.
     *
     * This function provides access to the single instance of the `NetworkConfig` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `NetworkConfig` instance.
     *
     * @details
     * - The `NetworkConfig` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * NetworkConfig& netconfInstance = NetworkConfig::getInstance();
     * netconfInstance.doStart();
     * ```
     */
    static NetworkConfig& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    NetworkConfig(const NetworkConfig&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    NetworkConfig& operator=(const NetworkConfig&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    NetworkConfig(NetworkConfig&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    NetworkConfig& operator=(NetworkConfig&&) = delete;

    /**
     * @brief Gets the WiFi SSID.
     * @return Reference to the WiFi SSID string.
     */
    const String &getWifiSSID() const;

    /**
     * @brief Gets the WiFi Password.
     * @return Reference to the WiFi Password string.
     */
    const String &getWifiPassword() const;

    /**
     * @brief Gets the MQTT Server address.
     * @return Reference to the MQTT Server string.
     */
    const String &getMQTTServer() const;

    /**
     * @brief Gets the MQTT Port.
     * @return The MQTT Port number.
     */
    uint16_t getMQTTPort() const;

    /**
     * @brief Gets the MQTT User.
     * @return Reference to the MQTT User string.
     */
    String getMQTTUser() const;

    /**
     * @brief Gets the MQTT Password.
     * @return Reference to the MQTT Password string.
     */
    String getMQTTPassword() const;

    /**
     * @brief Sets the WiFi SSID.
     * @param ssid The new WiFi SSID to set.
     */
    void setWifiSSID(const String &ssid);

    /**
     * @brief Sets the WiFi Password.
     * @param ssid The new WiFi Password to set.
     */
    void setWifiPassword(const String &ssid);

    /**
     * @brief Sets the MQTT Server address.
     * @param ssid The new MQTT Server to set.
     */
    void setMQTTServer(const String &ssid);

    /**
     * @brief Sets the MQTT Port.
     * @param port The new MQTT Port to set.
     */
    void setMQTTPort(uint16_t port);

    /**
     * @brief Sets the MQTT User.
     * @param ssid The new MQTT User to set.
     */
    void setMQTTUser(const String &ssid);

    /**
     * @brief Sets the MQTT Password.
     * @param ssid The new MQTT Password to set.
     */
    void setMQTTPassword(const String &ssid);

    /**
     * @brief Loads the network configuration from the file.
     * @return True if the configuration is successfully loaded, false otherwise.
     */
    bool loadNetworkFile();

    /**
     * @brief Saves the network configuration to the file.
     * @return True if the configuration is successfully saved, false otherwise.
     */
    bool saveNetworkFile();

    void printNetwork();

    /**
     * @brief Resets the network configuration to default values.
     */
    void resetNetwork();

protected:
    /**
     * @brief Default destructor for the NetworkConfig class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~NetworkConfig() = default;

private:
    /**
     * @brief Default constructor for the NetworkConfig class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    NetworkConfig() = default;

    static String _wifi_ssid;
    static String _wifi_pass;
    static String _mqtt_server;
    static uint16_t _mqtt_port;
    static String _mqtt_user;
    static String _mqtt_pass;

    static constexpr const char *_networkconfigfile = "/network.json";

    /**
     * @brief Helper function to initialize LittleFS.
     * @return True if initialization is successful, false otherwise.
     */
    static bool initFS(); // Helper function to initialize LittleFS
};