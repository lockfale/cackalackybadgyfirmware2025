/**
 * @file qrcode.h
 * @brief Header file for the QRCode class.
 *
 * This file contains the declaration of the QRCode class, which is a singleton
 * class designed to manage QR code generation. The class provides various methods
 * to create and display QR codes on the badge display. It supports generating QR codes
 * for specific URLs and managing the QR code generation state.
 *
 * The file also includes necessary macro definitions and library inclusions required for
 * the QRCode class to function correctly.
 *
 * @note Uncomment the QRDEBUG macro to enable debugging.
 */
#pragma once

#include <Arduino.h>
#include "master_debug.h"

#ifdef MASTER_BADGE_DEFINE
//#define QRDEBUG    // uncomment to enable neo debugging
#endif

/**
 * @class QRCode
 * @brief Singleton class to manage QR code generation.
 *
 * The QRCode class provides methods to create and display QR codes on the badge display.
 * It supports generating QR codes for specific URLs and managing the QR code generation state.
 */
class QRCode {
public:
	/**
     * @brief Get the singleton instance of QRCode.
     * @return Reference to the singleton instance.
     */
    static QRCode& getInstance();

    /**
     * @brief Create a QR code with the given message.
     * @param message The message to encode in the QR code.
     */
    static void create(const char* message);

    /**
     * @brief Create a QR code for Melvin.
     */
    static void createQRCodeMelvin();

    /**
     * @brief Create a QR code for Pandatrax.
     */
    static void createQRCodePandatrax();

    /**
     * @brief Create a QR code for Persinac.
     */
    static void createQRCodePersinac();

    /**
     * @brief Create a QR code for Stress.
     */
    static void createQRCodeStress();

    /**
     * @brief Create a QR code for Clarke Hackworth.
     */
    static void createQRCodeClarkeHackworth ();

	/**
     * @brief Check if the QR code generation is running.
     * @return True if running, false otherwise.
     */
    static bool isRunning();

	/**
     * @brief Start the QR code generation.
     */
    static void doStart();

    /**
     * @brief Stop the QR code generation.
     */
    static void doStop();

private:
	QRCode() = default;
	QRCode(const QRCode&) = delete;
	QRCode& operator=(const QRCode&) = delete;

	/**
     * @enum QR_STATE_t
     * @brief Enumeration of QR code generation states.
     */
	enum class QR_STATE_t {
        QR_DISABLED = 0, ///< QR code generation is disabled
        QR_RUNNING = 1   ///< QR code generation is running
    };
    static QR_STATE_t qrcode_state;

	static constexpr int offsetsX = 42; /**< X offset for QR code rendering */
    static constexpr int offsetsY = 10; /**< Y offset for QR code rendering */
    static constexpr int screenwidth = 128; /**< Screen width */
    static constexpr int screenheight = 64; /**< Screen height */

	/**
     * @brief Render a pixel on the display.
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Color of the pixel (1 for black, 0 for white)
     */
	static void render(int x, int y, int color);

	/**
	 * @brief Display the QR code on the badge display.
	 */
	static void screenwhite();
};