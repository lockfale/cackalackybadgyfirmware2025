#include <Wire.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "badge.h"
#include "ccserial.h"
#include "haptic.h"
#include "badge_haptic_tasks.h"
#include "game_timer.h"
#include "ogrady.h"
#include "badge_display.h"
#include "badgeMenu.h"
#include "ProgressConfig.h"
#include "neocontroller.h"
#include "CPStore.h"
#include "CyberPartner.h"

#define OGS_CHECK_TIME_INTERVAL 50

OGS& OGS::getInstance() {
    static OGS instance;
    return instance;
}

const String OGS::intToColor(ogsColor_t color) {
    switch (color) {
        case Y: return "Y";
        case R: return "R";
        case G: return "G";
        case B: return "B";
        default: return "?";
    }
}

// Return array contents as an int string
String OGS::getRoundArrayAsInts(int* arr, int size) {
    String result = "";
    for (int i = 0; i < size; i++) {
        result += String(arr[i]);
        if (i < size - 1) {
            result += ",";  // Separate values with commas
        }
    }
    return result;
}

// Return array contents as Color string
String OGS::getRoundArrayAsColors(int* arr, int size) {
    String result = "";
    for (int i = 0; i < size; i++) {
        ogsColor_t color = static_cast<ogsColor_t>(arr[i]);
        result += String(intToColor(color));
        if (i < size - 1) {
            result += ",";  // Separate values with commas
        }
    }
    return result;
}

// Doing most of the setup for each game round here
void OGS::populateRoundArrays() {
    // Free and/or allocate input array
    if (dynamicInputArray != nullptr) {
        delete[] dynamicInputArray;
    }
    dynamicInputArray = new int[roundArraySize];

    // Free and/or allocate pattern array
    if (dynamicRoundArray != nullptr) {
        delete[] dynamicRoundArray;
    }
    dynamicRoundArray = new int[roundArraySize];

    // Populate the array with random values between 1 and 4
    for (int i = 0; i < roundArraySize; i++) {
        int nextValue = random(1, 5);

        // Reroll 3rd consecutive value until not consecutive
        while (i > 1
            && nextValue == dynamicRoundArray[i - 1]
            && nextValue == dynamicRoundArray[i - 2]) {
                nextValue = random(1, 5);
        }
    dynamicRoundArray[i] = nextValue;
    dynamicInputArray[i] = 0;
    }

    #ifdef BADGEDEBUG
        Serial.println(F("(De?)Allocating Arrays via populateRoundArrays()"));
    #endif //BADGEDEBUG
}

bool OGS::isRunning()
{
    if(game_state == OGS_DISABLED){
        return false;
    } else {
        return true;
    }
}

void OGS::finishGame(bool result){
    ProgressConfig& progconfig = ProgressConfig::getInstance();

    if(result == true){
        game_result = GR_WIN;
        if (!progconfig.isBeatOgradyUnlocked()) {
            Serial.printf("Ogrady says you just beat the game, Achievement Unlocked!\n");
            progconfig.unlockBeatOgrady();
            progconfig.saveProgressFile();

        }
    } else {
        game_result = GR_LOST;
    }
    game_state = OGS_RESULT;
}

bool OGS::isRoundWinner(){
    #ifdef BADGEDEBUG
        Serial.println(F("Comparing Arrays via isRoundWinner()"));
        Serial.printf("\tRound arr: %s\n", getRoundArrayAsInts(dynamicRoundArray, roundArraySize).c_str());
        Serial.printf("\tInput arr: %s\n", getRoundArrayAsInts(dynamicInputArray, roundArraySize).c_str());
    #endif //BADGEDEBUG

    for (int i = 0; i < roundArraySize; i++){
        if (dynamicRoundArray[i] != dynamicInputArray[i]){
            return false;
        }
    }
    return true;
}

