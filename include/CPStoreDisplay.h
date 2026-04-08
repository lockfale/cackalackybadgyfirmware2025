/**
 * @file CPStoreDisplay.h
 * @brief Defines the CPStoreDisplay class, which displays the CPStore on the display.
 *
 * This file declares the CPStoreDisplay singleton.
 */
#pragma once

#include "CPStore.h"

class CPStoreDisplay
{
public:
    /**
     * @brief Retrieves the singleton instance of the CPStoreDisplay class.
     *
     * This function provides access to the single instance of the `CPStoreDisplay` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `CPStoreDisplay` instance.
     *
     * @details
     * - The `CPStoreDisplay` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * CPStoreDisplay& storeInstance = CPStoreDisplay::getInstance();
     * storeInstance.doStart();
     * ```
     */
    static CPStoreDisplay& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    CPStoreDisplay(const CPStoreDisplay&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    CPStoreDisplay& operator=(const CPStoreDisplay&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    CPStoreDisplay(CPStoreDisplay&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    CPStoreDisplay& operator=(CPStoreDisplay&&) = delete;

    /**
     * @brief Displays the amount of money that the CP has on the screen.
     */
    static void displayMoney();

    /**
     * @brief Displays the status bar for the CPStore.
     */
    static void displayBar();

    static void displayBuyState();

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
     * @brief Check if the CPStoreDisplay is running.
     * @return True if running, false otherwise.
     */
    static bool isActive();

	/**
     * @brief Start the CPStoreDisplay.
     */
    static void doStart();

    /**
     * @brief Stop the CPStoreDisplay.
     */
    static void doStop();

    protected:
    /**
     * @brief Default destructor for the CPStoreDisplay class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~CPStoreDisplay() = default;

private:
    /**
     * @brief Default constructor for the CPStoreDisplay class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    CPStoreDisplay() = default;

    static CPStoreMeals current_meal;

    /**
     * @enum CPSTORE_STATE_t
     * @brief Enumeration of QR code generation states.
     */
	enum class CPSTORE_STATE_t {
        CPSTORE_DISPLAY_DISABLED = 0, ///< CPStoreDisplay is disabled
        CPSTORE_DISPLAY_RUNNING = 1   ///< CPStoreDisplay is running
    };
    static CPSTORE_STATE_t cpstore_state;

    /**
     * @enum BUY_STATE_t
     * @brief Enumeration of QR code generation states.
     */
	enum class BUY_STATE_t {
        BUY_DISABLED,
        BUY_YES,
        BUY_NO
    };
    static BUY_STATE_t buy_state;
};
