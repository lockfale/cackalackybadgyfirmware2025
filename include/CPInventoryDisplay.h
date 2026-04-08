/**
 * @file CPInventoryDisplay.h
 * @brief Defines the CPInventoryDisplay class, which displays the CPStore on the display.
 *
 * This file declares the CPInventoryDisplay singleton.
 */
#pragma once

#include "CPStore.h"

class CPInventoryDisplay
{
public:
    /**
     * @brief Retrieves the singleton instance of the CPInventoryDisplay class.
     *
     * This function provides access to the single instance of the `CPInventoryDisplay` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `CPInventoryDisplay` instance.
     *
     * @details
     * - The `CPInventoryDisplay` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * CPInventoryDisplay& inventoryInstance = CPInventoryDisplay::getInstance();
     * inventoryInstance.doStart();
     * ```
     */
    static CPInventoryDisplay& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    CPInventoryDisplay(const CPInventoryDisplay&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    CPInventoryDisplay& operator=(const CPInventoryDisplay&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    CPInventoryDisplay(CPInventoryDisplay&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    CPInventoryDisplay& operator=(CPInventoryDisplay&&) = delete;

    /**
     * @brief Displays the amount of money that the CP has on the screen.
     */
    static void displayMoney();

    /**
     * @brief Displays the status bar for the CPStore.
     */
    static void displayBar();

    static void displayUseState();

    /**
     * @brief Displays the current meal on the screen.
     */
    static void displayMeal();

    /**
     * @brief Displays text on the screen at the specified location.
     * @param locX The x-coordinate of the text's location.
     * @param text The text to display.
     */
    static void displayStatusText(uint16_t locX, const char* text);

        /**
     * @brief Handles the left button action.
     */
    static void performLeftAction();

    /**
     * @brief Handles the middle button action.
     */
    static void performMiddleAction();

    /**
     * @brief Handles the right button action.
     */
    static void performRightAction();

    /**
     * @brief Handles the boot button action.
     */
    static void performBootAction();

    /**
     * @brief Check if the CPInventoryDisplay is running.
     * @return True if running, false otherwise.
     */
    static bool isActive();

	/**
     * @brief Start the CPInventoryDisplay.
     */
    static void doStart();

    /**
     * @brief Stop the CPInventoryDisplay.
     */
    static void doStop();

protected:
    /**
     * @brief Default destructor for the CPInventoryDisplay class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~CPInventoryDisplay() = default;

private:
    /**
     * @brief Default constructor for the CPInventoryDisplay class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    CPInventoryDisplay() = default;

    static CPStoreMeals current_meal;

    /**
     * @enum CPINVENTORY_STATE_t
     * @brief Enumeration of QR code generation states.
     */
	enum class CPINVENTORY_STATE_t {
        CPINVENTORY_DISPLAY_DISABLED = 0, ///< CPInventoryDisplay is disabled
        CPINVENTORY_DISPLAY_RUNNING = 1   ///< CPInventoryDisplay is running
    };
    static CPINVENTORY_STATE_t cpinventory_state;

    /**
     * @enum DISPLAY_STATE_t
     * @brief Enumeration of QR code generation states.
     */
	enum class DISPLAY_STATE_t {
        DISPLAY_NO_ITEMS_IN_INVENTORY,
        DISPLAY_ITEMS_IN_INVENTORY,
        DISPLAY_NO_MORE_ITEM
    };
    static DISPLAY_STATE_t display_state;

    /**
     * @enum USE_STATE_t
     * @brief State to determine if user wants to use item.
     */
	enum class USE_STATE_t {
        USE_DISABLED,
        USE_YES,
        USE_NO
    };
    static USE_STATE_t use_state;
};
