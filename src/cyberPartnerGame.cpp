/**
 * @file cyberPartnerGame.cpp
 * @brief Implements the CyberPartnerGame class methods.
 *
 * This file contains the implementation of the CyberPartnerGame class, which manages
 * the game logic, animations, and display updates for the CyberPartner game.
 */
#include "cyberPartnerGame.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "badge_display.h"
#include "badgeMenu.h"
#include "badge.h"
#include "ProgressConfig.h"
#include <NetworkTaskManager.h>
#include "CyberPartnerSprite.h"
#include "CPStore.h"
#include "haptic.h"

// Static member definitions
bool CyberPartnerGame::status = false;
bool CyberPartnerGame::death_move = false;
bool CyberPartnerGame::statsScreenEnabled = false;
bool CyberPartnerGame::trackBuzzTime = false;
uint8_t CyberPartnerGame::minX = 0;
uint8_t startX = BDISPLAY::SCREEN_WIDTH/2 - 10;
extern Scheduler baseTaskRunner;
extern Scheduler highPriorityTaskRunner;
Scheduler& CyberPartnerGame::taskRunner = baseTaskRunner;
Scheduler& CyberPartnerGame::highPrioTaskRunner = highPriorityTaskRunner;

Task CyberPartnerGame::displayTask(100, TASK_FOREVER, &animateCyberPartner);
Task CyberPartnerGame::dayUpdateTask(6000, TASK_FOREVER, &dayUpdateAction);

constexpr char titleStr[] PROGMEM = "Your CyberPartner!";

CyberPartnerSprite tsprite(Character::babyKnobby, Action::General1, startX, BDISPLAY::SCREEN_HEIGHT);

CyberPartnerGame& CyberPartnerGame::getInstance() {
    static CyberPartnerGame instance;
    return instance;
}

/**
 * @brief Enables the game.
 * @return True if the game is successfully enabled, false otherwise.
 */
void CyberPartnerGame::enable() {
    #ifdef CPGAMEDEBUG
    Serial.println(F("CyberPartnerGame: enable()"));
    #endif

    setActive(true);

    //displayTask.setInterval(100);
    //displayTask.setIterations(TASK_FOREVER);
    //displayTask.setCallback(animateCyberPartner);
    taskRunner.addTask(displayTask);
    displayTask.enable();

    CyberPartnerGame::displayStatusBar();
    animateCyberPartner();
}

/**
 * @brief Disables the game.
 */
void CyberPartnerGame::disable() {
    #ifdef CPGAMEDEBUG
    Serial.println(F("CyberPartnerGame: disable()"));
    #endif

    setActive(false);
    cancelBuzzTime();

    displayTask.disable();
    taskRunner.deleteTask(displayTask);
}

/**
 * @brief Checks if the game is active.
 * @return True if the game is active, false otherwise.
 */
bool CyberPartnerGame::isActive() {
    return status;
}

/**
 * @brief Sets the game status to active or inactive.
 * @param active The new status value (0 for inactive, 1 for active).
 */
void CyberPartnerGame::setActive(bool active) {
    status = active;
}

/**
 * @brief Animates the CyberPartner based on its current life phase.
 */
void CyberPartnerGame::animateCyberPartner() {
    CyberPartnerGame::displayStatusBar();
    LifePhase phase = CyberPartner::getInstance().getLifePhase();

    switch (phase) {
        case LifePhase::Egg:
            animateEggPhase();
            break;
        case LifePhase::Death:
            animateDeathPhase();
            break;
        default:
            animateOtherPhases();
            break;
    }
}

/**
 * @brief Animates the egg phase of the CyberPartner.
 */
void CyberPartnerGame::animateEggPhase() {
    if (statsScreenEnabled) {
        displayStatsScreen();
    } else {
        BDISPLAY::displayCenteredText(titleStr, 10, 1);
    }
    NeoController::getInstance().changePopo();
    static uint8_t prevActionChange = 0;
    static bool egg_state = true;

    if (prevActionChange <= 0) {
        prevActionChange = random(5, 20);
        egg_state = !egg_state;
        uint8_t width = birth_death_sprites_sizes[egg_state][0];
        uint8_t height = birth_death_sprites_sizes[egg_state][1];
        const unsigned char* egg_sprite = birth_death_sprites_images[egg_state];
        uint8_t y = BDISPLAY::SCREEN_HEIGHT - height;
        BDISPLAY::display.drawBitmap(startX, y, egg_sprite, width, height, WHITE);
        BDISPLAY::display.display();
    } else {
        prevActionChange -= 1;
    }
}

