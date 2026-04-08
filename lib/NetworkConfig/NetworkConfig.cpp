/**
 * @file NetworkConfig.cpp
 * @brief Implements the NetworkConfig class methods.
 *
 * This file contains the implementation of the NetworkConfig class, which manages
 * the WiFi and MQTT configuration for the badge. It includes methods for loading
 * and saving the configuration to a file, as well as initializing the file system.
 */
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "obfuscate.h"
#include "NetworkConfig.h"
#include "CryptoUtils.h"

namespace {
    // Default credentials (obfuscated)
    const char* _badge_default_wifi_ssid = AY_OBFUSCATE("your_wifi_ssid");
    const char* _badge_default_wifi_pass = AY_OBFUSCATE("your_wifi_password");
    const char* _badge_default_mqtt_server = AY_OBFUSCATE("your_mqtt_server");
    constexpr uint16_t _badge_default_mqtt_port = 1883;
    const char* _badge_default_mqtt_user = AY_OBFUSCATE("your_mqtt_username");
    const char* _badge_default_mqtt_pass = AY_OBFUSCATE("your_mqtt_password");

    // XOR keys for obfuscation
    const char MQTT_USER_KEY = 'U';
    const char MQTT_PASS_KEY = 'P';
}
// Static member definitions
String NetworkConfig::_wifi_ssid;
String NetworkConfig::_wifi_pass;
String NetworkConfig::_mqtt_server;
uint16_t NetworkConfig::_mqtt_port = 0;
String NetworkConfig::_mqtt_user;
String NetworkConfig::_mqtt_pass;

NetworkConfig& NetworkConfig::getInstance() {
    static NetworkConfig instance;
    return instance;
}

/**
 * @brief Gets the WiFi SSID.
 * @return Reference to the WiFi SSID string.
 */
const String& NetworkConfig::getWifiSSID() const { return _wifi_ssid; }

/**
 * @brief Gets the WiFi Password.
 * @return Reference to the WiFi Password string.
 */
const String& NetworkConfig::getWifiPassword() const { return _wifi_pass; }

/**
 * @brief Gets the MQTT Server address.
 * @return Reference to the MQTT Server string.
 */
const String& NetworkConfig::getMQTTServer() const { return _mqtt_server; }

/**
 * @brief Gets the MQTT Port.
 * @return The MQTT Port number.
 */
uint16_t NetworkConfig::getMQTTPort() const { return _mqtt_port; }

/**
 * @brief Gets the MQTT User.
 * @return Reference to the MQTT User string.
 */
String NetworkConfig::getMQTTUser() const {
    #ifdef NETCONFIGDEBUG
    Serial.println("NetworkConfig::getMQTTUser()");
    #endif
    return _mqtt_user;
 }

/**
 * @brief Gets the MQTT Password.
 * @return Reference to the MQTT Password string.
 */
String NetworkConfig::getMQTTPassword() const {
    #ifdef NETCONFIGDEBUG
    Serial.println("NetworkConfig::getMQTTPassword()");
    #endif
    return _mqtt_pass;
 }

/**
 * @brief Sets the WiFi SSID.
 * @param ssid The new WiFi SSID to set.
 */
void NetworkConfig::setWifiSSID(const String& ssid) { _wifi_ssid = ssid; }

/**
 * @brief Sets the WiFi Password.
 * @param pass The new WiFi Password to set.
 */
void NetworkConfig::setWifiPassword(const String& pass) { _wifi_pass = pass; }

/**
 * @brief Sets the MQTT Server address.
 * @param server The new MQTT Server to set.
 */
void NetworkConfig::setMQTTServer(const String& server) { _mqtt_server = server; }

/**
 * @brief Sets the MQTT Port.
 * @param port The new MQTT Port to set.
 */
void NetworkConfig::setMQTTPort(uint16_t port) { _mqtt_port = port; }

/**
 * @brief Sets the MQTT User.
 * @param user The new MQTT User to set.
 */
void NetworkConfig::setMQTTUser(const String& user) { _mqtt_user = user; }

/**
 * @brief Sets the MQTT Password.
 * @param pass The new MQTT Password to set.
 */
void NetworkConfig::setMQTTPassword(const String& pass) { _mqtt_pass = pass; }

/**
 * @brief Initializes the LittleFS file system.
 * @return true if initialization is successful, false otherwise.
 */
bool NetworkConfig::initFS() {
    static bool fsInitialized = false;
    if (!fsInitialized) {
        fsInitialized = LittleFS.begin();
        if (!fsInitialized) {
            Serial.println("Failed to mount LittleFS!");
        }
    }
    return fsInitialized;
}

