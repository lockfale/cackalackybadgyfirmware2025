// Standard Library Includes
#include <Arduino.h>
#include <ESP8266WiFi.h>

// Project-Specific Includes
#include "badge_status.h"
#include "badge.h"
#include "badge_display.h"
#include "badgeMenu.h"
#include "ccserial.h"
#include "neocontroller.h"
#include "NetworkConfig.h"
#include "BadgeWiFi.h"
#include "BadgeMQTT.h"
#include "registration.h"

//#define STATUS_DEBUG

namespace BADGE_STATUS {

    #define CHECK_TIME_INTERVAL 100

    // Build Strings for status display
    String versionStr;
    String regStr;
    String connectedStr;
    String ssidStr;
    #ifdef STATUS_DEBUG
    String ssidPwStr;
    String mqttStr;
    #endif // STATUS_DEBUG

    typedef enum {
        BADGE_STATUS_DISABLED = 0,
        BADGE_STATUS_RUNNING = 1
    } BADGE_STATUS_STATE_t;
    static BADGE_STATUS_STATE_t badge_status_state;

    static uint32_t previousRegMillis = 0;

    void showWaitingForCode( void ) {
        // The only status we care about updating is networking related
        if (BadgeWiFi::getInstance().isConnected()) {
            /*
            if (BadgeMQTT::getInstance().isConnected()) {
                connectedStr = "Conn:    Wifi+MQTT";
            } else {
                connectedStr = "Conn:    Wifi Only";
            }
            */
            connectedStr = BadgeMQTT::getInstance().isConnected() ? "Conn:    Wifi+MQTT" : "Conn:    Wifi Only";
        } else {
            connectedStr = "Conn:    Wifi Disconn";
        }

        BDISPLAY::display.setTextSize(1);
        BDISPLAY::display.setTextColor(WHITE);
        BDISPLAY::clearAndDisplayBatteryPercentage();
        BDISPLAY::display.setCursor(0, 0);
        BDISPLAY::display.println();
        BDISPLAY::display.println(versionStr);
        BDISPLAY::display.println(regStr);
        BDISPLAY::display.println(connectedStr);
        BDISPLAY::display.println(ssidStr);
        #ifdef STATUS_DEBUG
        BDISPLAY::display.println(ssidPwStr);
        BDISPLAY::display.println(mqttStr);
        #endif // STATUS_DEBUG
        BDISPLAY::display.display();
    }

    bool badgeStatusCheckTime( uint16_t duration ) {
        uint32_t curMillis = millis();
        if( curMillis - previousRegMillis >= duration) {
            previousRegMillis = curMillis;
            return true;
        } else {
            return false;
        }
    }

    void doStart( void ) {
        versionStr = "SW Vers: " + String(badge_version_str);
        regStr = Registration::isRegistered() ? "Reg'd:   Y" : "Reg'd:   N";
        ssidStr = "SSID:    " + String(NetworkConfig::getInstance().getWifiSSID());
        #ifdef STATUS_DEBUG
        ssidPwStr = "SSID PW: " + String(NetworkConfig::getInstance().getWifiPassword());
        mqttStr = "MQTT: " + String(NetworkConfig::getInstance().getMQTTServer()) + ":" + String(NetworkConfig::getInstance().getMQTTPort());
        #endif // STATUS_DEBUG

        // disable menu before starting
        BMenu::disableMenu();

        NeoController::getInstance().changeRed();
        badge_status_state = BADGE_STATUS_RUNNING;
        BDISPLAY::clearAndDisplayBatteryPercentage();
        BDISPLAY::display.display();
    }

    void doStop( void ) {
        badge_status_state = BADGE_STATUS_DISABLED;

        BDISPLAY::clearAndDisplayBatteryPercentage();
        BDISPLAY::display.display();

        // enable menu since we are done
        BMenu::enableMenu();
    }

    bool isRunning( void ) {
        if(badge_status_state == BADGE_STATUS_DISABLED){
            return false;
        } else {
            return true;
        }
    }

    void update( void ) {
        if (isRunning()) {
            if (badgeStatusCheckTime(CHECK_TIME_INTERVAL)) {
                showWaitingForCode();
            }
        }
    }

    void taskCallback() {
        update();
    }
}