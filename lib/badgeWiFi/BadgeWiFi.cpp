/**
 * @file BadgeWiFi.cpp
 * @brief Implements the BadgeWiFi class methods.
 *
 * This file contains the implementation of the BadgeWiFi class, which manages
 * the WiFi connection for the badge. It includes methods for initializing,
 * connecting, and disconnecting from WiFi, as well as checking the connection status.
 */
#include "BadgeWiFi.h"

#include <time.h>

#include "NetworkConfig.h"
#include "obfuscate.h"

/* Configuration of NTP */
#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"

time_t now;

bool BadgeWiFi::_creds_loaded = false;
bool BadgeWiFi::_connected = false;
bool BadgeWiFi::_connecting = false;
char BadgeWiFi::_ssid[32] = {0};
char BadgeWiFi::_password[70] = {0};
unsigned long BadgeWiFi::_lastAttemptMillis = 0;

/**
 * @brief Gets the singleton instance of the BadgeWiFi class.
 * @return Reference to the singleton instance.
 */
BadgeWiFi& BadgeWiFi::getInstance() {
    static BadgeWiFi instance;
    return instance;
}

/**
 * @brief Sets up the WiFi connection.
 *
 * This function sets up the WiFi connection for the badge by loading the
 * WiFi credentials from the NetworkConfig and configuring the WiFi settings.
 */
void BadgeWiFi::setup() {
    strncpy(_ssid, NetworkConfig::getInstance().getWifiSSID().c_str(), sizeof(_ssid) - 1);
    strncpy(_password, NetworkConfig::getInstance().getWifiPassword().c_str(), sizeof(_password) - 1);
    _creds_loaded = true;

    #ifdef BADGEWIFIDEBUG
        Serial.print(F("WIFI SSID: "));
        Serial.print(_ssid);
        Serial.print(F(" | Password: "));
        Serial.println(_password);
    #endif

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    configTime(MY_TZ, MY_NTP_SERVER);
}

/**
 * @brief Initializes the WiFi connection.
 *
 * This function attempts to connect to the WiFi network using the loaded credentials.
 */
void BadgeWiFi::begin() {
    if (!_connected && !_connecting) {  // Only attempt if not connected & not already trying
        _connecting = true;
        WiFi.begin(_ssid, _password);
        Serial.println("Connecting to WiFi...");
    }
}

/**
 * @brief Checks the WiFi connection status.
 *
 * This function should be called in the main loop to check the WiFi connection status
 * and attempt reconnection if necessary.
 */
void BadgeWiFi::loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if(!_connected) Serial.println("\n----- WiFi Connected!");
        _connecting = false;
        _connected = true;
    } else if (WiFi.status() != WL_CONNECTED) {
        if (!_connecting) {  // Only attempt connection once per cycle
            _connecting = true;
            _lastAttemptMillis = millis();
            WiFi.begin(_ssid, _password);
            Serial.println("Connecting to WiFi...");
        } else if (millis() - _lastAttemptMillis >= _retryInterval) {
            _lastAttemptMillis = millis();
        }
    } else {
        Serial.print("WiFi in unknown state: ");
        Serial.println(WiFi.status());
    }
}

/**
 * @brief Checks if the WiFi is connected.
 * @return True if the WiFi is connected, false otherwise.
 */
bool BadgeWiFi::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

/**
 * @brief Disconnects from WiFi and disables networking.
 *
 * This function disconnects the badge from WiFi and disables networking.
 */
void BadgeWiFi::disconnect() {
    if(_connected || _connecting) {
        WiFi.disconnect(true); // `true` clears stored credentials
        WiFi.mode(WIFI_OFF);   // Turns off WiFi radio
        _connected = false;
        _connecting = false;
    }
}