/**
 * @brief Animates the death phase of the CyberPartner.
 */
void CyberPartnerGame::animateDeathPhase() {
    if (statsScreenEnabled) {
        displayStatsScreen();
    }

    CyberPartner& partner = CyberPartner::getInstance();

    BDISPLAY::displayCenteredText(partner.WaysToDieToString(partner.getDeathCause()), BDISPLAY::SCREEN_HEIGHT - 8, 1);

    static uint16_t death_x = 0;
    static uint16_t death_y = 0;

    static unsigned long animationStartTime = 0; // Tracks when the animation starts
    static unsigned long totalAnimationTime = 0; // Accumulates the total animation time

    uint8_t death_state = 2;
    uint8_t width = birth_death_sprites_sizes[death_state][0];
    uint8_t height = birth_death_sprites_sizes[death_state][1];
    const unsigned char* egg_sprite = birth_death_sprites_images[death_state];

    // Determine the target X position based on whether the stats screen is enabled
    uint16_t target_x = statsScreenEnabled
        ? (BDISPLAY::SCREEN_WIDTH / 4) * 3 - width / 2 // Center of the right half
        : BDISPLAY::SCREEN_WIDTH / 2 - width / 2;     // True center of the screen

    if (!death_move) {
        NeoController::getInstance().changeWhite();
        death_move = !death_move;
        death_x = tsprite.getXLocation();
        death_y = BDISPLAY::SCREEN_HEIGHT - height;

        // Start the animation timer
        trackBuzzTime = true;
        animationStartTime = millis();
        if (!BadgeHaptic::getInstance().isHapticOn()) {
            BadgeHaptic::getInstance().hapticOn();
        }
    } else {
        // Move horizontally toward the target X position
        if (death_x < target_x) {
            death_x += 1;
        } else if (death_x > target_x) {
            death_x -= 1;
        }

        // Move vertically toward the target Y position (10)
        if (death_y > 10) {
            death_y -= 1;
        }

        if (death_x == target_x && death_y == 10) {
            // Stop the haptic motor when the sprite reaches the target position
            if (BadgeHaptic::getInstance().isHapticOn()) {
                BadgeHaptic::getInstance().hapticOff();
            }


            if (trackBuzzTime) {
                // Stop tracking the animation time
                trackBuzzTime = false;
                // Accumulate the total animation time
                totalAnimationTime += millis() - animationStartTime;

                // Log the total animation time
                Serial.print("Death Buzz: ");
                Serial.print(totalAnimationTime / 1000.0); // Convert to seconds
                Serial.println(" seconds");

                if (totalAnimationTime > 20000) {
                    // Celebrate the animation time if it exceeds 20 seconds
                    JsonDocument buzzDoc;
                    buzzDoc["game"] = "deathbuzz";
                    buzzDoc["totaltime"] = totalAnimationTime;

                    partner.queueGeneralUpdate("cp/game/play", buzzDoc);
                }

                // Reset the animation start time
                // This ensures that the next animation starts from the current time
                animationStartTime = 0;
                totalAnimationTime = 0;
            }
        }
    }

    // Draw the sprite at the current position
    BDISPLAY::display.drawBitmap(death_x, death_y, egg_sprite, width, height, WHITE);
    BDISPLAY::display.display();
}

/**
 * @brief Animates other life phases of the CyberPartner.
 */
void CyberPartnerGame::animateOtherPhases() {
    if (statsScreenEnabled) {
        displayStatsScreen();
    } else {
        BDISPLAY::displayCenteredText(titleStr, 10, 1);
    }
    tsprite.move(minX);
    tsprite.draw();
    BDISPLAY::display.display();
}

/**
 * @brief Configures the status bar for the CyberPartner game.
 */
void CyberPartnerGame::displayStatusBar() {
    BDISPLAY::clearAndDisplayBatteryPercentage();
    if (!statsScreenEnabled) {
        displayStatusAge();
        displayStatusWeight();
    }
    BDISPLAY::display.setCursor(0, 0);
    BDISPLAY::display.setTextSize(1);
}

/**
 * @brief Displays the age of the CyberPartner on the screen.
 */
