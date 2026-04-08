/**
 * @file qrcode.cpp
 * @brief Implementation file for the QRCode class.
 *
 * This file contains the implementation of the QRCode class, which is a singleton
 * class designed to manage QR code generation. The class provides various methods
 * to create and display QR codes on the badge display. It supports generating QR codes
 * for specific URLs and managing the QR code generation state.
 *
 * The file includes the definitions of the methods declared in the qrcode.h header file.
 * It also initializes static member variables and sets up the task scheduler for handling
 * timed events and background processing.
 */
#include <Arduino.h>

#include "badge_display.h"
#include "badgeMenu.h"
#include "neocontroller.h"
#include "qrcode.h"
#include "qrcode_bitmaps.h"
#include "qrencode.h"

QRCode::QR_STATE_t QRCode::qrcode_state = QRCode::QR_STATE_t::QR_DISABLED;

QRCode& QRCode::getInstance() {
    static QRCode instance;
    return instance;
}

void QRCode::render(int x, int y, int color){
    x=x+offsetsX;
    y=y+offsetsY;
    if(color==1) {
        BDISPLAY::display.drawPixel(x, y, BLACK);
    } else {
        BDISPLAY::display.drawPixel(x, y, WHITE);
    }
}

void QRCode::screenwhite(){
    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.fillRect(0,0,BDISPLAY::SCREEN_WIDTH, BDISPLAY::SCREEN_HEIGHT, WHITE);
    BDISPLAY::display.display();
}

void QRCode::create(const char* message) {
    uint8_t textStart = BDISPLAY::SCREEN_HEIGHT - 15;

    // create QR code
    strncpy((char *)strinbuf, message, 260);
    qrencode();
    screenwhite();

    #ifdef QRDEBUG
    Serial.println("QRcode render");
    Serial.println();
    #endif

    for (uint8_t i = 0; i < 6; i++) {
        BDISPLAY::display.drawBitmap(
                10, textStart - i * 8,
                QRCODE::qrMsgArray[i], 8, 5,
                BLACK);
    }

    // print QR Code
    for (byte x = 0; x < WD; x+=2) {
        for (byte y = 0; y < WD; y++) {
            if ( QRBIT(x,y) &&  QRBIT((x+1),y)) {
                // black square on top of black square
                #ifdef QRDEBUG
                Serial.print("@");
                #endif
                render(x, y, 1);
                render((x+1), y, 1);
            }
            if (!QRBIT(x,y) &&  QRBIT((x+1),y)) {
                // white square on top of black square
                #ifdef QRDEBUG
                Serial.print(" ");
                #endif
                render(x, y, 0);
                render((x+1), y, 1);
            }
            if ( QRBIT(x,y) && !QRBIT((x+1),y)) {
                // black square on top of white square
                #ifdef QRDEBUG
                Serial.print("@");
                #endif
                render(x, y, 1);
                render((x+1), y, 0);
            }
            if (!QRBIT(x,y) && !QRBIT((x+1),y)) {
                // white square on top of white square
                #ifdef QRDEBUG
                Serial.print(" ");
                #endif
                render(x, y, 0);
                render((x+1), y, 0);
            }
        }
        #ifdef QRDEBUG
        Serial.println();
        #endif
    }
    BDISPLAY::display.display();
}

void QRCode::doStart( void )
{
    // disable menu before starting
    BMenu::disableMenu();

    NeoController::getInstance().changeBlue();
    qrcode_state = QR_STATE_t::QR_RUNNING;
    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.display();
}

void QRCode::doStop( void )
{
    qrcode_state = QR_STATE_t::QR_DISABLED;

    BDISPLAY::display.clearDisplay();
    BDISPLAY::display.display();

    // enable menu since we are done
    BMenu::enableMenu();
}

void QRCode::createQRCodeMelvin( void )
{
    doStart();
    create("https://github.com/melvin2001");
}

void QRCode::createQRCodePandatrax( void )
{
    doStart();
    create("https://github.com/pandatrax");
}

void QRCode::createQRCodePersinac( void )
{
    doStart();
    create("https://github.com/persinac");
}

void QRCode::createQRCodeStress( void )
{
    doStart();
    create("https://github.com/working-as-designed");
}

void QRCode::createQRCodeClarkeHackworth( void )
{
    doStart();
    create("https://github.com/clarkehackworth");
}

bool QRCode::isRunning( void )
{
    return qrcode_state != QR_STATE_t::QR_DISABLED;
}
