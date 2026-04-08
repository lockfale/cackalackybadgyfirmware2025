/**
 * @file CPInventoryDisplay.cpp
 * @brief Implements the CPPStore class methods.
 *
 * This file contains the logic for handling CPPStore Display functionality.
 * It includes the constructor, destructor, and methods for displaying the store.
 */
#include "CPInventoryDisplay.h"
#include "badgeMenu.h"
#include "badge_display.h"

static CPStoreSprite msprite(CPStoreMeals::MEAL_APPLE, BDISPLAY::SCREEN_WIDTH/2, BDISPLAY::SCREEN_HEIGHT);
CPStoreMeals CPInventoryDisplay::current_meal = CPStoreMeals::MEAL_APPLE;

CPInventoryDisplay::CPINVENTORY_STATE_t CPInventoryDisplay::cpinventory_state = CPInventoryDisplay::CPINVENTORY_STATE_t::CPINVENTORY_DISPLAY_DISABLED;
CPInventoryDisplay::USE_STATE_t CPInventoryDisplay::use_state = CPInventoryDisplay::USE_STATE_t::USE_DISABLED;
CPInventoryDisplay::DISPLAY_STATE_t CPInventoryDisplay::display_state = DISPLAY_STATE_t::DISPLAY_NO_ITEMS_IN_INVENTORY;

constexpr char titleStr[] PROGMEM = "My Inventory";
constexpr char noItemsStr[] PROGMEM = "No items";
constexpr char inInventoryStr[] PROGMEM = "in inventory.";
constexpr char playGameStr[] PROGMEM = "Play a game!";

CPInventoryDisplay& CPInventoryDisplay::getInstance() {
    static CPInventoryDisplay instance;
    return instance;
}

/**
 * @brief Configures the status bar for the CPStore.
 */
void CPInventoryDisplay::displayBar() {
    CPStore& store = CPStore::getInstance();

    BDISPLAY::clearAndDisplayBatteryPercentage();

    BDISPLAY::displayCenteredText(titleStr, 10, 1);

    if (store.getMealCount(current_meal) > 0) {
        display_state = DISPLAY_STATE_t::DISPLAY_ITEMS_IN_INVENTORY;
    } else if (store.getMealCount(current_meal) == 0) {
        display_state = DISPLAY_STATE_t::DISPLAY_NO_MORE_ITEM;
    }


    if (display_state == DISPLAY_STATE_t::DISPLAY_NO_ITEMS_IN_INVENTORY) {
        BDISPLAY::displayCenteredText(noItemsStr, 25, 1);
        BDISPLAY::displayCenteredText(inInventoryStr, 35, 1);
        BDISPLAY::displayCenteredText(playGameStr, 45, 1);
    } else {
        displayMeal();
        if (store.getMealCount(current_meal) > 0) {
            displayUseState();
        } else {
            BDISPLAY::display.setCursor(45, 25);
            char mealText[32]; // Adjust size as needed
            snprintf(mealText, sizeof(mealText), "No %s", store.mealToString(current_meal));
            BDISPLAY::display.println(mealText);
        }
    }

    BDISPLAY::display.display();
}