/**
 * @brief Resets the network configuration to default values.
 */
void NetworkConfig::resetNetwork() {
    _wifi_ssid   = String(_badge_default_wifi_ssid);
    _wifi_pass   = String(_badge_default_wifi_pass);
    _mqtt_server = String(_badge_default_mqtt_server);
    _mqtt_port   = _badge_default_mqtt_port;
    _mqtt_user   = String(_badge_default_mqtt_user);
    _mqtt_pass   = String(_badge_default_mqtt_pass);
}

/**
 * @brief Loads the network configuration from the file.
 * @return True if the configuration is successfully loaded, false otherwise.
 */
bool NetworkConfig::loadNetworkFile() {
    #ifdef NETCONFIGDEBUG
    Serial.println("NetworkConfig::loadNetworkFile()");
    #endif

    // Check if the file exists
    if (!LittleFS.exists(_networkconfigfile)) {
        #ifdef NETCONFIGDEBUG
        Serial.println("Configuration file does not exist. Using default settings.");
        #endif
        resetNetwork(); // Reset to default values
        return true;    // Return true since defaults are valid
    }

    JsonDocument doc;
    if (!CryptoUtils::loadEncryptedFile(_networkconfigfile, doc, 0xB7C4E9A2)) {
        Serial.println("NetworkConfig::loadNetworkFile failed");
        resetNetwork(); // Reset to defaults if loading fails
        saveNetworkFile(); // Save defaults to create a new file
        return false;
    }

    _wifi_ssid   = doc["wifi-ssid"] | String(_badge_default_wifi_ssid);
    _wifi_pass   = doc["wifi-pass"] | String(_badge_default_wifi_pass);
    _mqtt_server = doc["mqtt_server"] | String(_badge_default_mqtt_server);
    _mqtt_port   = doc["mqtt_port"] | _badge_default_mqtt_port;
    _mqtt_user   = doc["mqtt_user"] | String(_badge_default_mqtt_user);
    _mqtt_pass   = doc["mqtt_password"] | String(_badge_default_mqtt_pass);

    return true;
}

/**
 * @brief Saves the network configuration to the file.
 * @return True if the configuration is successfully saved, false otherwise.
 */
bool NetworkConfig::saveNetworkFile() {
    #ifdef NETCONFIGDEBUG
    Serial.println("Saving network configuration file.");
    #endif

    JsonDocument doc;
    // Only save options that differ from the defaults
    if (_wifi_ssid != String(_badge_default_wifi_ssid)) {
        doc["wifi-ssid"] = _wifi_ssid;
    }
    if (_wifi_pass != String(_badge_default_wifi_pass)) {
        doc["wifi-pass"] = _wifi_pass;
    }
    if (_mqtt_server != String(_badge_default_mqtt_server)) {
        doc["mqtt_server"] = _mqtt_server;
    }
    if (_mqtt_port != _badge_default_mqtt_port) {
        doc["mqtt_port"] = _mqtt_port;
    }
    if (_mqtt_user != String(_badge_default_mqtt_user)) {
        doc["mqtt_user"] = _mqtt_user;
    }
    if (_mqtt_pass != String(_badge_default_mqtt_pass)) {
        doc["mqtt_password"] = _mqtt_pass;
    }

    // Check if the JSON document is empty
    if (doc.isNull() || doc.size() == 0) {
        #ifdef NETCONFIGDEBUG
        Serial.println("No changes to save. Skipping file save.");
        #endif

        // Delete the file if it exists
        if (LittleFS.exists(_networkconfigfile)) {
            if (LittleFS.remove(_networkconfigfile)) {
                #ifdef NETCONFIGDEBUG
                Serial.println("Existing file deleted successfully.");
                #endif
            } else {
                #ifdef NETCONFIGDEBUG
                Serial.println("Failed to delete existing file.");
                #endif
            }
        }

        return true; // No need to save if there are no changes
    }

    return CryptoUtils::saveEncryptedFile(_networkconfigfile, doc, 0xB7C4E9A2, true);
}

void NetworkConfig::printNetwork() {
    Serial.println("Network Configuration:");
    Serial.print("WiFi SSID: ");
    Serial.println(_wifi_ssid);
    Serial.print("WiFi Password: ");
    Serial.println(_wifi_pass);
    Serial.print("MQTT Server: ");
    Serial.println(_mqtt_server);
    Serial.print("MQTT Port: ");
    Serial.println(_mqtt_port);
    Serial.print("MQTT User: ");
    Serial.println(_mqtt_user);
    Serial.print("MQTT Password: ");
    Serial.println(_mqtt_pass);
}