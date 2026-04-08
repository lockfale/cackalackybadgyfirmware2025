#pragma once

#include "neocontroller.h"

/*
    badge.h -- part of the cackalackabadgy2023 project.
    Declarations for everything that was generated...
*/

// Buttons
// BOOT_BUTT - which pin is the boot button connected to?
#define BOOT_BUTT  0
#define LEFT_BUTT  15
#define MIDDLE_BUTT 22
#define RIGHT_BUTT 2

/* Defaults should be fine, but you may set each to
    something appropriate for your situation.
*/

// serial_baud_rate -- connect to device at this baud rate
#define serial_baud_rate        9600

#define greeting_str    "Cacka|ackybadgy"

#define badge_version_str    "25.1.1"

#define DIE_HORRIBLY(msg)    for(;;){ Serial.println(msg); delay(1000); }
