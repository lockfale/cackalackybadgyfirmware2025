/**
 * @file CPStoreDisplay.cpp
 * @brief Implements the CPPStore class methods.
 *
 * This file contains the logic for handling CPPStore Display functionality.
 * It includes the constructor, destructor, and methods for displaying the store.
 */
#include "CPStoreDisplay.h"
#include "badgeMenu.h"
#include "badge_display.h"

static CPStoreSprite msprite(CPStoreMeals::MEAL_APPLE, BDISPLAY::SCREEN_WIDTH/2, BDISPLAY::SCREEN_HEIGHT);
CPStoreMeals CPStoreDisplay::current_meal = CPStoreMeals::MEAL_APPLE;


CPStoreDisplay::CPSTORE_STATE_t CPStoreDisplay::cpstore_state = CPStoreDisplay::CPSTORE_STATE_t::CPSTORE_DISPLAY_DISABLED;
CPStoreDisplay::BUY_STATE_t CPStoreDisplay::buy_state = CPStoreDisplay::BUY_STATE_t::BUY_DISABLED;

constexpr char titleStr[] PROGMEM = "Welcome to CPStore!";

CPStoreDisplay& CPStoreDisplay::getInstance() {
    static CPStoreDisplay instance;
    return instance;
}

/**
 * @brief Configures the status bar for the CPStore.
 */
void CPStoreDisplay::displayBar() {
    BDISPLAY::clearAndDisplayBatteryPercentage();
    displayMoney();

    BDISPLAY::displayCenteredText(titleStr, 10, 1);

    displayMeal();

    displayBuyState();

    BDISPLAY::display.display();
}

void CPStoreDisplay::displayBuyState() {
    CPStore& store = CPStore::getInstance();

    uint8_t startLocX = 45;
    uint8_t startLocY = 25;
    BDISPLAY::display.setCursor(startLocX, startLocY);
    // Use a pre-allocated buffer for the text instead of multiple String concatenations
    // to avoid heap fragmentation on ESP8266
    char mealText[32]; // Adjust size as needed
    snprintf(mealText, sizeof(mealText), "Buy %s?", store.mealToString(current_meal));
    BDISPLAY::display.println(mealText);

    char moneyText[16];
    snprintf(moneyText, sizeof(moneyText), "%uc", store.getMealCost(current_meal));
    BDISPLAY::displayRightJustifiedText(moneyText, startLocX + 35, startLocY + 15, 1);

    // Draw a rectangle around the "Yes" and "No" options
    // Adjust the coordinates and dimensions as needed
    uint8_t yesnoX = startLocX + 40; // Y-coordinate of "Yes"
    uint8_t yesnoY = startLocY + 15; // Y-coordinate of "Yes"
    uint8_t letterWidth = 10;           // Width of the rectangle around each letter
    uint8_t yesHeight = 12;          // Height of the rectangle around "Yes"
    uint8_t noHeight = 12;           // Height of the rectangle around "No"

    BDISPLAY::display.setCursor(yesnoX, yesnoY);
    BDISPLAY::display.println(F("Y"));

    BDISPLAY::display.setCursor(yesnoX + 10, yesnoY);
    BDISPLAY::display.println(F("N"));

    // Display the current buy state
    switch (buy_state) {
        case BUY_STATE_t::BUY_YES:
            // Draw the rectangle around "Yes"
            BDISPLAY::display.drawRect(yesnoX - 2, yesnoY - 2, letterWidth, yesHeight, WHITE);
            break;
        case BUY_STATE_t::BUY_NO:
            // Draw the rectangle around "No"
            BDISPLAY::display.drawRect(yesnoX + 10 - 2, yesnoY - 2, letterWidth, noHeight, WHITE);
            break;
        default:
            // Handle unexpected state
            break;
    }

    uint8_t ownedLocY = yesnoY + 15;
    snprintf(mealText, sizeof(mealText), "Owned: %u", store.getMealCount(current_meal));
    BDISPLAY::display.setCursor(startLocX, ownedLocY);
    BDISPLAY::display.println(mealText);
}

