#include <Arduino.h>

namespace QRCODE {
    const  uint8_t bmMsg0[] PROGMEM = {
        0b10001100,
        0b10010010,
        0b10010010,
        0b10010010,
        0b01100010,
    };

    const  uint8_t bmMsg1[] PROGMEM = {
        0b00000100,
        0b00100010,
        0b00100010,
        0b00100010,
        0b00011100,
    };

    const  uint8_t bmMsg2[] PROGMEM = {
        0b00011110,
        0b00101010,
        0b00101010,
        0b00101010,
        0b00000100,
    };

    const  uint8_t bmMsg3[] PROGMEM = {
        0b00011110,
        0b00100000,
        0b00100000,
        0b00010000,
        0b00111110,
    };

    const  uint8_t bmMsg4[] PROGMEM = {
        0b00011110,
        0b00100000,
        0b00011000,
        0b00100000,
        0b00111110,
    };

    const  uint8_t bmMsg5[] PROGMEM = {
        0b00011000,
        0b00101010,
        0b00101010,
        0b00101010,
        0b00011100,
    };

    const uint8_t* const qrMsgArray[] PROGMEM = {
                bmMsg0, bmMsg1, bmMsg2, bmMsg3, bmMsg4, bmMsg5};
}