void CyberPartnerGame::displayStatusAge() {
    uint16_t locX = 10;
    uint16_t locY = 2;
    uint8_t ageIndex = 3;
    drawStatusSprite(locX, locY, ageIndex);
    displayStatusText(locX + 8, (String(CyberPartner::getInstance().getAge()) + "y").c_str());
}

/**
 * @brief Displays the weight of the CyberPartner on the screen.
 */
void CyberPartnerGame::displayStatusWeight() {
    uint16_t locX = 50;
    uint16_t locY = 0;
    uint8_t weightIndex = 4;
    drawStatusSprite(locX, locY, weightIndex);
    displayStatusText(locX + 10, (String(CyberPartner::getInstance().getWeight()) + "g").c_str());
}

void CyberPartnerGame::toggleStatsScreen()
{
    if (!statsScreenEnabled) {
        statsScreenEnabled = true;
        minX = BDISPLAY::SCREEN_WIDTH / 2;
        startX = (BDISPLAY::SCREEN_WIDTH - minX)/2 + minX - 10;
    } else {
        statsScreenEnabled = false;
        minX = 0;
        startX = BDISPLAY::SCREEN_WIDTH/2 - 10;
    }
}

void CyberPartnerGame::cancelBuzzTime() {
    if (trackBuzzTime) {
        trackBuzzTime = false;
        //totalAnimationTime = 0;
        //animationStartTime = 0;
        if (BadgeHaptic::getInstance().isHapticOn()) {
            BadgeHaptic::getInstance().hapticOff();
        }
    }
}

void CyberPartnerGame::displayStatsScreen()
{
    BDISPLAY::display.setTextSize(1);
    BDISPLAY::display.setTextColor(WHITE);
    BDISPLAY::display.setCursor(0, 00);
    CyberPartner& partner = CyberPartner::getInstance();

    BDISPLAY::display.print("Aage: ");
    BDISPLAY::display.print(partner.attributes.age);
    BDISPLAY::display.println("y");

    BDISPLAY::display.print("Hlth: ");
    BDISPLAY::display.println(partner.attributes.health);

    BDISPLAY::display.print("Wght: ");
    BDISPLAY::display.print(partner.attributes.weight);
    BDISPLAY::display.println("g");

    BDISPLAY::display.print("Hngr: ");
    BDISPLAY::display.println(partner.attributes.hunger);

    BDISPLAY::display.print("Trst: ");
    BDISPLAY::display.println(partner.attributes.thirst);

    BDISPLAY::display.print("Hapi: ");
    BDISPLAY::display.println(partner.attributes.happiness);

    BDISPLAY::display.print("Faze: ");
    BDISPLAY::display.println(partner.lifePhaseToString(partner.attributes.lifePhase));
}

/**
 * @brief Draws a sprite on the screen at the specified location.
 * @param locX The x-coordinate of the sprite's location.
 * @param locY The y-coordinate of the sprite's location.
 * @param spriteIndex The index of the sprite to draw.
 */
void CyberPartnerGame::drawStatusSprite(uint16_t locX, uint16_t locY, uint8_t spriteIndex) {
    uint8_t width = birth_death_sprites_sizes[spriteIndex][0];
    uint8_t height = birth_death_sprites_sizes[spriteIndex][1];
    const unsigned char* sprite = birth_death_sprites_images[spriteIndex];
    BDISPLAY::display.drawBitmap(locX, locY, sprite, width, height, WHITE);
}

/**
 * @brief Displays text on the screen at the specified location.
 * @param locX The x-coordinate of the text's location.
 * @param text The text to display.
 */
void CyberPartnerGame::displayStatusText(uint16_t locX, const char* text) {
    BDISPLAY::display.setTextSize(1);
    BDISPLAY::display.setTextColor(WHITE);
    BDISPLAY::display.setCursor(locX, 0);
    BDISPLAY::display.print(text);
}

/**
 * @brief Displays a message on the screen.
 * @param text The text to display.
 */
void CyberPartnerGame::displayText(const char* text) {
    CyberPartnerGame::displayStatusBar();
    BDISPLAY::display.println(text);
    BDISPLAY::display.display();
}

/**
 * @brief Enters the CyberPartner game.
 *
 * This function initializes the game and starts the necessary tasks.
 */