void OGS::runOgradySays()
{
    NeoController& neo = NeoController::getInstance();

    // First evaluate expired timers
    // TitleBanner timer expired, Set gamestate to instruction
    if(titleBannerTicker >= 75){
        BDISPLAY::clearAndDisplayBatteryPercentage();
        game_state = OGS_INSTRUCT;
    }
    // Instruction timer expired, Set gamestate to Pattern Show
    if(instructBannerTicker >= 75){
        BDISPLAY::clearAndDisplayBatteryPercentage();
        game_state = OGS_PATTERNSHOW;
    }
    // PatternShow timer expired, Set gamestate to get input
    if(patternBannerTicker >= roundArraySize * roundScalar){
        neo.turnAllOff();
        BDISPLAY::clearAndDisplayBatteryPercentage();
        game_state = OGS_GET_INPUT;
    }
    // Input timer expired, check win/loss && Set gamestate to PatternShow/Result
    if(inputBannerTicker >= inputBannerLimit){
        BDISPLAY::clearAndDisplayBatteryPercentage();
        inputBannerTicker = 0; //  Input done for now, reset before forcing next mode
        if(isRoundWinner()){
            if(roundWinCounter >= 7){  // Killscreen condition
                finishGame(true);
            } else { // Round win, increment counters and GOTO Pattern Banner
            roundWinCounter++;
            roundArraySize++;
            inputBannerLimit =  roundArraySize * roundScalar; // Scale round time limit
            game_state = OGS_PATTERNSHOW;
            #ifdef BADGEDEBUG
                Serial.printf("Round %d Won!", roundWinCounter);
                Serial.printf("\troundWinCounter: %d\troundArraySize: %d \troundInputCounter: %d\n",
                    roundWinCounter, roundArraySize, roundInputCounter);
            #endif //BADGEDEBUG
            }
        } else { // Round/Game Loss condition
            #ifdef BADGEDEBUG
                Serial.printf("Round %d lost :c", roundWinCounter + 1);
                Serial.printf("\troundWinCounter: %d\troundArraySize: %d \troundInputCounter: %d\n",
                    roundWinCounter, roundArraySize, roundInputCounter);
            #endif //BADGEDEBUG
            finishGame(false);
        }
    }
    // ResultBanner done, terminate
    if(resultBannerTicker >= 75){
        doStop();
    }

    // Title Banner
    if(game_state == OGS_TITLEBANNER){
        if (titleBannerTicker < 2) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.println();
            BDISPLAY::display.println(F(" O'Grady\n   Says"));
            BDISPLAY::display.display();
        }
    titleBannerTicker++;

    // Instruction Banner
    } else if(game_state == OGS_INSTRUCT){
    titleBannerTicker = 0;
        if (instructBannerTicker < 2) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.println();
            BDISPLAY::display.println(F("1) Watch the pattern\n"));
            BDISPLAY::display.println(F("2) Repeat via buttons\n"));
            BDISPLAY::display.println(F("3) New patterns on"));
            BDISPLAY::display.println(F("    next round!"));
            BDISPLAY::display.display();
            neo.changeWhite();
        }
        instructBannerTicker++;  // Tick current Banner (Instruction)

    //  Pattern Banner
    } else if(game_state == OGS_PATTERNSHOW){
        instructBannerTicker = 0;  // Reset previous game states timers/counter
        inputBannerTicker = 0;
        roundInputCounter = 0;
        // Only modify the Round Pattern at the beginning of a round
        if (patternBannerTicker == 0) {
            populateRoundArrays();
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.printf("Round %d\n", roundWinCounter + 1);
            #ifdef BADGEDEBUG
                BDISPLAY::display.setTextSize(1);
                BDISPLAY::display.printf("Current Pattern:\n  %s\n", getRoundArrayAsInts(dynamicRoundArray, roundArraySize).c_str());
            #endif //BADGEDEBUG
            BDISPLAY::display.display();
        }
        // For each index of the round array, tick roundScalaer number of times for pattern display
        if (patternBannerTicker % roundScalar == 0) {
            // Turn on the neo at index
            if (dynamicRoundArray[roundShowPatternCounter] == 1) {  // Yellow
                neo.changeOgradyTopLeft();
            } else if (dynamicRoundArray[roundShowPatternCounter] == 2) {  // Red
                neo.changeOgradyBottomLeft();
            } else if (dynamicRoundArray[roundShowPatternCounter] == 3) {  // Green
                neo.changeOgradyBottomRight();
            } else if (dynamicRoundArray[roundShowPatternCounter] == 4) {  // Blue
                neo.changeOgradyTopRight();
            }
            BadgeHaptic::getInstance().hapticOn();
        } else if (patternBannerTicker % roundScalar == roundScalar - 5) {
            // Turn the neo off for the last few ticks of each roundscalar
            neo.turnAllOff();
            BadgeHaptic::getInstance().hapticOff();
        } else if (patternBannerTicker % roundScalar == roundScalar - 1) {
            // bump the counter on the last tick for each roundScaler
            roundShowPatternCounter++;
        }
        patternBannerTicker++;
    //  Input Banner & Input gathering
    } else if(game_state == OGS_GET_INPUT){
        patternBannerTicker = 0;
        roundShowPatternCounter = 0;
        if (inputBannerTicker % 10 == 0) {
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            BDISPLAY::display.printf("Timer: %d\n", (inputBannerLimit - inputBannerTicker)/10);
            BDISPLAY::display.setTextSize(1);
            BDISPLAY::display.printf("\nYour Moves:\n  %s\n", getRoundArrayAsColors(dynamicInputArray, roundArraySize).c_str());
            BDISPLAY::display.display();
            neo.turnAllOff();
            BadgeHaptic::getInstance().hapticOff();
            if(roundInputCounter == roundArraySize){
                inputBannerTicker = inputBannerLimit + 42069;  // you can't stop me
            }
        }
        inputBannerTicker++;
    // Results Banner
    } else if(game_state == OGS_RESULT){
        resultBannerTicker++;
        if (resultBannerTicker < 2){
            // Start building the final display banner
            BDISPLAY::clearAndDisplayBatteryPercentage();
            BDISPLAY::display.setTextSize(2);
            BDISPLAY::display.setTextColor(WHITE);
            BDISPLAY::display.setCursor(0, 0);
            if(game_result == GR_WIN){
                BDISPLAY::display.printf("You Won\n %d Rounds!\n", roundWinCounter);
                BDISPLAY::display.setTextSize(1);
                neo.changeRainbow();
            } else {
                BDISPLAY::display.println(F("Keep"));
                BDISPLAY::display.println(F(" Trying!"));
                BDISPLAY::display.setTextSize(1);
                BDISPLAY::display.printf("Lost on round %d\n", roundWinCounter + 1);
                neo.changeOgrady();
            }
            // Dump game info to serial, score rewards, add a line to the display
            Serial.printf("O'Grady Game Seed: %lu\n", ogsPatternSeed);
            Serial.printf("Score: %d\n", roundWinCounter);
            if(roundWinCounter >= 7){
                CPStore::earnReward(CPStoreReward::REWARD_MAX);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_MAX));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣤⣤⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⠀⠀⠀⢀⣴⠟⠉⠀⠀⠀⠈⠻⣦⡀⠀⠀⠀⣤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣷⣀⢀⣾⠿⠻⢶⣄⠀⠀⣠⣶⡿⠶⣄⣠⣾⣿⠗⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⢻⣿⣿⡿⣿⠿⣿⡿⢼⣿⣿⡿⣿⣎⡟⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⡟⠉⠛⢛⣛⡉⠀⠀⠙⠛⠻⠛⠑⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣧⣤⣴⠿⠿⣷⣤⡤⠴⠖⠳⣄⣀⣹⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⣀⣟⠻⢦⣀⡀⠀⠀⠀⠀⣀⡈⠻⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⡿⠉⡇⠀⠀⠛⠛⠛⠋⠉⠉⠀⠀⠀⠹⢧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣾⡟⠀⢦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠃⠀⠈⠑⠪⠷⠤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣾⣿⣿⣿⣦⣼⠛⢦⣤⣄⡀⠀⠀⠀⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠑⠢⡀⠀⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠀⢀⣠⠴⠲⠖⠛⠻⣿⡿⠛⠉⠉⠻⠷⣦⣽⠿⠿⠒⠚⠋⠉⠁⡞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⢦⠀⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⠀⢀⣾⠛⠁⠀⠀⠀⠀⠀⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠤⠒⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢣⠀⠀⠀"));
                Serial.println(F("⠀⠀⠀⠀⣰⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣑⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⡇⠀⠀"));
                Serial.println(F("⠀⠀⠀⣰⣿⣁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣧⣄⠀⠀⠀⠀⠀⠀⢳⡀⠀"));
                Serial.println(F("⠀⠀⠀⣿⡾⢿⣀⢀⣀⣦⣾⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡰⣫⣿⡿⠟⠻⠶⠀⠀⠀⠀⠀⢳⠀"));
                Serial.println(F("⠀⠀⢀⣿⣧⡾⣿⣿⣿⣿⣿⡷⣶⣤⡀⠀⠀⠀⠀⠀⠀⠀⢀⡴⢿⣿⣧⠀⡀⠀⢀⣀⣀⢒⣤⣶⣿⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇"));
                Serial.println(F("⠀⠀⡾⠁⠙⣿⡈⠉⠙⣿⣿⣷⣬⡛⢿⣶⣶⣴⣶⣶⣶⣤⣤⠤⠾⣿⣿⣿⡿⠿⣿⠿⢿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇"));
                Serial.println(F("⠀⣸⠃⠀⠀⢸⠃⠀⠀⢸⣿⣿⣿⣿⣿⣿⣷⣾⣿⣿⠟⡉⠀⠀⠀⠈⠙⠛⠻⢿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇"));
                Serial.println(F("⠀⣿⠀⠀⢀⡏⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⠿⠿⠛⠛⠉⠁⠀⠀⠀⠀⠀⠉⠠⠿⠟⠻⠟⠋⠉⢿⣿⣦⡀⢰⡀⠀⠀⠀⠀⠀⠀⠁"));
                Serial.println(F("⢀⣿⡆⢀⡾⠀⠀⠀⠀⣾⠏⢿⣿⣿⣿⣯⣙⢷⡄⠀⠀⠀⠀⠀⢸⡄⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣿⣻⢿⣷⣀⣷⣄⠀⠀⠀⠀⢸⠀"));
                Serial.println(F("⢸⠃⠠⣼⠃⠀⠀⣠⣾⡟⠀⠈⢿⣿⡿⠿⣿⣿⡿⠿⠿⠿⠷⣄⠈⠿⠛⠻⠶⢶⣄⣀⣀⡠⠈⢛⡿⠃⠈⢿⣿⣿⡿⠀⠀⠀⠀⠀⡀"));
                Serial.println(F("⠟⠀⠀⢻⣶⣶⣾⣿⡟⠁⠀⠀⢸⣿⢅⠀⠈⣿⡇⠀⠀⠀⠀⠀⣷⠂⠀⠀⠀⠀⠐⠋⠉⠉⠀⢸⠁⠀⠀⠀⢻⣿⠛⠀⠀⠀⠀⢀⠇"));
                Serial.println(F("⠀⠀⠀⠀⠹⣿⣿⠋⠀⠀⠀⠀⢸⣧⠀⠰⡀⢸⣷⣤⣤⡄⠀⠀⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡆⠀⠀⠀⠀⡾⠀⠀⠀⠀⠀⠀⢼⡇"));
                Serial.println(F("⠀⠀⠀⠀⠀⠙⢻⠄⠀⠀⠀⠀⣿⠉⠀⠀⠈⠓⢯⡉⠉⠉⢱⣶⠏⠙⠛⠚⠁⠀⠀⠀⠀⠀⣼⠇⠀⠀⠀⢀⡇⠀⠀⠀⠀⠀⠀⠀⡇"));
                Serial.println(F("⠀⠀⠀⠀⠀⠀⠻⠄⠀⠀⠀⢀⣿⠀⢠⡄⠀⠀⠀⣁⠁⡀⠀⢠⠀⠀⠀⠀⠀⠀⠀⠀⢀⣐⡟⠀⠀⠀⠀⢸⡇⠀⠀⠀⠀⠀⠀⢠⡇"));
            } else if (roundWinCounter == 6){
                CPStore::earnReward(CPStoreReward::REWARD_HIGH);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_HIGH));
                Serial.println(F("  ██████  ▒█████       ▄████  ▒█████   ▒█████  ▓█████▄  ▐██▌ "));
                Serial.println(F("▒██    ▒ ▒██▒  ██▒    ██▒ ▀█▒▒██▒  ██▒▒██▒  ██▒▒██▀ ██▌ ▐██▌ "));
                Serial.println(F("░ ▓██▄   ▒██░  ██▒   ▒██░▄▄▄░▒██░  ██▒▒██░  ██▒░██   █▌ ▐██▌ "));
                Serial.println(F("  ▒   ██▒▒██   ██░   ░▓█  ██▓▒██   ██░▒██   ██░░▓█▄   ▌ ▓██▒ "));
                Serial.println(F("▒██████▒▒░ ████▓▒░   ░▒▓███▀▒░ ████▓▒░░ ████▓▒░░▒████▓  ▒▄▄  "));
                Serial.println(F("▒ ▒▓▒ ▒ ░░ ▒░▒░▒░     ░▒   ▒ ░ ▒░▒░▒░ ░ ▒░▒░▒░  ▒▒▓  ▒  ░▀▀▒ "));
                Serial.println(F("░ ░▒  ░ ░  ░ ▒ ▒░      ░   ░   ░ ▒ ▒░   ░ ▒ ▒░  ░ ▒  ▒  ░  ░ "));
                Serial.println(F("░  ░  ░  ░ ░ ░ ▒     ░ ░   ░ ░ ░ ░ ▒  ░ ░ ░ ▒   ░ ░  ░     ░ "));
                Serial.println(F("      ░      ░ ░           ░     ░ ░      ░ ░     ░     ░    "));
                Serial.println(F("                                                ░            "));
            } else  if (roundWinCounter == 5){
                CPStore::earnReward(CPStoreReward::REWARD_MED);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_MED));
                Serial.println(F("Not bad, dork... not too bad."));
            } else  if (roundWinCounter == 4){
                CPStore::earnReward(CPStoreReward::REWARD_MED);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_MED));
                Serial.println(F("Your mom did about this good."));
            } else  if (roundWinCounter == 3){
                CPStore::earnReward(CPStoreReward::REWARD_LOW);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_LOW));
                Serial.println(F("Don't you dare blame this on your neuro-whatever \"condition\"."));
            } else  if (roundWinCounter == 2){
                CPStore::earnReward(CPStoreReward::REWARD_LOW);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_LOW));
                Serial.println(F("Are you done \"warming up\"??"));
            } else if (roundWinCounter == 1){
                CPStore::earnReward(CPStoreReward::REWARD_MIN);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_MIN));
                Serial.println(F("Well. At least you tried."));
            } else {
                CPStore::earnReward(CPStoreReward::REWARD_MIN);
                BDISPLAY::display.printf("Awarded $%d\n", CPStore::getRewardAmount(CPStoreReward::REWARD_MIN));
                Serial.println(F("Did you even try?"));
            }
            // Call display to show our built screen.
            BDISPLAY::display.display();
            JsonDocument doc;
            doc["game"] = "ogrady";
            doc["roundsWon"] = roundWinCounter;
            CyberPartner::getInstance().queueGeneralUpdate("cp/game/play", doc);
        }

    }
}

