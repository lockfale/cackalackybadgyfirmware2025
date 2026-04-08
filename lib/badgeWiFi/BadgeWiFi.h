/**
 * @file BadgeWiFi.h
 * @brief Declares the BadgeWiFi class methods.
 *
 * This file contains the declaration of the BadgeWiFi class, which manages
 * the WiFi connection for the badge. It includes methods for initializing,
 * connecting, and disconnecting from WiFi, as well as checking the connection status.
 */
#pragma once

#include <ESP8266WiFi.h>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define BADGEWIFIDEBUG // Comment out to disable NetConfig debugging
#endif

/**
 * @class BadgeWiFi
 * @brief Manages the WiFi connection for the badge.
 *
 * This class handles the WiFi connection for the badge, including initialization,
 * connection, disconnection, and status checking. It follows the singleton pattern
 * to ensure only one instance of the class is created.
 */
class BadgeWiFi {
public:
    /**
     * @brief Retrieves the singleton instance of the BadgeWiFi class.
     *
     * This function provides access to the single instance of the `BadgeWiFi` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `BadgeWiFi` instance.
     *
     * @details
     * - The `BadgeWiFi` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * BadgeWiFi& matingInstance = BadgeWiFi::getInstance();
     * matingInstance.begin();
     * ```
     */
    static BadgeWiFi& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    BadgeWiFi(const BadgeWiFi&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    BadgeWiFi& operator=(const BadgeWiFi&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    BadgeWiFi(BadgeWiFi&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    BadgeWiFi& operator=(BadgeWiFi&&) = delete;

    /**
     * @brief Initializes the WiFi connection.
     */
    void begin();

    /**
     * @brief Sets up the WiFi connection.
     */
    void setup();

    /**
     * @brief Checks the WiFi connection status and attempts to reconnect if necessary.
     */
    void loop();

    /**
     * @brief Checks if the badge is connected to WiFi.
     * @return True if connected, false otherwise.
     */
    bool isConnected();

    /**
     * @brief Disconnects from the WiFi network.
     */
    void disconnect();

    protected:
    /**
     * @brief Default destructor for the BadgeWiFi class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~BadgeWiFi() = default;

private:
    /**
     * @brief Default constructor for the BadgeWiFi class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    BadgeWiFi() = default;

    static bool _connected;  /**< Flag indicating if WiFi is connected */
    static bool _connecting;  /**< Flag indicating if WiFi connection is in progress */

    static bool _creds_loaded;  /**< Flag indicating if WiFi credentials are loaded */
    static char _ssid[32];    /**< WiFi SSID */
    static char _password[70];  /** WiFi Password */
    static unsigned long _lastAttemptMillis;  /** Timestamp of the last connection attempt */
    const unsigned long _retryInterval = 500; /** Retry interval for connection attempts */
};
