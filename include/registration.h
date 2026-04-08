/**
 * @file registration.h
 * @brief Defines the Registration class, which initially registers badge and then checks for iButton after every power cycle.
 *
 * This file declares the Registration singleton, its attributes, and methods.
 */
#pragma once
#include <array>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
//#define REGISTRATIONDEBUG // uncomment to enable registration debugging
#endif

/**
 * @class Registration
 * @brief Manages the attributes and state of the Registration.
 *
 * This class follows a singleton pattern to ensure only one instance exists.
 *
 * Features:
 * - Tracks the Registration's state and registration code.
 */
class Registration {
public:
	/**
	 * @brief Retrieves the singleton instance of the Registration class.
	 *
	 * This function provides access to the single instance of the `Registration` class,
	 * ensuring that only one instance exists throughout the program's lifecycle.
	 *
	 * @return Reference to the singleton `Registration` instance.
	 *
	 * @details
	 * - The `Registration` class follows the singleton design pattern to ensure that
	 *   only one instance of the class is created and shared.
	 * - This function is thread-safe and guarantees that the instance is initialized
	 *   only once.
	 *
	 * @example
	 * ```cpp
	 * Registration& reg = Registration::getInstance();
	 * reg.setup();
	 * ```
	 */
	static Registration& getInstance();

	/**
	 * @brief Deleted copy constructor to prevent copying of the singleton instance.
	 */
	Registration(const Registration&) = delete;

	/**
	 * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
	 */
	Registration& operator=(const Registration&) = delete;

	/**
	 * @brief Deleted move constructor to prevent moving of the singleton instance.
	 */
	Registration(Registration&&) = delete;

	/**
	 * @brief Deleted move assignment operator to prevent moving of the singleton instance.
	 */
	Registration& operator=(Registration&&) = delete;

	/**
     * @brief Check if the Registration is running.
     * @return True if running, false otherwise.
     */
    static bool isActive();

	static bool isRegistered();

	static void unRegisterBadge();

	static void setButtonID(const std::array<uint8_t, 8>& key);
	static std::array<uint8_t, 8> getButtonID();
	static void setMasterKey(const std::array<uint8_t, 8>& key);
	static std::array<uint8_t, 8> getMasterKey();


	/**
     * @brief Start Registration.
     */
    static void doStart();

    /**
     * @brief Stop Registration.
     */
    static void doStop();

	static void discordRegistration();

protected:
	/**
	 * @brief Default destructor for the Registration class.
	 *
	 * The destructor is declared as `protected` to prevent direct deletion of the
	 * singleton instance. This ensures that the lifecycle of the singleton is
	 * managed internally by the class.
	 */
	~Registration() = default;

private:
	/**
	 * @brief Default constructor for the Registration class.
	 *
	 * The constructor is declared as `private` to enforce the singleton design
	 * pattern, ensuring that no additional instances of the class can be created
	 * outside of the `getInstance()` method.
	 */
	Registration() = default;

	/**
     * @enum REGISTRATION_STATE_t
     * @brief Enumeration of QR code generation states.
     */
	enum class REGISTRATION_STATE_t {
        REGISTRATION_DISABLED = 0, ///< Registration is disabled
        REGISTRATION_RUNNING = 1,  ///< Registration is running
		DISCORD_REGISTRATION = 2   ///< Discord registration is running
    };
    static REGISTRATION_STATE_t reg_state;

	enum class DisplayState {
		NeedsRegistration,
		FinishedRegistration,
		ShowAlreadyRegistered,
		VerificationFailed,
		FinishVerify,
	};

	static bool isReg;
	static std::array<uint8_t, 8> buttonID;
	static std::array<uint8_t, 8> masterKey;

	static void displayMessage(const __FlashStringHelper* message);
	static void showDisplayState(DisplayState state);

	static void registerBadge();

	static void verifyButton();
};