void CPInventoryDisplay::displayUseState() {
    CPStore& store = CPStore::getInstance();

    uint8_t startLocX = 45;
    uint8_t startLocY = 25;
    BDISPLAY::display.setCursor(startLocX, startLocY);
    // Use a pre-allocated buffer for the text instead of multiple String concatenations
    // to avoid heap fragmentation on ESP8266
    char mealText[32]; // Adjust size as needed
    snprintf(mealText, sizeof(mealText), "Use %s?", store.mealToString(current_meal));
    BDISPLAY::display.println(mealText);

    // Draw a rectangle around the "Yes" and "No" options
    // Adjust the coordinates and dimensions as needed
    uint8_t yesnoY = startLocY + 15; // Y-coordinate of "Yes"
    uint8_t yesWidth = 21;           // Width of the rectangle around "Yes"
    uint8_t yesHeight = 12;          // Height of the rectangle around "Yes"
    uint8_t noWidth = 16;            // Width of the rectangle around "No"
    uint8_t noHeight = 12;           // Height of the rectangle around "No"

    BDISPLAY::display.setCursor(startLocX, yesnoY);
    BDISPLAY::display.println(F("Yes"));

    BDISPLAY::display.setCursor(startLocX + 25, yesnoY);
    BDISPLAY::display.println(F("No"));

    // Display the current buy state
    switch (use_state) {
        case USE_STATE_t::USE_YES:
            // Draw the rectangle around "Yes"
            BDISPLAY::display.drawRect(startLocX - 2, yesnoY - 2, yesWidth, yesHeight, WHITE);
            break;
        case USE_STATE_t::USE_NO:
            // Draw the rectangle around "No"
            BDISPLAY::display.drawRect(startLocX + 25 - 2, yesnoY - 2, noWidth, noHeight, WHITE);
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

void CPInventoryDisplay::displayMeal() {
    msprite.setMealStage(current_meal, CPMealsStages::STAGE_FULL);
    msprite.setLocation(10, BDISPLAY::SCREEN_HEIGHT - 7);
    msprite.draw();
}

/**
 * @brief Displays the amount of money that the CP has on the screen.
 */
void CPInventoryDisplay::displayMoney() {
    uint16_t locX = 10;
    //uint16_t locY = 2;
    displayStatusText(locX + 8, (String(CPStore::getInstance().getMoney()) + "c").c_str());
}

/**
 * @brief Displays text on the screen at the specified location.
 * @param locX The x-coordinate of the text's location.
 * @param text The text to display.
 */
void CPInventoryDisplay::displayStatusText(uint16_t locX, const char* text) {
    BDISPLAY::display.setTextSize(1);
    BDISPLAY::display.setTextColor(WHITE);
    BDISPLAY::display.setCursor(locX, 0);
    BDISPLAY::display.print(text);
}

/**
 * @brief Handles the left action button press.
 */
void CPInventoryDisplay::performLeftAction() {
    if(use_state == USE_STATE_t::USE_DISABLED) {
        // Find the previous owned meal
        int mealCount = static_cast<int>(CPStoreMeals::MEAL_COUNT);
        for (int i = 1; i <= mealCount; i++) {
            // Calculate the previous meal index, wrapping around if necessary
            CPStoreMeals prevMeal = static_cast<CPStoreMeals>(
                (static_cast<int>(current_meal) - i + mealCount) % mealCount
            );

            // Check if the user owns this meal
            if (CPStore::getInstance().getMealCount(prevMeal) > 0) {
                current_meal = prevMeal;
                break;
            }
        }
    } else {
        // Toggle between BUY_YES and BUY_NO
        use_state = (use_state == USE_STATE_t::USE_YES) ? USE_STATE_t::USE_NO : USE_STATE_t::USE_YES;
    }
    displayBar();
}

/**
 * @brief Handles the middle action button press.
 */
void CPInventoryDisplay::performMiddleAction() {
    if(use_state == USE_STATE_t::USE_DISABLED) {
        // Find the next owned meal
        int mealCount = static_cast<int>(CPStoreMeals::MEAL_COUNT);
        for (int i = 1; i <= mealCount; i++) {
            // Calculate the next meal index, wrapping around if necessary
            CPStoreMeals nextMeal = static_cast<CPStoreMeals>(
                (static_cast<int>(current_meal) + i) % mealCount
            );

            // Check if the user owns this meal
            if (CPStore::getInstance().getMealCount(nextMeal) > 0) {
                current_meal = nextMeal;
                break;
            }
        }
    } else {
        // Toggle between BUY_YES and BUY_NO
        use_state = (use_state == USE_STATE_t::USE_YES) ? USE_STATE_t::USE_NO : USE_STATE_t::USE_YES;
    }
    displayBar();
}

/**
 * @brief Handles the right action button press.
 */
void CPInventoryDisplay::performRightAction() {
    if(use_state == USE_STATE_t::USE_DISABLED) {
        use_state = USE_STATE_t::USE_YES;
    } else if(use_state == USE_STATE_t::USE_YES) {
        // Attempt to consume the meal
        if (CPStore::getInstance().consumeMeal(current_meal)) {
            // Use successful
            use_state = USE_STATE_t::USE_DISABLED;
        } else {
            // Not enough meals
            displayStatusText(10, "Not enough meals!");
            delay(2000);
        }
    } else {
        // Cancel using the item
        use_state = USE_STATE_t::USE_DISABLED;
    }
    displayBar();
}

/**
 * @brief Handles the boot button action.
 */
void CPInventoryDisplay::performBootAction()
{
    if(use_state == USE_STATE_t::USE_DISABLED) {
        CPInventoryDisplay::doStop();
        BMenu::enableMenu();
    } else {
        // Cancel using the item
        use_state = USE_STATE_t::USE_DISABLED;
        displayBar();
    }
}

bool CPInventoryDisplay::isActive( void )
{
    return cpinventory_state != CPINVENTORY_STATE_t::CPINVENTORY_DISPLAY_DISABLED;
}

void CPInventoryDisplay::doStart( void )
{
    // disable menu before starting
    BMenu::disableMenu();

    cpinventory_state = CPINVENTORY_STATE_t::CPINVENTORY_DISPLAY_RUNNING;

    // Find the first owned meal
    int mealCount = static_cast<int>(CPStoreMeals::MEAL_COUNT);

    for (int i = 0; i < mealCount; i++) {
        CPStoreMeals meal = static_cast<CPStoreMeals>(i);
        if (CPStore::getInstance().getMealCount(meal) > 0) {
            current_meal = meal;
            display_state = DISPLAY_STATE_t::DISPLAY_ITEMS_IN_INVENTORY;
            break;
        }
    }

    displayBar();

}

void CPInventoryDisplay::doStop( void )
{
    cpinventory_state = CPINVENTORY_STATE_t::CPINVENTORY_DISPLAY_DISABLED;

    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.display();

    // enable menu since we are done
    BMenu::enableMenu();
}
