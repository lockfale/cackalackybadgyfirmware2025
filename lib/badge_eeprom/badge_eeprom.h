/**
 * @file badge_eeprom.h
 * @brief Defines the bEeprom class, which manages any reads or writes from the badge eeprom.
 *
 * This file declares the bEeprom singleton, its attributes, and methods.
 */
#pragma once

#include <Arduino.h>
#include <array>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
#define EEPROMDEBUG // uncomment to enable eeprom debugging
#endif

/**
 * @class Eeprom
 * @brief Manages the attributes and state of the bEeprom.
 *
 * This class follows a singleton pattern to ensure only one instance exists.
 *
 * Features:
 * - Tracks the bEeprom's money and purchased items.
 */
class bEeprom {
public:
    /**
     * @brief Retrieves the singleton instance of the bEeprom class.
     *
     * This function provides access to the single instance of the `bEeprom` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `bEeprom` instance.
     *
     * @details
     * - The `bEeprom` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * bEeprom& beepromInstance = bEeprom::getInstance();
     * beepromInstance.setup();
     * ```
     */
    static bEeprom& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    bEeprom(const bEeprom&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    bEeprom& operator=(const bEeprom&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    bEeprom(bEeprom&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    bEeprom& operator=(bEeprom&&) = delete;

    /**
     * @brief Initializes the bEeprom.
     *
     * This function sets up the bEeprom for use, including any necessary
     * configurations or initializations.
     */
    bool setup();

    void registerBadge();
    void unRegisterBadge();

    bool isBadgeRegistered();

    /**
     * @brief Writes the iButton address to EEPROM.
     *
     * This function writes the iButton address to the EEPROM at a specified
     * address. The address is expected to be in the form of a byte array.
     *
     * @param address Pointer to the iButton address to be written.
     * @param size Size of the address to be written.
     */
    void writeiButtonAddressToEEPROM(const std::array<uint8_t, 8>& address);

    /**
     * @brief Reads the iButton address from EEPROM.
     *
     * This function reads the iButton address from the EEPROM and stores it in the provided
     * address buffer.
     *
     * @param address Pointer to the buffer where the iButton address will be stored.
     * @param size Size of the buffer.
     */
    std::array<uint8_t, 8> readiButtonAddressFromEEPROM();

protected:
    /**
     * @brief Default destructor for the bEeprom class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~bEeprom() = default;

private:
    /**
     * @brief Default constructor for the bEeprom class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    bEeprom() = default;

    void debugPrintObfuscatediButtonAddress();

};