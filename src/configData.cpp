#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <base64.h>
#include "badge.h"
#include "ccserial.h"
#include "configData.h"
#include "uuidGenerator.h"
#include "registration.h"

/*
    configData.cpp -- part of the cackalackabadgy2023 project.
    Implementation of config functions using LittleFS
*/

/* *** SaveCFG *** */
namespace CFG {

    const char *supersecretfile = "/supersecret.json";

    // Saves the file
    void saveSecret( void ) {
        // Open file for writing
        File file = LittleFS.open(supersecretfile, "w");
        if (!file) {
            Serial.println(F("Failed to create secret file"));
            return;
        }

        char c_code[55];
        byte xorKey = 0x69;
        String code = UUIDG::getUUID() + "&" + WiFi.macAddress();
        strlcpy(c_code, code.c_str(), 55);
        for(uint8_t i = 0; i < sizeof(c_code) - 1; i++) {
            c_code[i] = c_code[i] ^ xorKey;
        }
        String toEncode = String(c_code);
        String encoded = base64::encode(toEncode);
        String url = "https://badge.cackalacky.ninja/capturetheflag?code=" + encoded;

        JsonDocument doc;

        // Set the values in the document
        doc["capturetheflag"] = url.c_str();

        // Serialize JSON to file
        if (serializeJson(doc, file) == 0) {
            Serial.println(F("Failed to write to secret file"));
        }

        // Close the file
        file.close();

    }

    String getSecretFile( void ) {
        // Open file for reading
        File file = LittleFS.open(supersecretfile, "r");

        JsonDocument doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, file);

        // Close the file (Curiously, File's destructor doesn't close the file)
        file.close();

        if (error) {
            Serial.print(F("Failed deserializing the json in progress file:"));
            Serial.println(error.f_str());
            String output = "";
            return output;
        }

        String body = doc.as<String>();
        return body;

        }

    // Prints the content of a file to the Serial
    void printFiles( void )
    {
#ifdef BADGEDEBUG
            Serial.println(F("printFiles()."));
#endif // BADGEDEBUG
        Dir dir = LittleFS.openDir("/");
        while (dir.next()) {
            Serial.print("/");
            Serial.print(dir.fileName());
            Serial.print(": ");
            if(dir.fileSize()) {
                File f = dir.openFile("r");
                Serial.println(f.size());

                // Extract each characters by one by one
                while (f.available()) {
                    Serial.print((char)f.read());
                }
                Serial.println();
                Serial.println();

                // Close the file
                f.close();
            }
        }
    }


    void printVERSION(void) {
        Serial.print(F("\nBadge Version: "));
        Serial.println(badge_version_str);
    }
}