void OGS::doStart()
{
    // disable menu before starting
    BMenu::disableMenu();

    NeoController::getInstance().changeOgrady();
    Serial.println(F("Tick, Tock, Tick, Tock... Shit clock's tickin', Rick."));
    game_state = OGS_TITLEBANNER;
    game_result = GR_DISABLED;
    titleBannerTicker = 0;
    instructBannerTicker = 0;
    patternBannerTicker = 0;
    inputBannerTicker = 0;
    resultBannerTicker = 0;

    roundScalar = 20;  //  20 is a pretty good scalar
    roundWinCounter = 0;
    roundArraySize = 3;
    roundShowPatternCounter = 0;
    inputBannerLimit = roundArraySize * roundScalar;

    ogsPatternSeed = millis() + ESP.getCycleCount();  // uptime + CPU cycle count, yuge variation
    randomSeed(ogsPatternSeed);
}

void OGS::doStop()
{
    // Disable game, empty vars & deallocate arrays
    game_state = OGS_DISABLED;
    titleBannerTicker = 0;
    instructBannerTicker = 0;
    patternBannerTicker = 0;
    inputBannerTicker = 0;
    resultBannerTicker = 0;

    roundScalar = 0;
    roundWinCounter = 0;
    roundArraySize = 0;
    roundInputCounter = 0;
    roundShowPatternCounter = 0;
    inputBannerLimit = 0;

    ogsPatternSeed = 0;
    randomSeed(ogsPatternSeed);  // ~deseed random

    if (dynamicInputArray != nullptr) {
        delete[] dynamicInputArray;
        dynamicInputArray = nullptr;
    }
    if (dynamicRoundArray != nullptr) {
        delete[] dynamicRoundArray;
        dynamicRoundArray = nullptr;
    }
    NeoController::getInstance().turnAllOff();
    BadgeHaptic::getInstance().hapticOff();
    BMenu::enableMenu();  // enable menu since we are done
}