void CPStoreDisplay::displayMeal() {
    msprite.setMealStage(current_meal, CPMealsStages::STAGE_FULL);
    msprite.setLocation(10, BDISPLAY::SCREEN_HEIGHT - 7);
    msprite.draw();
}

/**
 * @brief Displays the amount of money that the CP has on the screen.
 */
void CPStoreDisplay::displayMoney() {
    uint16_t locX = 10;
    //uint16_t locY = 2;
    displayStatusText(locX + 8, (String(CPStore::getInstance().getMoney()) + "c").c_str());
}

/**
 * @brief Displays text on the screen at the specified location.
 * @param locX The x-coordinate of the text's location.
 * @param text The text to display.
 */
void CPStoreDisplay::displayStatusText(uint16_t locX, const char* text) {
    BDISPLAY::display.setTextSize(1);
    BDISPLAY::display.setTextColor(WHITE);
    BDISPLAY::display.setCursor(locX, 0);
    BDISPLAY::display.print(text);
}

/**
 * @brief Handles the left action button press.
 */
void CPStoreDisplay::performLeftAction() {
    if(buy_state == BUY_STATE_t::BUY_DISABLED) {
        // Move to the previous meal, looping to the last meal if at the first
        current_meal = static_cast<CPStoreMeals>(
            (static_cast<int>(current_meal) - 1 + static_cast<int>(CPStoreMeals::MEAL_COUNT))
            % static_cast<int>(CPStoreMeals::MEAL_COUNT)
        );
    } else {
        // Toggle between BUY_YES and BUY_NO
        buy_state = (buy_state == BUY_STATE_t::BUY_YES) ? BUY_STATE_t::BUY_NO : BUY_STATE_t::BUY_YES;
    }
    displayBar();
}

/**
 * @brief Handles the middle action button press.
 */
void CPStoreDisplay::performMiddleAction() {
    if(buy_state == BUY_STATE_t::BUY_DISABLED) {
        // Move to the next meal, looping to the first meal if at the last
        current_meal = static_cast<CPStoreMeals>(
            (static_cast<int>(current_meal) + 1) % static_cast<int>(CPStoreMeals::MEAL_COUNT)
        );
    } else {
        // Toggle between BUY_YES and BUY_NO
        buy_state = (buy_state == BUY_STATE_t::BUY_YES) ? BUY_STATE_t::BUY_NO : BUY_STATE_t::BUY_YES;
    }
    displayBar();
}

/**
 * @brief Handles the right action button press.
 */
void CPStoreDisplay::performRightAction() {
    if(buy_state == BUY_STATE_t::BUY_DISABLED) {
        buy_state = BUY_STATE_t::BUY_YES;
    } else if(buy_state == BUY_STATE_t::BUY_YES) {
        // Attempt to purchase the meal
        if (CPStore::getInstance().purchaseMeal(current_meal)) {
            // Purchase successful
            buy_state = BUY_STATE_t::BUY_DISABLED;
        } else {
            // Not enough money
            displayStatusText(10, "Not enough money!");
            delay(2000);
        }
    } else {
        // Cancel the purchase
        buy_state = BUY_STATE_t::BUY_DISABLED;
    }
    displayBar();
}

/**
 * @brief Handles the boot button action.
 */
void CPStoreDisplay::performBootAction()
{
    if(buy_state == BUY_STATE_t::BUY_DISABLED) {
        CPStoreDisplay::doStop();
        BMenu::enableMenu();
    } else {
        // Cancel the purchase
        buy_state = BUY_STATE_t::BUY_DISABLED;
        displayBar();
    }
}

bool CPStoreDisplay::isActive( void )
{
    return cpstore_state != CPSTORE_STATE_t::CPSTORE_DISPLAY_DISABLED;
}

void CPStoreDisplay::doStart( void )
{
    // disable menu before starting
    BMenu::disableMenu();

    cpstore_state = CPSTORE_STATE_t::CPSTORE_DISPLAY_RUNNING;
    displayBar();

}
void CPStoreDisplay::doStop( void )
{
    cpstore_state = CPSTORE_STATE_t::CPSTORE_DISPLAY_DISABLED;

    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.display();

    // enable menu since we are done
    BMenu::enableMenu();
}