void CyberPartnerGame::start() {
    #ifdef CPGAMEDEBUG
    Serial.println(F("CyberPartnerGame: start()"));
    #endif

    CyberPartner::getInstance().loadCPStatsFile();
    CPStore::getInstance().loadStoreFile();

    taskRunner.addTask(dayUpdateTask);
    dayUpdateTask.enable();

    enable();
}

/**
 * @brief Updates the CyberPartner's stats periodically.
 */
void CyberPartnerGame::statUpdateAction() {
    #ifdef CPGAMEDEBUG
    Serial.println(F("CyberPartnerGame: statUpdateAction()"));
    #endif

    CyberPartner& partner = CyberPartner::getInstance();

    partner.increaseHunger();
}

/**
 * @brief Updates the CyberPartner's life phase and appearance based on its age.
 */
void CyberPartnerGame::dayUpdateAction() {
    NeoController& neo = NeoController::getInstance();

    #ifdef CPGAMEDEBUG
    //Serial.println(F("CyberPartnerGame: dayUpdateAction()"));
    #endif

    CyberPartner& partner = CyberPartner::getInstance();

    partner.determineAge();

    if (partner.attributes.rock) {
        neo.turnAllOff();
        tsprite.setCharacter(Character::rock);
    } else {
        switch (partner.getLifePhase()) {
            case LifePhase::Baby:
                neo.changeRainbow();
                tsprite.setCharacter(Character::babyKnobby);
                break;
            case LifePhase::Toddler:
                neo.changeConfetti();
                tsprite.setCharacter(Character::toddlerKnobby);
                break;
            case LifePhase::Child:
                neo.changeConfetti();
                tsprite.setCharacter(Character::childKnobby);
                break;
            case LifePhase::Teen:
                neo.changeChase();
                tsprite.setCharacter(Character::teenKnobby);
                break;
            case LifePhase::Adult:
                neo.changeSmoothChase();
                tsprite.setCharacter(Character::adultKnobby);
                break;
            case LifePhase::Senior:
                neo.changeSurge();
                tsprite.setCharacter(Character::seniorKnobby);
                break;
            default:
                // This shouldn't happen (Egg or Death Phase)
                break;
        }
    }
}

/**
 * @brief Handles the left action button press.
 */
void CyberPartnerGame::performLeftAction() {
    toggleStatsScreen();
    animateCyberPartner();
}

/**
 * @brief Handles the middle action button press.
 */
void CyberPartnerGame::performMiddleAction() {
    CyberPartner::getInstance().die(WaysToDie::DeathButton);
}

/**
 * @brief Handles the right action button press.
 */
void CyberPartnerGame::performRightAction() {
    CyberPartner& partner = CyberPartner::getInstance();

    if (partner.getLifePhase() == LifePhase::Death) {
        partner.bornAgain();
    } else {
        Serial.println("Your CyberPartner is not dead yet!");
    }
    // Reset the sprite position
    // and the death move flag
    death_move = false;
    tsprite.setLocation(startX, BDISPLAY::SCREEN_HEIGHT);
    Serial.println("Reset CyberPartner Age");
}

/**
 * @brief Handles the aftermath of dropping/launching the CyberPartner.
 * Death if egg phase, otherwise decrease happiness
 */
void CyberPartnerGame::handleFreeFallEvent( float meters ) {
    CyberPartner& partner = CyberPartner::getInstance();
    ProgressConfig& progconfig = ProgressConfig::getInstance();

    if (partner.getLifePhase() == LifePhase::Egg) {
        // If the CyberPartner is in the Egg phase, it dies
        Serial.println("You dropped your CyberPartner egg... SPLAT!");
        partner.die();
    } else {
        // If the CyberPartner is older than Egg, decrease happiness
        int currentHappiness = partner.attributes.happiness;
        partner.setHappiness(max(0, currentHappiness - 10)); // Decrease happiness by 10, but not below 0
        Serial.println("You dropped your CyberPartner. Nobody liked that.");
    }

    // Tell the server about the drop-death event
    JsonDocument dropDoc;
    dropDoc["event"] = "Drop";
    dropDoc["distanceMeters"] = meters;
    partner.queueGeneralUpdate("cp/event", dropDoc);

    // If it's your first drop, have an unlock
    if (!progconfig.isGotDroppedUnlocked()) {
        Serial.printf("GotDropped Achievement Unlocked!\n");
        progconfig.unlockGotDropped();
        progconfig.saveProgressFile();
    }
}