void OGS::update()
{
    if (game_state != OGS_DISABLED) {
        if (gameTimer.check(OGS_CHECK_TIME_INTERVAL)) {
            runOgradySays();
        }
    }
}

void OGS::setup()
{
    game_state = OGS_DISABLED;
}

void OGS::clickYellowButton()  // top left (button 1)
{
    if(game_state == OGS_GET_INPUT && roundInputCounter < roundArraySize){
        NeoController::getInstance().changeOgradyTopLeft();
        BadgeHaptic::getInstance().hapticOn();
        dynamicInputArray[roundInputCounter] = 1;
        inputBannerTicker -= (inputBannerTicker % 10);
        roundInputCounter++;
    }
}

void OGS::clickRedButton()  // bottom left (button 2)
{
    if(game_state == OGS_GET_INPUT && roundInputCounter < roundArraySize){
        NeoController::getInstance().changeOgradyBottomLeft();
        BadgeHaptic::getInstance().hapticOn();
        dynamicInputArray[roundInputCounter] = 2;
        inputBannerTicker -= (inputBannerTicker % 10);
        roundInputCounter++;
    }
}

void OGS::clickGreenButton()  // bottom right (button 3)
{
    if(game_state == OGS_GET_INPUT && roundInputCounter < roundArraySize){
        NeoController::getInstance().changeOgradyBottomRight();
        BadgeHaptic::getInstance().hapticOn();
        dynamicInputArray[roundInputCounter] = 3;
        inputBannerTicker -= (inputBannerTicker % 10);
        roundInputCounter++;
    }
}

void OGS::clickBlueButton()  // top right (button 4)
{
    if(game_state == OGS_GET_INPUT && roundInputCounter < roundArraySize){
        NeoController::getInstance().changeOgradyTopRight();
        BadgeHaptic::getInstance().hapticOn();
        dynamicInputArray[roundInputCounter] = 4;
        inputBannerTicker -= (inputBannerTicker % 10);
        roundInputCounter++;
    }
}
