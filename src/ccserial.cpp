#include <Arduino.h>
#include <ArduinoJson.h>
#include "ccserial.h"
#include "badge.h"
#include "configData.h"
#include "ProgressConfig.h"
#include "obfuscate.h"
#include "badge_display.h"
#include "badgeEmoji.h"
#include "neocontroller.h"
#include "NetworkConfig.h"
#include "NetworkTaskManager.h"
#include "CyberPartner.h"
#include "CPStore.h"
#include "registration.h"
#include "badgeOneWireMenu.h"
#include "cyberPartnerGame.h"

namespace ccSerial {
    const uint8_t numChars = 64;
    char receivedChars[numChars];
    char tempChars[numChars];

    char command[numChars] = {0};
    int integerFromPC = 0;
    float floatFromPC = 0.0;

    static bool badgeAdmin = false;

    boolean newData = false;

    void recvWithNewline() {
        static byte ndx = 0;

        while (Serial.available() > 0 && !newData) {
            char rc = Serial.read();

            if (rc == '\b') {  // Handle Backspace
                if (ndx > 0) {  // Ensure we don't go negative
                    ndx--;
                    receivedChars[ndx] = '\0';  // Remove last character
                    Serial.print("\b \b");  // Erase character visually
                }
                continue;
            }

            // Validate character (Allow A-Z, a-z, 0-9, space, and newline)
            if (!isalnum(rc) && rc != ' ' && rc != '.' && rc != '!' && rc != ':' && rc != '/' && rc != '-' && rc != '\n') {
                continue;
            }

            if (rc != '\n') {
                if (ndx < numChars - 1) { // Prevent overflow
                    receivedChars[ndx++] = rc;
                }
                Serial.write(rc);
            } else {
                receivedChars[ndx] = '\0';
                ndx = 0;
                newData = true;
                Serial.println();
            }
        }
    }



    void showStatus() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        Serial.println(F("What the hell is going on with this badge....."));
        Serial.println(F("------"));
        if (progconfig.isHelloWorldUnlocked()) {
            Serial.println(F("[+] HelloWorld: unlocked."));
        }
        if (progconfig.isSuUnlocked()) {
            Serial.println(F("[+] SU: unlocked."));
        }
        if (progconfig.isGotDroppedUnlocked()) {
            Serial.println(F("[+] Dropped: your CyberPartner egg :c"));
        }
        if (progconfig.isGotStarvedUnlocked()) {
            Serial.println(F("[+] Starved: your CyberPartner :c"));
        }
        if (progconfig.isGotThirstyUnlocked()) {
            Serial.println(F("[+] Thirsted (to death): your CyberPartner :c"));
        }
        if (progconfig.isGotDepressedUnlocked()) {
            Serial.println(F("[+] Depressed: your CyberPartner lost the will to play."));
        }
        if (progconfig.isGotOldUnlocked()) {
            Serial.println(F("[+] Old: your CyberPartner aged out of the program."));
        }
        if (progconfig.isGotHeavyUnlocked()) {
            Serial.println(F("[+] Heavy: your Cyberpartner was in a serious caloric surplus."));
        }
        if (progconfig.isGotSkinnyUnlocked()) {
            Serial.println(F("[+] Skinny: your CyberPartner was in a serious caloric deficit."));
        }
        if (progconfig.isGotChillyUnlocked()) {
            Serial.println(F("[+] Chilly: Winter came for your CyberPartner."));
        }
        if (progconfig.isGotSweatyUnlocked()) {
            Serial.println(F("[+] Sweaty: Your cyberPartner learned the difference in heat and humidity."));
        }
        if (progconfig.isBeatOgradyUnlocked()) {
            Serial.println(F("[+] Ogrady: beated."));
        }
        if (progconfig.isBeatRoulottoUnlocked()) {
            Serial.println(F("[+] Roulotto: Won an inside bet."));
        }
        if (progconfig.isD3adb33fUnlocked()) {
            Serial.println(F("[+] d3adb33f: Spoofed a token."));
        }
        if (progconfig.isTonyHawkUnlocked()) {
            Serial.println(F("[+] 900: Two and a half spins."));
        }
    }

    void unlockHelloWorldCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isHelloWorldUnlocked()) {
            progconfig.unlockHelloWorld();
            progconfig.saveProgressFile();
            Serial.println(F("[+] HelloWorld: unlocked."));
            Serial.println(F("Yay!! You can use a Serial port."));
        } else {
            Serial.println(F("You have already unlocked HelloWorld, but good job at trying it again!"));
        }
    }

    void lockHelloWorldCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isHelloWorldUnlocked()) {
            progconfig.lockHelloWorld();
            progconfig.saveProgressFile();
            Serial.println(F("HelloWorld challenge is locked"));
        } else {
            Serial.println(F("HelloWorld challenge is already locked."));
        }
    }

    void unlockGotDroppedCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotDroppedUnlocked()) {
            progconfig.unlockGotDropped();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotDropped: unlocked."));
            Serial.println(F("Why haven't we been asked to create hardware for physics classrooms?"));
        } else {
            Serial.println(F("You have already unlocked GotDropped, but good job at trying it again!"));
        }
    }

    void lockGotDroppedCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotDroppedUnlocked()) {
            progconfig.lockGotDropped();
            progconfig.saveProgressFile();
            Serial.println(F("GotDropped challenge is locked"));
        } else {
            Serial.println(F("GotDropped challenge is already locked."));
        }
    }

    void unlockGotStarvedCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotStarvedUnlocked()) {
            progconfig.unlockGotStarved();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotStarved: unlocked."));
            Serial.println(F("It's like a dog, you have to feed it!"));
        } else {
            Serial.println(F("You have already unlocked GotStarved, but good job at trying it again!"));
        }
    }

    void lockGotStarvedCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotStarvedUnlocked()) {
            progconfig.lockGotStarved();
            progconfig.saveProgressFile();
            Serial.println(F("GotStarved challenge is locked"));
        } else {
            Serial.println(F("GotStarved challenge is already locked."));
        }
    }

    void unlockGotThirstyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotThirstyUnlocked()) {
            progconfig.unlockGotThirsty();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotThirsty: unlocked."));
            Serial.println(F("It's not a cat, you have to give it water!"));
        } else {
            Serial.println(F("You have already unlocked GotThirsty, but good job at trying it again!"));
        }
    }

    void lockGotThirstyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotThirstyUnlocked()) {
            progconfig.lockGotThirsty();
            progconfig.saveProgressFile();
            Serial.println(F("GotThirsty challenge is locked"));
        } else {
            Serial.println(F("GotThirsty challenge is already locked."));
        }
    }

    void unlockGotDepressedCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotDepressedUnlocked()) {
            progconfig.unlockGotDepressed();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotDepressed: unlocked."));
            Serial.println(F("Do you even care about your CyberPartner?"));
        } else {
            Serial.println(F("You have already unlocked GotDepressed, but good job at trying it again!"));
        }
    }

    void lockGotDepressedCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotDepressedUnlocked()) {
            progconfig.lockHelloWorld();
            progconfig.saveProgressFile();
            Serial.println(F("GotDepressed challenge is locked"));
        } else {
            Serial.println(F("GotDepressed challenge is already locked."));
        }
    }

    void unlockGotOldCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotOldUnlocked()) {
            progconfig.unlockGotOld();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotOld: unlocked."));
            Serial.println(F("Your CyberPartner lived a long life."));
        } else {
            Serial.println(F("You have already unlocked GotOld, but good job at trying it again!"));
        }
    }

    void lockGotOldCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotOldUnlocked()) {
            progconfig.lockGotOld();
            progconfig.saveProgressFile();
            Serial.println(F("GotOld challenge is locked"));
        } else {
            Serial.println(F("GotOld challenge is already locked."));
        }
    }

    void unlockGotHeavyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotHeavyUnlocked()) {
            progconfig.unlockGotHeavy();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotHeavy: unlocked."));
            Serial.println(F("All this food... is making me fat..."));
        } else {
            Serial.println(F("You have already unlocked GotHeavy, but good job at trying it again!"));
        }
    }

    void lockGotHeavyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotHeavyUnlocked()) {
            progconfig.lockGotHeavy();
            progconfig.saveProgressFile();
            Serial.println(F("GotHeavy challenge is locked"));
        } else {
            Serial.println(F("GotHeavy challenge is already locked."));
        }
    }

    void unlockGotSkinnyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotSkinnyUnlocked()) {
            progconfig.unlockGotSkinny();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotSkinny: unlocked."));
            Serial.println(F("Pick'em up, Set'em down!"));
        } else {
            Serial.println(F("You have already unlocked GotSkinny, but good job at trying it again!"));
        }
    }

    void lockGotSkinnyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotSkinnyUnlocked()) {
            progconfig.lockGotSkinny();
            progconfig.saveProgressFile();
            Serial.println(F("GotSkinny challenge is locked"));
        } else {
            Serial.println(F("GotSkinny challenge is already locked."));
        }
    }

    void unlockGotChillyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotChillyUnlocked()) {
            progconfig.unlockGotChilly();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotChilly: unlocked."));
            Serial.println(F("The badge is not a coaster... but you already knew that"));
        } else {
            Serial.println(F("You have already unlocked GotChilly, but good job at trying it again!"));
        }
    }

    void lockGotChillyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotChillyUnlocked()) {
            progconfig.lockGotChilly();
            progconfig.saveProgressFile();
            Serial.println(F("GotChilly challenge is locked"));
        } else {
            Serial.println(F("GotChilly challenge is already locked."));
        }
    }

    void unlockGotSweatyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isGotSweatyUnlocked()) {
            progconfig.unlockGotSweaty();
            progconfig.saveProgressFile();
            Serial.println(F("[+] GotSweaty: unlocked."));
            Serial.println(F("DO NOT microwave the badge... again..."));
        } else {
            Serial.println(F("You have already unlocked GotSweaty, but good job at trying it again!"));
        }
    }

    void lockGotSweatyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isGotSweatyUnlocked()) {
            progconfig.lockGotSweaty();
            progconfig.saveProgressFile();
            Serial.println(F("GotSweaty challenge is locked"));
        } else {
            Serial.println(F("GotSweaty challenge is already locked."));
        }
    }

    void unlockBeatOgradyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isBeatOgradyUnlocked()) {
            progconfig.unlockBeatOgrady();
            progconfig.saveProgressFile();
            Serial.println(F("[+] BeatOgrady: unlocked."));
            Serial.println(F("You beat Ogrady! Did you have to build a memory palace?"));
        } else {
            Serial.println(F("You have already unlocked BeatOgrady, but good job at trying it again!"));
        }
    }

    void lockBeatOgradyCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isBeatOgradyUnlocked()) {
            progconfig.lockBeatOgrady();
            progconfig.saveProgressFile();
            Serial.println(F("BeatOgrady challenge is locked"));
        } else {
            Serial.println(F("BeatOgrady challenge is already locked."));
        }
    }

    void unlockBeatRoulottoCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isBeatRoulottoUnlocked()) {
            progconfig.unlockBeatRoulotto();
            progconfig.saveProgressFile();
            Serial.println(F("[+] BeatRoulotto: unlocked."));
        } else {
            Serial.println(F("You have already unlocked BeatRoulotto, but good job at trying it again!"));
        }
    }

    void lockBeatRoulottoCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isBeatRoulottoUnlocked()) {
            progconfig.lockBeatRoulotto();
            progconfig.saveProgressFile();
            Serial.println(F("BeatRoulotto challenge is locked"));
        } else {
            Serial.println(F("BeatRoulotto challenge is already locked."));
        }
    }

    void unlockD3adb33fCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isD3adb33fUnlocked()) {
            progconfig.unlockD3adb33f();
            progconfig.saveProgressFile();
            if (ProgressConfig::getInstance().isD3adb33fUnlocked()) {
                BDISPLAY::display.invertDisplay(true); // Invert colors
            } else {
                BDISPLAY::display.invertDisplay(false); // Normal colors
            }
            Serial.println(F("[+] d3adb33f: unlocked."));
            Serial.println(F("You Spoofed OneWire Did you use a flipper?"));
        } else {
            Serial.println(F("You have already unlocked d3adb33f, but good job at trying it again!"));
        }
    }

    void lockD3adb33fCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isD3adb33fUnlocked()) {
            progconfig.lockD3adb33f();
            progconfig.saveProgressFile();
            if (ProgressConfig::getInstance().isD3adb33fUnlocked()) {
                BDISPLAY::display.invertDisplay(true); // Invert colors
            } else {
                BDISPLAY::display.invertDisplay(false); // Normal colors
            }
            Serial.println(F("d3adb33f challenge is locked"));
        } else {
            Serial.println(F("d3adb33f challenge is already locked."));
        }
    }

    void unlockTonyHawkCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (!progconfig.isTonyHawkUnlocked()) {
            progconfig.unlockTonyHawk();
            progconfig.saveProgressFile();
            Serial.println(F("[+] TonyHawk: unlocked."));
            Serial.println(F("You must be a Tony Hawk Pro Skater to do a 900!"));
        } else {
            Serial.println(F("You have already unlocked TonyHawk, but good job at trying it again!"));
        }
    }

    void lockTonyHawkCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        if (progconfig.isTonyHawkUnlocked()) {
            progconfig.lockTonyHawk();
            progconfig.saveProgressFile();
            Serial.println(F("TonyHawk challenge is locked"));
        } else {
            Serial.println(F("TonyHawk challenge is already locked."));
        }
    }

    void delprogCommand() {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        progconfig.resetProgress();
        progconfig.saveProgressFile();
        progconfig.loadProgressFile();
        Serial.println(F("Deleting progress."));
    }

    void listfilesCommand() {
        CFG::printFiles();
    }

    void resetNetworkCommand() {
        NetworkConfig::getInstance().resetNetwork();
        NetworkConfig::getInstance().saveNetworkFile();
        Serial.println(F("Network configuration reset."));
    }

    void printNetworkCommand() {
        NetworkConfig::getInstance().printNetwork();
    }

    void setWifiSSIDCommand(const String& ssid) {
        NetworkConfig::getInstance().setWifiSSID(ssid);
        Serial.print(F("WiFi SSID set to: "));
        Serial.println(ssid.c_str());
    }

    void setWifiPasswordCommand(const String& password) {
        NetworkConfig::getInstance().setWifiPassword(password);
        Serial.println(F("WiFi password set."));
    }

    void setMQTTServerCommand(const String& server) {
        NetworkConfig::getInstance().setMQTTServer(server);
        Serial.print(F("MQTT server set to: "));
        Serial.println(server.c_str());
    }

    void setMQTTPortCommand(const String& port) {
        NetworkConfig::getInstance().setMQTTPort(strtol(port.c_str(), nullptr, 10));
        Serial.print(F("MQTT port set to: "));
        Serial.println(port.c_str());
    }

    void setMQTTUserCommand(const String& user) {
        NetworkConfig::getInstance().setMQTTUser(user);
        Serial.println(F("MQTT user set."));
    }

    void setMQTTPasswordCommand(const String& password) {
        NetworkConfig::getInstance().setMQTTPassword(password);
        Serial.println(F("MQTT password set."));
    }

    void setCPStoreMoneyCommand(const String& money) {
        uint32_t amount = strtoul(money.c_str(), nullptr, 10);
        if (CPStore::getInstance().setMoney(amount)) {
            Serial.print(F("CPStore money set to: "));
            Serial.println(amount);
        } else {
            Serial.println(F("Failed to set CPStore money."));
        }
    }

    void buySushiCommand() {
        if (CPStore::getInstance().purchaseMeal(CPStoreMeals::MEAL_SUSHI)) {
            Serial.println("You bought sushi!");
        } else {
            Serial.println("Not enough money to purchase sushi.");
        }
    }

    void eatSushiCommand() {
        CPStore& store = CPStore::getInstance();

        if (store.getMealCount(CPStoreMeals::MEAL_SUSHI) > 0) {
            store.consumeMeal(CPStoreMeals::MEAL_SUSHI);
            Serial.println("You ate sushi!");
        } else {
            Serial.println("No sushi available to eat.");
        }
    }

    void increaseHungerCommand() {
        CyberPartner& partner = CyberPartner::getInstance();

        partner.setHunger(95);;
        Serial.println(F("Hunger increased."));
        JsonDocument hungerDoc;
        hungerDoc["command"] = "inchungrr";
        hungerDoc["result"] = "Some idiot inchungrrd oh my God";
        partner.queueGeneralUpdate("cp/game/play", hungerDoc);
    }

    void setHungerCommand(const String& hunger) {
        int hungerValue = strtol(hunger.c_str(), nullptr, 10);
        if (hungerValue >= 0 && hungerValue <= 100) {
            CyberPartner::getInstance().setHunger(hungerValue);
            Serial.print(F("Hunger set to: "));
            Serial.println(hungerValue);
        } else {
            Serial.println(F("Invalid hunger value. Must be between 0 and 100."));
        }
    }

    void suCommand(const String& password) {
        ProgressConfig& progconfig = ProgressConfig::getInstance();

        JsonDocument suDoc;
        suDoc["command"] = "su";
        suDoc["password"] = password;
        if (password == AY_OBFUSCATE("Ecruiaiergo")) {
            badgeAdmin = true;
            suDoc["admin"] = true;

            Serial.println(F("Prest-O Change-O! You are now an admin."));
            if (!progconfig.isSuUnlocked()) {
                Serial.printf("Achievement Unlocked!\n");
                progconfig.unlockSu();
                progconfig.saveProgressFile();
            }
        } else {
            suDoc["admin"] = false;
            Serial.println(F("Incorrect password."));
        }

        CyberPartner::getInstance().queueGeneralUpdate("cp/achmnt/su", suDoc);
    }

    void rotateScreen() {
        Serial.println(F("Rotating Screen..."));
        BDISPLAY::rotateDisplay();
        NeoController::getInstance().changeStrobe();
        if (BDISPLAY::rotatedDegrees >= 900) {
            if (!ProgressConfig::getInstance().isTonyHawkUnlocked()) {
                ProgressConfig::getInstance().unlockTonyHawk();
                ProgressConfig::getInstance().saveProgressFile();
                Serial.println(F("You must be a Tony Hawk Pro Skater to do a 900!"));
                Serial.println(F("Achievement Unlocked!"));
            }
        }
    }

    void nineonesevenCommand() {
        Serial.println(F("This is the combination for my luggage:  12345"));
        // Add your specific functionality here
    }

    void ntpCommand() {
        time_t now = time(nullptr);
        if (now > 946684800) {
            Serial.println("NTP time synced!");
            Serial.print("Current time: ");
            Serial.println(ctime(&now)); // Print the synced time
        } else {
            Serial.println("NTP time not yet synced or invalid.");
        }

    }

    void toggleRock() {
        CyberPartner& partner = CyberPartner::getInstance();

        if (partner.attributes.rock) {
            partner.attributes.rock = !partner.attributes.rock;
            Serial.println("You're no longer a rock :c");
        } else {
            partner.attributes.rock = !partner.attributes.rock;
            Serial.println("You're a rock! Heck Yeah!");
        }
    }

    void restartCommand() {
        Serial.println(F("Restarting badge..."));
        delay(1000);
        ESP.restart();
    }

    // Need to add a forward declaration for the function showHalp()
    void showHalp();

    static const struct Command {
        const char *name;
        bool requiresAdmin;
        void (*action)();
        void (*actionWithOption)(const String& option);
        const char *helpMessage;
    } commands[] = {
        /***********************
         *  Unauthed Commands  *
         ***********************/
        {"HALP",          false,  showHalp, nullptr, "Displays this halp message"},
        {"STATUS",        false,  showStatus, nullptr, "Displays the current badge status"},
        {"INCHUNGRR",     false,  increaseHungerCommand, nullptr, "Increases hunger"},
        {"PARTNERSTATS",  false,  []() { CyberPartner::getInstance().printAttributes(); }, nullptr, "Displays CyberPartner stats"},
        {"STORESTATS",    false,  []() { CPStore::getInstance().printStoreStats(); }, nullptr, "Displays Store stats"},
        {"HELLOWORLD",    false,  unlockHelloWorldCommand, nullptr, nullptr},
        {"NTP",           false,  ntpCommand, nullptr, nullptr},
        {"917",           false,  nineonesevenCommand, nullptr, "It's a mystery."},
        {"ROTATE",        false,  rotateScreen, nullptr, nullptr},

        // Don't give help for neopixel commands. Even tho they don't need admin, let them figure it out
        {"PINK",          false,  []() { NeoController::getInstance().changePink(); }, nullptr, nullptr},
        {"RED",           false,  []() { NeoController::getInstance().changeRed(); }, nullptr, nullptr},
        {"GREEN",         false,  []() { NeoController::getInstance().changeGreen(); }, nullptr, nullptr},
        {"BLUE",          false,  []() { NeoController::getInstance().changeBlue(); }, nullptr, nullptr},
        {"YELLOW",        false,  []() { NeoController::getInstance().changeYellow(); }, nullptr, nullptr},
        {"WHITE",         false,  []() { NeoController::getInstance().changeWhite(); }, nullptr, nullptr},
        {"CHASE",         false,  []() { NeoController::getInstance().changeChase(); }, nullptr, nullptr},
        {"CONFETTI",      false,  []() { NeoController::getInstance().changeConfetti(); }, nullptr, nullptr},
        {"OGRADY",        false,  []() { NeoController::getInstance().changeOgrady(); }, nullptr, nullptr},
        {"POPO",          false,  []() { NeoController::getInstance().changePopo(); }, nullptr, nullptr},
        {"RAINBOW",       false,  []() { NeoController::getInstance().changeRainbow(); }, nullptr, nullptr},
        {"STROBE",        false,  []() { NeoController::getInstance().changeStrobe(); }, nullptr, nullptr},
        {"SURGE",         false,  []() { NeoController::getInstance().changeSurge(); }, nullptr, nullptr},
        {"SC",            false,  []() { NeoController::getInstance().changeSmoothChase(); }, nullptr, nullptr},
        // End Neopixel commands

        //{"FCE",         false,  []() { EMOJI::drawNoFretCitizenEmoji(); }, nullptr, nullptr},
        //{"TE",          false,  []() { EMOJI::drawTacoEmoji(); }, nullptr, nullptr},
        //{"WE",          false,  []() { EMOJI::drawWaveEmoji(); }, nullptr, nullptr},
        {"SU",            false,  nullptr, suCommand, "Grants admin privileges"},
        {"RESETNETWORK",  false,  resetNetworkCommand, nullptr, "Resets network configuration"},
        {"RESTART",       false,  restartCommand, nullptr, "Restarts the badge"},

        /***********************
         *  Authed Commands    *
         ***********************/
        {AY_OBFUSCATE("DELPROG"),       true,  delprogCommand, nullptr, "Deletes progress file"},
        {"PARTNERDEBUG",                true,  []() { CyberPartner::getInstance().setDebug(true); }, nullptr, nullptr},

        {AY_OBFUSCATE("LHW"),           true,  lockHelloWorldCommand, nullptr, "Locks HelloWorld challenge"},
        {AY_OBFUSCATE("LGDROP"),        true,  lockGotDroppedCommand, nullptr, "Locks GotDropped challenge"},
        {AY_OBFUSCATE("LGSTARVED"),     true,  lockGotStarvedCommand, nullptr, "Locks GotStarved challenge"},
        {AY_OBFUSCATE("LGTHIRSTY"),     true,  lockGotThirstyCommand, nullptr, "Locks GotThirsty challenge"},
        {AY_OBFUSCATE("LGDEPRESSED"),   true,  lockGotDepressedCommand, nullptr, "Locks GotDepressed challenge"},
        {AY_OBFUSCATE("LGOLD"),         true,  lockGotOldCommand, nullptr, "Locks GotOld challenge"},
        {AY_OBFUSCATE("LGHEAVY"),       true,  lockGotHeavyCommand, nullptr, "Locks GotHeavy challenge"},
        {AY_OBFUSCATE("LGSKINNY"),      true,  lockGotSkinnyCommand, nullptr, "Locks GotSkinny challenge"},
        {AY_OBFUSCATE("LGCHILLY"),      true,  lockGotChillyCommand, nullptr, "Locks GotChilly challenge"},
        {AY_OBFUSCATE("LGSWEATY"),      true,  lockGotSweatyCommand, nullptr, "Locks GotSweaty challenge"},
        {AY_OBFUSCATE("LBO"),           true,  lockBeatOgradyCommand, nullptr, "Locks BeatOgrady challenge"},
        {AY_OBFUSCATE("LBR"),           true,  lockBeatRoulottoCommand, nullptr, "Locks BeatRoulotto challenge"},
        {AY_OBFUSCATE("LSOW"),          true,  lockD3adb33fCommand, nullptr, "Locks the Spoof OneWire challenge"},
        {AY_OBFUSCATE("LTH"),          true,  lockTonyHawkCommand, nullptr, "Locks the 900 challenge"},

        {AY_OBFUSCATE("UGDROP"),        true,  unlockGotDroppedCommand, nullptr, "Unlocks GotDropped challenge"},
        {AY_OBFUSCATE("UGSTARVED"),     true,  unlockGotStarvedCommand, nullptr, "Unlocks GotStarved challenge"},
        {AY_OBFUSCATE("UGTHIRSTY"),     true,  unlockGotThirstyCommand, nullptr, "Unlocks GotThirsty challenge"},
        {AY_OBFUSCATE("UGDEPRESSED"),   true,  unlockGotDepressedCommand, nullptr, "Unlocks GotDepressed challenge"},
        {AY_OBFUSCATE("UGOLD"),         true,  unlockGotOldCommand, nullptr, "Unlocks GotOld challenge"},
        {AY_OBFUSCATE("UGHEAVY"),       true,  unlockGotHeavyCommand, nullptr, "Unlocks GotHeavy challenge"},
        {AY_OBFUSCATE("UGSKINNY"),      true,  unlockGotSkinnyCommand, nullptr, "Unlocks GotSkinny challenge"},
        {AY_OBFUSCATE("UGCHILLY"),      true,  unlockGotChillyCommand, nullptr, "Unlocks GotChilly challenge"},
        {AY_OBFUSCATE("UGSWEATY"),      true,  unlockGotSweatyCommand, nullptr, "Unlocks GotSweaty challenge"},
        {AY_OBFUSCATE("UBO"),           true,  unlockBeatOgradyCommand, nullptr, "Unlocks BeatOgrady challenge"},
        {AY_OBFUSCATE("UBR"),           true,  unlockBeatRoulottoCommand, nullptr, "Unlocks BeatRoulotto challenge"},
        {AY_OBFUSCATE("UDB"),          true,  unlockD3adb33fCommand, nullptr, "Unlocks the Spoof OneWire challenge"},
        {AY_OBFUSCATE("UTH"),          true,  unlockTonyHawkCommand, nullptr, "Unlocks the 900 challenge"},

        {AY_OBFUSCATE("LS"),            true,  listfilesCommand, nullptr, "Lists files"},
        {AY_OBFUSCATE("SETMONEY"),      true,  nullptr, setCPStoreMoneyCommand, "Sets CPStore money"},
        {AY_OBFUSCATE("BUYSUSHI"),      true,  buySushiCommand, nullptr, "Buys sushi"},
        {AY_OBFUSCATE("EATSUSHI"),      true,  eatSushiCommand, nullptr, "Eats sushi"},
        {AY_OBFUSCATE("SETHUNGER"),     true,  nullptr, setHungerCommand, "Sets hunger"},
#ifndef ENABLE_KIOSK_MODE
        {AY_OBFUSCATE("SETWIFISSID"),   true,  nullptr, setWifiSSIDCommand, "Sets WiFi SSID"},
        {AY_OBFUSCATE("SETWIFIPASS"),   true,  nullptr, setWifiPasswordCommand, "Sets WiFi password"},
        {AY_OBFUSCATE("SETMQTTSERVER"), true,  nullptr, setMQTTServerCommand, "Sets MQTT server"},
        {AY_OBFUSCATE("SETMQTTPORT"),   true,  nullptr, setMQTTPortCommand, "Sets MQTT port"},
        {AY_OBFUSCATE("SETMQTTUSER"),   true,  nullptr, setMQTTUserCommand, "Sets MQTT user"},
        {AY_OBFUSCATE("SETMQTTPASS"),   true,  nullptr, setMQTTPasswordCommand, "Sets MQTT password"},
        {AY_OBFUSCATE("SAVENETWORK"),   true,  []() { NetworkConfig::getInstance().saveNetworkFile(); }, nullptr, "Save network configuration"},
        {AY_OBFUSCATE("UNREGISTER"),    true,  []() { Registration::getInstance().unRegisterBadge(); }, nullptr, "Unregister badge"},
        {AY_OBFUSCATE("SETIB"),         true,  []() { BadgeOneWireMenu::kioskSetupIButton(); }, nullptr, nullptr},
        {AY_OBFUSCATE("PRINTNETWORK"),  true,  printNetworkCommand, nullptr, "Print network configuration"},
        {AY_OBFUSCATE("TOGGLEROCK"),  true,  toggleRock, nullptr, "Live your dreams, transform"},
#endif
#ifdef ENABLE_KIOSK_MODE
        {AY_OBFUSCATE("SETWIFISSID"),   false,  nullptr, setWifiSSIDCommand, "Sets WiFi SSID"},
        {AY_OBFUSCATE("SETWIFIPASS"),   false,  nullptr, setWifiPasswordCommand, "Sets WiFi password"},
        {AY_OBFUSCATE("SETMQTTSERVER"), false,  nullptr, setMQTTServerCommand, "Sets MQTT server"},
        {AY_OBFUSCATE("SETMQTTPORT"),   false,  nullptr, setMQTTPortCommand, "Sets MQTT port"},
        {AY_OBFUSCATE("SETMQTTUSER"),   false,  nullptr, setMQTTUserCommand, "Sets MQTT user"},
        {AY_OBFUSCATE("SETMQTTPASS"),   false,  nullptr, setMQTTPasswordCommand, "Sets MQTT password"},
        {AY_OBFUSCATE("SAVENETWORK"),   false,  []() { NetworkConfig::getInstance().saveNetworkFile(); }, nullptr, "Save network configuration"},
        {AY_OBFUSCATE("UNREGISTER"),    false,  []() { Registration::getInstance().unRegisterBadge(); }, nullptr, "Unregister badge"},
        {AY_OBFUSCATE("SETIB"),         false,  []() { BadgeOneWireMenu::kioskSetupIButton(); }, nullptr, nullptr},
        {AY_OBFUSCATE("PRINTNETWORK"),  false,  printNetworkCommand, nullptr, "Print network configuration"},
#endif
    };

    static const size_t numCommands = sizeof(commands) / sizeof(commands[0]);

    void showHalp() {
        Serial.println(F("HALP"));
        Serial.println(F("----"));
        Serial.println(F("The following commands are available:"));
        for (size_t i = 0; i < numCommands; ++i) {
            // Skip commands that require admin if the user is not an admin
            if (commands[i].requiresAdmin && !badgeAdmin) {
                continue;
            }

            // Print the halp message if it exists
            if (commands[i].helpMessage) {
                Serial.print(F("  "));
                Serial.print(commands[i].name);
                Serial.print(F(" - "));
                Serial.print(commands[i].helpMessage);
                if (commands[i].requiresAdmin) {
                    Serial.println(F(" (Requires admin privileges)"));
                } else {
                    Serial.println();
                }
            }
        }
    }

    void parseData() {
        char *token = strtok(tempChars, " ");
        if (!token) return;

        strcpy(command, token);
        for (char *p = command; *p; ++p) *p = toupper(*p); // Convert to uppercase

        Serial.print(F("Command recieved: "));
        Serial.println(command);

        // Lookup and execute command
        for (size_t i = 0; i < numCommands; ++i) {
            if (strcmp(command, commands[i].name) == 0) {
                if (commands[i].requiresAdmin && !badgeAdmin) {
                    Serial.println(F("Condition not met. Command cannot be executed."));
                } else {
                    if (commands[i].action) {
                        commands[i].action();
                    } else if (commands[i].actionWithOption) {
                        // Get the option
                        token = strtok(nullptr, " ");
                        if (token) {
                            String option(token);
                            commands[i].actionWithOption(option);
                        } else {
                            Serial.println(F("Option missing for command."));
                        }
                    }
                }
                Serial.println(F("__________________________________"));
                return;
            }
        }

        Serial.println(F("I don't understand."));
        Serial.println(F("__________________________________"));
        if (CyberPartnerGame::getInstance().isActive()) {
            rotateScreen();
        }
    }

    void setup( void )
    {
        Serial.print(F("** "));
        Serial.print(greeting_str);
        Serial.print(F(" v"));
        Serial.print(badge_version_str);
        Serial.println(F(" **"));

        if (ProgressConfig::getInstance().isHelloWorldUnlocked()) {
            Serial.println(F("What is thy bidding?"));
        } else {
            Serial.println(F("Scream for halp if you need it."));
        }
    }

    void update( void )
    {
        recvWithNewline();
        if (newData == true) {
            if ( strlen(receivedChars) > 0 ) {
                strcpy(tempChars, receivedChars);
                parseData();
            } else {
                Serial.println(F("I don't understand."));
            }
            newData = false;
        }

    }

}