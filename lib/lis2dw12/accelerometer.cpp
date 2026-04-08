#include <Wire.h>
#include <Arduino.h>
#include "accelerometer.h"
#include "haptic.h"


static int ADXLAddress = 0x19; // Device address
static int16_t __range_d = 0;
static float __range = 0.061;

static uint16_t tickIndex = 0;
bool BadgeAcceler::tapInterruptAttached = false;
unsigned long BadgeAcceler::lastAccelerDetachTime = 0;
unsigned long BadgeAcceler::freeFallStartTime = 0;
unsigned long BadgeAcceler::freeFallEndTime = 0;

/*****************************************
 *
 * Constants; direct ports from the BACCELER namespace
 *
 *****************************************/
constexpr uint8_t ACCEL_CHECK_TIME_INTERVAL = 150;
constexpr uint8_t REG_CARD_ID = 0x0F; // hex value pointing to WHO_AM_I register, should always return 0x44
constexpr uint8_t REG_CTRL_REG1 = 0x20;        //Control register 1
constexpr uint8_t REG_CTRL_REG2 = 0x21;        //Control register 2
constexpr uint8_t REG_CTRL_REG3 = 0x22;        //Control register 3
constexpr uint8_t REG_CTRL_REG4 = 0x23;        //Control register 4
constexpr uint8_t REG_CTRL_REG5 = 0x24;        //Control register 5
constexpr uint8_t REG_CTRL_REG6 = 0x25;        //Control register 6
constexpr uint8_t REG_CTRL_REG7 = 0x3F;        //Control register 7
constexpr uint8_t REG_OUT_T = 0x26;            //Temperature output register
constexpr uint8_t REG_OUT_T_L = 0x0D;          //Temperature output register in 12 bit resolution
constexpr uint8_t REG_OUT_T_H = 0x0E;          //Temperature output register in 12 bit resolution
constexpr uint8_t REG_STATUS_REG = 0x27;       //Status register
constexpr uint8_t REG_OUT_X_L = 0x28;          //The low order of the X-axis acceleration register
constexpr uint8_t REG_OUT_X_H = 0x29;          //The high point of the X-axis acceleration register
constexpr uint8_t REG_OUT_Y_L = 0x2A;          //The low order of the Y-axis acceleration register
constexpr uint8_t REG_OUT_Y_H = 0x2B;          //The high point of the Y-axis acceleration register
constexpr uint8_t REG_OUT_Z_L = 0x2C;          //The low order of the Z-axis acceleration register
constexpr uint8_t REG_OUT_Z_H = 0x2D;          //The high point of the Z-axis acceleration register
constexpr uint8_t REG_WAKE_UP_DUR = 0x35;      //Wakeup and sleep duration configuration register (r/w).
constexpr uint8_t REG_FREE_FALL = 0x36;        //Free fall event register
constexpr uint8_t REG_STATUS_DUP = 0x37;       //Interrupt event status register
constexpr uint8_t REG_WAKE_UP_SRC = 0x38;      //Wakeup source register
constexpr uint8_t REG_TAP_SRC = 0x39;          //Tap source register
constexpr uint8_t REG_SIXD_SRC = 0x3A;         //6D source register
constexpr uint8_t REG_ALL_INT_SRC = 0x3B;      //Reading this register, all related interrupt function flags routed to the INT pads are reset simultaneously
constexpr uint8_t REG_TAP_THS_X = 0x30;        //4D configuration enable and TAP threshold configuration .
constexpr uint8_t REG_TAP_THS_Y = 0x31;        //Threshold for tap recognition @ FS = ±2 g on Y direction
constexpr uint8_t REG_TAP_THS_Z = 0x32;        //Threshold for tap recognition @ FS = ±2 g on Z direction
constexpr uint8_t REG_INT_DUR = 0x33;          //Interrupt duration register
constexpr uint8_t REG_WAKE_UP_THS = 0x34;      //Wakeup threshold register
constexpr uint8_t SPI_READ_BIT = 0x80;         // bit 0: RW bit. When 0, the data DI(7:0) is written into the device. When 1, the data DO(7:0) from the device is read.
constexpr uint8_t ID = 0x44;

/*****************************************
 *
 * Power Mode
 *
 *****************************************/
constexpr uint8_t HIGH_PERFORMANCE_14BIT = 0x04;     //High-Performance Mode
constexpr uint8_t CONT_LOWPWR4_14BIT = 0x03;         //Continuous measurement,Low-Power Mode 4(14-bit resolution)
constexpr uint8_t CONT_LOWPWR3_14BIT = 0x02;         //Continuous measurement,Low-Power Mode 3(14-bit resolution)
constexpr uint8_t CONT_LOWPWR2_14BIT = 0x01;         //Continuous measurement,Low-Power Mode 2(14-bit resolution)
constexpr uint8_t CONT_LOWPWR1_12BIT = 0x00;         //Continuous measurement,Low-Power Mode 1(12-bit resolution)
constexpr uint8_t SING_LELOWPWR4_14BIT = 0x0B;       //Single data conversion on demand mode,Low-Power Mode 4(14-bit resolution)
constexpr uint8_t SING_LELOWPWR3_14BIT = 0x0A;       //Single data conversion on demand mode,Low-Power Mode 3(14-bit resolution
constexpr uint8_t SING_LELOWPWR2_14BIT = 0x09;       //Single data conversion on demand mode,Low-Power Mode 2(14-bit resolution)
constexpr uint8_t SING_LELOWPWR1_12BIT = 0x08;       //Single data conversion on demand mode,Low-Power Mode 1(12-bit resolution)
constexpr uint8_t HIGHP_ERFORMANCELOW_NOISE_14BIT = 0x14;   //High-Performance Mode,Low-noise enabled
constexpr uint8_t CONT_LOWPWRLOWNOISE4_14BIT = 0x13;        //Continuous measurement,Low-Power Mode 4(14-bit resolution,Low-noise enabled)
constexpr uint8_t CONT_LOWPWRLOWNOISE3_14BIT = 0x12;        //Continuous measurement,Low-Power Mode 3(14-bit resolution,Low-noise enabled)
constexpr uint8_t CONT_LOWPWRLOWNOISE2_14BIT = 0x11;        //Continuous measurement,Low-Power Mode 2(14-bit resolution,Low-noise enabled)
constexpr uint8_t CONT_LOWPWRLOWNOISE1_12BIT = 0x10;        //Continuous measurement,Low-Power Mode 1(14-bit resolution,Low-noise enabled)
constexpr uint8_t SINGLE_LOWPWRLOWNOISE4_14BIT = 0x1B;      //Single data conversion on demand mode,Low-Power Mode 4(14-bit resolution),Low-noise enabled
constexpr uint8_t SINGLE_LOWPWRLOWNOISE3_14BIT = 0x1A;      //Single data conversion on demand mode,Low-Power Mode 3(14-bit resolution),Low-noise enabled
constexpr uint8_t SINGLE_LOWPWRLOWNOISE2_14BIT = 0x19;      //Single data conversion on demand mode,Low-Power Mode 2(14-bit resolution),Low-noise enabled
constexpr uint8_t SINGLE_LOWPWRLOWNOISE1_12BIT = 0x18;      //Single data conversion on demand mode,Low-Power Mode 1(12-bit resolution),Low-noise enabled

/*****************************************
 *
 * Sensor Range
 *
 *****************************************/
constexpr uint8_t RANGE_2G = 2;    // +/-2g
constexpr uint8_t RANGE_4G = 4;    // +/-4g
constexpr uint8_t RANGE_8G = 8;    // +/-8g
constexpr uint8_t RANGE_16G = 16;  // +/-16g

/*****************************************
 *
 * Filtering mode
 *
 *****************************************/
constexpr uint8_t LPF = 0x00;    //Low pass filter
constexpr uint8_t HPF = 0x10;    //High pass filter

/*****************************************
 *
 * bandwidth of collected data
 *
 *****************************************/
constexpr uint8_t RATE_DIV_2 = 0;    //RATE/2 (up to RATE = 800 Hz, 400 Hz when RATE = 1600 Hz)
constexpr uint8_t RATE_DIV_4 = 1;    //RATE/4 (High Power/Low Power)
constexpr uint8_t RATE_DIV_10 = 2;   //RATE/10 (HP/LP)
constexpr uint8_t RATE_DIV_20 = 3;   //RATE/20 (HP/LP)

/*****************************************
 *
 * Data collection rate
 *
 *****************************************/
constexpr uint8_t RATE_OFF = 0x00;      //Measurement off
constexpr uint8_t RATE1HZ6 = 0x01;      //1.6Hz, use only under low-power mode
constexpr uint8_t RATE12HZ5 = 0x02;     //12.5Hz
constexpr uint8_t RATE25HZ = 0x03;      //25Hz
constexpr uint8_t RATE50HZ = 0x04;      //50Hz
constexpr uint8_t RATE100HZ = 0x05;     //100Hz
constexpr uint8_t RATE200HZ = 0x06;     //200Hz
constexpr uint8_t RATE400HZ = 0x07;     //400Hz, Use only under high-performance mode
constexpr uint8_t RATE800HZ = 0x08;     //800Hz, Use only under high-performance mode
constexpr uint8_t RATE1600HZ = 0x09;    //1600Hz, Use only under high-performance mode
constexpr uint8_t SETSWTRIG = 0x12;             //The software triggers a single measurement

/*****************************************
 *
 * Motion detection mode
 *
 *****************************************/
constexpr uint8_t NO_DETECTION = 0;      //No detection
constexpr uint8_t DETECT_ACT = 1;        //Detect movement
constexpr uint8_t DETECT_STATMOTION = 3; //Detect motion

/*****************************************
 *
 * Interrupt source 1 trigger event setting
 *
 *****************************************/
constexpr uint8_t DOUBLE_TAP = 0x08;  //Double tap event
constexpr uint8_t FREE_FALL = 0x10;   //Free fall event
constexpr uint8_t WAKEUP = 0x20;      //Wakeup event
constexpr uint8_t SINGLE_TAP = 0x40;  //Single tap event
constexpr uint8_t IA6D = 0x80;        //An event changed the status of facing up/down/left/right/forward/back

/*****************************************
 *
 * Interrupt source 2 trigger event setting
 *
 *****************************************/
constexpr uint8_t SLEEP_STATE = 0x40;  //Sleep change status routed to INT2 pad
constexpr uint8_t SLEEP_CHANGE = 0x80; //Enable routing of SLEEP_STATE on INT2 pad

/*****************************************
 *
 * tap or double tap
 *
 *****************************************/
constexpr uint8_t S_TAP = 0;    //single tap
constexpr uint8_t D_TAP = 1;    //double tap
constexpr uint8_t NO_TAP = 2;    //no tap
constexpr uint8_t HAP_TAP = 3;   //haptic-driven tap

// which direction is tap event detected
constexpr uint8_t DIR_X_UP = 0;      //Tap event generated in the positive X direction
constexpr uint8_t DIR_X_DOWN = 1;    //Tap event generated in the negative X direction
constexpr uint8_t DIR_Y_UP = 2;      //Tap event generated in the positive Y direction
constexpr uint8_t DIR_Y_DOWN = 3;    //Tap event generated in the negative Y direction
constexpr uint8_t DIR_Z_UP = 4;      //Tap event generated in the positive Z direction
constexpr uint8_t DIR_Z_DOWN = 5;    //Tap event generated in the negative Z direction

// which direction is wake up event detected
constexpr uint8_t DIR_X = 0;    //Motion in the X direction woke up the chip
constexpr uint8_t DIR_Y = 1;    //Motion in the Y direction woke up the chip
constexpr uint8_t DIR_Z = 2;    //Motion in the Z direction woke up the chip

constexpr uint8_t ERROR = 0xFF;

// tap detection mode
constexpr uint8_t ONLY_SINGLE = 0;        //Only single tap events detected
constexpr uint8_t BOTH_SINGLE_DOUBLE = 1; //Both single-tap and double-tap events detected

/*****************************************
 *
 * Position detection
 *
 *****************************************/
constexpr uint8_t DEGREES_80 = 0;    //80 degrees.
constexpr uint8_t DEGREES_70 = 1;    //70 degrees.
constexpr uint8_t DEGREES_60 = 2;    //60 degrees.
constexpr uint8_t DEGREES_50 = 3;    //50 degrees.

// orientation
constexpr uint8_t X_DOWN = 0;    //X is now down
constexpr uint8_t X_UP = 1;      //X is now up
constexpr uint8_t Y_DOWN = 2;    //Y is now down
constexpr uint8_t Y_UP = 3;      //Y is now up
constexpr uint8_t Z_DOWN = 4;    //Z is now down
constexpr uint8_t Z_UP = 5;      //Z is now up

/**************************************************************
 * Register      : CTRL1
 * Address       : 0x20
 * Bit Group Name: ODR
 * Permission    : RW
 **************************************************************/
enum class ODR_t {
    ODR_POWER_DOWN = 0x00,
    ODR_12_5_1_6HZ = 0x10,
    ODR_12_5Hz = 0x20,
    ODR_25Hz = 0x30,
    ODR_50Hz = 0x40,
    ODR_100Hz = 0x50,
    ODR_200Hz = 0x60,
    ODR_400_200Hz = 0x70,
    ODR_800_200Hz = 0x80,
    ODR_1600_200Hz = 0x90
};

/**************************************************************
 * Register      : CTRL2
 * Address       : 0x21
 * Bit Group Name: BDU
 * Permission    : RW
 **************************************************************/
constexpr uint8_t BDU_CONTINUOUS_UPDATE = 0x00;
constexpr uint8_t BDU_NOT_UPDATE_MSB_LSB = 0x08;

/**************************************************************
 * Register      : CTRL2
 * Address       : 0x21
 * Bit Group Name: SOFT_RESET
 * Permission    : RW
 **************************************************************/
constexpr uint8_t SOFT_RESET_DISABLE = 0x00;
constexpr uint8_t SOFT_RESET_ENABLE = 0x40;

/**************************************************************
 * Register      : CTRL6
 * Address       : 0x25
 * Bit Group Name: LOW_NOISE
 * Permission    : RW
 **************************************************************/
enum class LOW_NOISE_t {
    LOW_NOISE_DISABLE = 0x00,
    LOW_NOISE_ENABLE = 0x04
};

/**************************************************************
 * Register      : CTRL6
 * Address       : 0x25
 * Bit Group Name: FS
 * Permission    : RW
 **************************************************************/
constexpr int8_t FS_2G = 0x00;
constexpr int8_t FS_4G = 0x10;
constexpr int8_t FS_8G = 0x20;
constexpr int8_t FS_16G = 0x30;

/**************************************************************
 * Register      : TAP_THS_X
 * Address       : 0x30
 * Bit Group Name: 6D_THS
 * Permission    : RW
 **************************************************************/
enum class BGN_6D_THS_t {
    BGN_6D_THS_0 = 0x00,
    BGN_6D_THS_11 = 0x20,
    BGN_6D_THS_16 = 0x40,
    BGN_6D_THS_21 = 0x60
};

/**************************************************************
 * Register      : TAP_THS_X
 * Address       : 0x30
 * Bit Group Name: 4D_EN
 * Permission    : RW
 **************************************************************/
enum class BGN_4D_EN_t {
    BGN_4D_EN_DISABLE = 0x00,
    BGN_4D_EN_PORTRAIT_LANDSCAPE = 0x80
};

/**************************************************************
 * Register      : TAP_THS_Z
 * Address       : 0x32
 * Bit Group Name: TAP_Z_EN
 * Permission    : RW
 **************************************************************/
enum class TAP_Z_EN_t {
    TAP_Z_EN_DISABLE = 0x00,
    TAP_Z_EN_ENABLE = 0x20
};

/**************************************************************
 * Register      : TAP_THS_Z
 * Address       : 0x32
 * Bit Group Name: TAP_Y_EN
 * Permission    : RW
 **************************************************************/
enum class TAP_Y_EN_t {
    TAP_Y_EN_DISABLE = 0x00,
    TAP_Y_EN_ENABLE = 0x40
};

/**************************************************************
 * Register      : TAP_THS_Z
 * Address       : 0x32
 * Bit Group Name: TAP_X_EN
 * Permission    : RW
 **************************************************************/
enum class TAP_X_EN_t {
    TAP_X_EN_DISABLE = 0x00,
    TAP_X_EN_ENABLE = 0x80
};


auto BadgeAcceler::getInstance() -> BadgeAcceler& {
    static BadgeAcceler instance; // Static local variable
    return instance;
}

auto BadgeAcceler::isTapInterruptAttached() -> bool {
    return tapInterruptAttached;
}

auto BadgeAcceler::getLastAccelerDetachTime() -> unsigned long {
    return lastAccelerDetachTime;
}

void BadgeAcceler::setLastAccelerDetachTime(unsigned long time) {
    lastAccelerDetachTime = time;
}

void BadgeAcceler::setTapInterruptAttached(bool value) {
    tapInterruptAttached = value;
}

uint8_t BadgeAcceler::getInterruptStatus( void ) {
    return readRegister(REG_STATUS_DUP);
}


auto BadgeAcceler::accelCheckTime(uint16_t duration) -> bool {
    static unsigned long lastCheckTime = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime >= duration) {
        lastCheckTime = currentTime;
        return true;
    }
    return false;
}

auto BadgeAcceler::readRegister(uint8_t reg) -> uint8_t {
    Wire.beginTransmission(ADXLAddress);
    Wire.write(reg | SPI_READ_BIT);
    Wire.endTransmission();
    Wire.requestFrom(ADXLAddress, 1);
    return Wire.read();
}

auto BadgeAcceler::readRegisterInt16(uint8_t offset) -> int16_t {
    uint8_t result1 = readRegister(offset);
    uint8_t result2 = readRegister(offset + 1);
    return (int16_t)result1 | (int16_t(result2) << 8);
}

void BadgeAcceler::writeRegister(int reg, int data) {
    Wire.beginTransmission(ADXLAddress);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

// *************************************************************
//
// Accelerometer section
//
// *************************************************************

//
// calcAccel
// Adjst the raw acceleration value by the measurement range
//
// param input : raw acceleration data
// return adjusted acceleration data
//
auto BadgeAcceler::calcAccel(int16_t input) -> float {
    return (float(input) * __range);
}

    //
// readRawAccelX
// Read the raw acceleration value in the x direction
//
// return raw acceleration data from x
//
auto BadgeAcceler::readRawAccelX() -> int16_t {
    return readRegisterInt16(REG_OUT_X_L);
}

//
// readFloatAccelX
// Calculate acceleration data from x(mg) adjusted by the measurement range
//
// Return adjusted acceleration data from x(mg)
//
auto BadgeAcceler::readFloatAccelX() -> float {
    return calcAccel(readRawAccelX());
}

//
// readRawAccelY
// Read the raw acceleration value in the y direction
//
// return raw acceleration data from y
//
auto BadgeAcceler::readRawAccelY() -> int16_t {
    return readRegisterInt16(REG_OUT_Y_L);
}

//
// readFloatAccelY
// Calculate acceleration data from y(mg) adjusted by the measurement range
//
// Return adjusted acceleration data from y(mg)
//
auto BadgeAcceler::readFloatAccelY() -> float {
    return calcAccel(readRawAccelY());
}

//
// readRawAccelZ
// Read the raw acceleration value in the z direction
//
// return raw acceleration data from z
//
auto BadgeAcceler::readRawAccelZ() -> int16_t {
    return readRegisterInt16(REG_OUT_Z_L);
}

//
// readFloatAccelZ
// Calculate acceleration data from z(mg) adjusted by the measurement range
//
// Return adjusted acceleration data from z(mg)
//
auto BadgeAcceler::readFloatAccelZ() -> float {
    return calcAccel(readRawAccelZ());
}

// *************************************************************
//
//  Temperature section
//
//  REG_OUT_T = 0x26;            //Temperature output register (8bit)
//  REG_OUT_T_L = 0x0D;          //Temperature output register in 12 bit resolution
//  REG_OUT_T_H = 0x0E;          //Temperature output register in 12 bit resolution
//**************************************************************

// Low-resolution temperature functions
auto BadgeAcceler::readTempRaw8() -> int8_t {
    return readRegister(REG_OUT_T);
}

// Celcius increments by 1 in low-res mode
auto BadgeAcceler::getTempLoResCelcius() -> int8_t {
    int8_t rawTemp = static_cast<int8_t>(readTempRaw8());
    return rawTemp + 25;
}

//Farenheit increments by 2 in low-res mode
auto BadgeAcceler::getTempLoResFarenheit() -> int8_t {
    // Convert the Celsius temperature to Fahrenheit
    int8_t tempC = getTempLoResCelcius();
    return static_cast<int8_t>(tempC * 1.8 + 32);
}


// High-resolution temperature functions
auto BadgeAcceler::readTempRaw12() -> int16_t {
    uint8_t loByte = readRegister(REG_OUT_T_L);
    uint8_t hiByte = readRegister(REG_OUT_T_H);

    // Extract the high nibble from the low byte
    uint8_t lsbNibble = (loByte >> 4) & 0x0F;
    // Combine the high byte and the low byte's high nibble, sign it
    int16_t temp12 = (static_cast<uint16_t>(hiByte) << 4) | lsbNibble;
        // Check if the 12th bit (sign bit) is set
        if (temp12 & 0x0800) {
            // Sign-extend to 16 bits by filling the high nibble with 1s
            temp12 |= 0xF000;
        }
    return temp12;
}

auto BadgeAcceler::getTempHiResCelcius() -> float {
    int16_t rawTemp = readTempRaw12();
    float temp12C = (rawTemp * 0.0625) + 25; // 0.0625 degrees Celsius per LSB, 1/16
    return temp12C;
}

auto BadgeAcceler::getTempHiResFarenheit() -> float {
    // Convert the Celsius temperature to Fahrenheit
    float tempC = getTempHiResCelcius();
    return (tempC * 1.8) + 32;
}

auto BadgeAcceler::getTempHiResKelvin() -> float {
    // Convert the Celsius temperature to Kelvin
    float tempC = getTempHiResCelcius();
    return tempC + 273.15;
}

// *************************************************************
//
// Free-fall / Wakeup detection section
//
// *************************************************************

//
// REG_FREE_FALL = 0x36;        //Free fall event register
// Set to detect free fall (drop/launch) events
//
// param: enable
//            true - Enable freefall detection
//            false - disable freefall detection
//
void BadgeAcceler::enableFreeFallDetection(bool enable) {
    uint8_t value = readRegister(REG_FREE_FALL);
    if (enable) {
        value |= 0x20; // Enable free-fall detection
    } else {
        value &= ~0x20; // Disable free-fall detection
    }
    writeRegister(REG_FREE_FALL, value);
}

bool BadgeAcceler::isFreeFallDetected() {
    uint8_t status = readRegister(REG_WAKE_UP_SRC);
    if ((status & 0x20) > 0) { // Free-fall event detected
        if (freeFallStartTime == 0) {
            freeFallStartTime = millis(); // Record the start time
        }
        return true;
    } else {
        if (freeFallStartTime != 0) {
            freeFallEndTime = millis(); // Record the end time
        }
        return false;
    }
}

float BadgeAcceler::calculateFreeFallDistance() {
    if (freeFallStartTime == 0 || freeFallEndTime == 0) {
        return 0.0f; // No valid freefall event
    }

    // Get the ODR value to derive the freefall trigger duration, calculate the duration of the freefall in milliseconds
    uint8_t ODR_value = (readRegister(REG_CTRL_REG1) >> 4) & 0x0F;
    // Get the freefall trigger duration and convert it to milliseconds
    // The trigger duration is in multiples of 1/ODR
    uint8_t ffTriggerDurationMs = (((readRegister(REG_FREE_FALL) >> 3) & 0x1F) * 1000.0f) / ODR_value;
    // Measured start time will be late, so subtract the trigger duration from it to approximate a true start time
    float freeFallDurationSeconds = (freeFallEndTime - (freeFallStartTime - ffTriggerDurationMs)) / 1000.0f;

    // Reset the freefall times for the next event
    freeFallStartTime = 0;
    freeFallEndTime = 0;

    // Use the formula d = 0.5 * g * t^2
    const float gravity = 9.8f; // Acceleration due to gravity in m/s^2
    return 0.5f * gravity * freeFallDurationSeconds * freeFallDurationSeconds;
}

void BadgeAcceler::setFreeFallConfig(uint8_t threshold, uint8_t duration) {
    // REG_FREE_FALL (0x36) is an 8-bit register used to configure free-fall detection.
    // Bits [2:0] (lower 3 bits) represent the free-fall threshold in increments of 31.25 mg.
    // Bits [7:3] (upper 5 bits) represent the free-fall duration in multiples of 1/ODR.

    // Ensure the threshold and duration values are within valid ranges
    threshold &= 0x07; // Mask to ensure only the lower 3 bits are used
    duration &= 0x1F;  // Mask to ensure only the upper 5 bits are used

    // Combine the duration and threshold into a single byte
    uint8_t configValue = (duration << 3) | threshold;

    // Write the combined value to the REG_FREE_FALL register
    writeRegister(REG_FREE_FALL, configValue);
}


void BadgeAcceler::enableWakeUpDetection(bool enable) {
    if (enable) {
        // Enable wake-up detection
        uint8_t value = readRegister(REG_WAKE_UP_THS);
        value |= 0x80; // Set the wake-up detection bit
        writeRegister(REG_WAKE_UP_THS, value);
    } else {
        // Disable wake-up detection
        uint8_t value = readRegister(REG_WAKE_UP_THS);
        value = 0x00; // Clear the wake-up detection bit
        writeRegister(REG_WAKE_UP_THS, value);
    }
}

// *************************************************************
//
// TAP detection section
//
// *************************************************************

//
// enable_tap_detection_on_x
// Set to detect tap events in the X direction
//
// param: enable
//            true - Enable tap detection
//            false - disable tap detection
//
auto BadgeAcceler::enableTapDetectionOnX(bool enable) -> void {
    uint8_t result = readRegister(REG_TAP_THS_X);
    result &= (~(1<<7));
    result |= (enable << 7);
    writeRegister(REG_TAP_THS_X, result);
}

//
// enable_tap_detection_on_y
// Set to detect tap events in the Y direction
//
// param: enable
//            true - Enable tap detection
//            false - disable tap detection
//
auto BadgeAcceler::enableTapDetectionOnY(bool enable) -> void {
    uint8_t result = readRegister(REG_TAP_THS_Y);
    result &= (~(1<<6));
    result |= (enable << 6);
    writeRegister(REG_TAP_THS_Y, result);
}

//
// enable_tap_detection_on_z
// Set to detect tap events in the Z direction
//
// param: enable
//            true - Enable tap detection
//            false - disable tap detection
//
auto BadgeAcceler::enableTapDetectionOnZ(bool enable) -> void {
    uint8_t result = readRegister(REG_TAP_THS_Z);
    result &= (~(1<<5));
    result |= (enable << 5);
    writeRegister(REG_TAP_THS_Z, result);
}

auto BadgeAcceler::setTapThresholdOnX(float th) -> void {
    uint8_t th1 = (float(th)/__range_d) * 32;
    uint8_t result = readRegister(REG_TAP_THS_X);
    result = result & 0xE0;
    result = result | (int(th1) & 0x1f);
    writeRegister(REG_TAP_THS_X,result);
}
auto BadgeAcceler::setTapThresholdOnY(float th) -> void {
    uint8_t th1 = (float(th)/__range_d) * 32;
    uint8_t result = readRegister(REG_TAP_THS_Y);
    result = result & (~0x1f);
    result = result | (int(th1) & 0x1f);
    writeRegister(REG_TAP_THS_Y,result);
}
auto BadgeAcceler::setTapThresholdOnZ(float th) -> void {
    uint8_t th1 = (float(th)/__range_d) * 32;
    uint8_t result = readRegister(REG_TAP_THS_Z);
    result = result & (~0x1f);
    result = result | (int(th1) & 0x1f);
    writeRegister(REG_TAP_THS_Z,result);
}
auto BadgeAcceler::setTapQuiet(int quiet) -> void {
    uint8_t result = readRegister(REG_INT_DUR);
    result = result & (~0x0C);
    quiet = quiet & 0x03;
    result = result | (quiet<<2);
    writeRegister(REG_INT_DUR,result);
}
auto BadgeAcceler::setTapShock(int shock) -> void {
    uint8_t result = readRegister(REG_INT_DUR);
    result = result & (~0x03);
    shock = shock & 0x03;
    result = result | (shock);
    writeRegister(REG_INT_DUR,result);
}
auto BadgeAcceler::setTapDuration(int dur) -> void {
    uint8_t result = readRegister(REG_INT_DUR);
    result = result & (~0xf0);
    result = result | (dur << 4);
    writeRegister(REG_INT_DUR, result);
    setTapQuiet(2);
    setTapShock(2);
}
auto BadgeAcceler::setTapMode(int mode) -> void {
    uint8_t result = readRegister(REG_WAKE_UP_THS);
    result = result & (~0x80);
    result = result | (mode << 7);
    writeRegister(REG_WAKE_UP_THS,result);
}
auto BadgeAcceler::tapDetect() -> int8_t {
    uint8_t result = readRegister(REG_TAP_SRC);
    uint8_t tap = NO_TAP;
    if (BadgeHaptic::getInstance().isHapticOn()) {
        tap = HAP_TAP;
    } else if ((result & 0x20) > 0) {
        tap = S_TAP;
    } else if ((result & 0x10) > 0) {
        tap = D_TAP;
    }
    return tap;
}
auto BadgeAcceler::getTapDirection() -> int8_t {
    uint8_t result = readRegister(REG_TAP_SRC);
    uint8_t direction = ERROR;
    uint8_t positive = result & 0x08;
    if ((result & 0x4)>0 && positive > 0) {
        direction = DIR_X_UP;
    } else if ((result & 0x4)>0 && positive == 0) {
        direction = DIR_X_DOWN;
    } else if ((result & 0x2)>0 && positive > 0) {
        direction = DIR_Y_UP;
    } else if ((result & 0x2)>0 && positive == 0) {
        direction = DIR_Y_DOWN;
    } else if ((result & 0x1)>0 && positive > 0) {
        direction = DIR_Z_UP;
    } else if ((result & 0x1)>0 && positive == 0) {
        direction = DIR_Z_DOWN;
    }
    return direction;
}

//
// set_int1_event
// Set the interrupt source of the int1 pin
//
// Param event : Several interrupt events, after setting, when an event is
//               generated, a level transition will be generated on the int1 pin
//            DOUBLE_TAP    #Double tap event
//            FREEFALL      #Freefall event
//            WAKEUP        #Wake-up event
//            SINGLE_TAP    #Single tap event
//            IA6D          #An event changed the status of facing
//                                up/down/left/right/forward/back
//
auto BadgeAcceler::setInt1Event(uint8_t event) -> void {
    uint8_t value1 = readRegister(REG_CTRL_REG4);
    uint8_t value3 = readRegister(REG_CTRL_REG7);
    value3 &= (~0x20);
    value3 |= 0x20;
    value1 |= event;
    writeRegister(REG_CTRL_REG4, value1);
    writeRegister(REG_CTRL_REG7, value3);
}

//
// set_int2_event
// Select the interrupt event generated on the int2 pin
//
// Param event  Several interrupt events, after setting, when an event
//                    is generated, a level transition will be generated
//                    on the int2 pin
//            SLEEP_CHANGE  #Enable routing of SLEEP_STATE on INT2 pad
//            SLEEP_STATE   #0x80 Sleep change status routed to INT2 pad
//

auto BadgeAcceler::setInt2Event(uint8_t event) -> void {
    uint8_t value1 = readRegister(REG_CTRL_REG4);
    uint8_t value2 = readRegister(REG_CTRL_REG5);
    uint8_t value3 = readRegister(REG_CTRL_REG7);
    value3 &= (~0x20);
    value3 |= 0x20;
    value1 |= event;
    writeRegister(REG_CTRL_REG5, value2);
    writeRegister(REG_CTRL_REG7, value3);
}

// *************************************************************
//
// Config Section
//
// *************************************************************

//
// soft_reset
// Software reset to restore the value of all registers to the default value
//
auto BadgeAcceler::softReset() -> void {
    uint8_t value = readRegister(REG_CTRL_REG2);
    value |= SOFT_RESET_ENABLE;
    writeRegister(REG_CTRL_REG2, value);
}

//
// set_range
// Set the measurement range
//
// Param range_r : range
//                RANGE_2G     #±2g
//                RANGE_4G     #±4g
//                RANGE_8G     #±8g
//                RANGE_16G    #±16g
//
auto BadgeAcceler::setRange(uint8_t range_r) -> void {
    uint8_t result = readRegister(REG_CTRL_REG6);
    __range_d = range_r;
    result = result & 0xCF; // Zero out FS bits
    if (range_r == RANGE_2G) {
        result |= FS_2G;
        __range = 0.061;
    } else if (range_r == RANGE_4G) {
        result |= FS_4G;
        __range = 0.122;
    } else if (range_r == RANGE_8G) {
        result |= FS_8G;
        __range = 0.244;
    } else if (range_r == RANGE_16G) {
        result |= FS_16G;
        __range = 0.488;
    }
    writeRegister(REG_CTRL_REG6, result);
}

//
// contin_refresh
// Choose whether to continuously let the chip collect data
//
// Param enable : enable
//            true(continuous update)
//            false( output registers not updated until MSB and LSB read)
//
auto BadgeAcceler::continRefresh(bool enable) -> void {
    uint8_t result = readRegister(REG_CTRL_REG2);
    result = result & (~(1<<3)); // Zero out BDU bit
    // Since we zeroed out the bit, we set BDU_CONTINUOUS_UPDATE
    if (enable == false) {
        result |= BDU_NOT_UPDATE_MSB_LSB;
    }
    writeRegister(REG_CTRL_REG2, result);
}

//
// set_data_rate
// Set data measurement rate
//
// Param rate : rate
//            RATE_OFF          #Measurement off
//            RATE_1HZ6         #1.6hz, use only under low-power mode
//            RATE_12HZ5        #12.5hz
//            RATE_25HZ
//            RATE_50HZ
//            RATE_100HZ
//            RATE_200HZ
//            RATE_400HZ        #Use only under High-Performance mode
//            RATE_800HZ        #Use only under High-Performance mode
//            RATE_1600HZ        #Use only under High-Performance mode
//            SETSWTRIG         #The software triggers a single measurement
//
auto BadgeAcceler::setDataRate(uint8_t rate) -> void {
    uint8_t result = readRegister(REG_CTRL_REG1);
    result = result & 0x0F; // Zero out ODR bits
    result |= (rate << 4);
    writeRegister(REG_CTRL_REG1, result);

    // I don't know why this is even here honestly.
    //uint8_t enable = (rate & 0x30) >> 4;
    //result = readRegister(REG_CTRL_REG3);
    //result &= (~3); // Zero out SLP_MODE_SEL and SLP_MODE_1 bits
    //result |= enable;
    //write_reg(REG_CTRL_REG3,result);
}

//
// set_filter_path
// Set the filter processing mode
//
// Param path : path of filtering
//            #define LPF 0x00    //Low pass filter
//          #define HPF 0x10    //High pass filter
//
auto BadgeAcceler::setFilterPath(uint8_t path) -> void {
    //Set filter processing mode
    uint8_t result = readRegister(REG_CTRL_REG6);
    uint8_t enable = path & 0x10;
    if (enable > 0) {
        enable = 1;
    }
    result = result & 0xF7;
    result |= (enable << 3);
    writeRegister(REG_CTRL_REG6, result);

    result = readRegister(REG_CTRL_REG7);
    result = result & 0xEF;
    result |= (enable << 4);
    writeRegister(REG_CTRL_REG7, result);
}

//
// set_power_mode
// Set power mode
//
// Param mode : 16 power modes to choose from
//                HIGH_PERFORMANCE_14BIT          #High-Performance Mode
//                CONT_LOWPWR4_14BIT              #Continuous measurement,Low-Power Mode 4(14-bit resolution)
//                CONT_LOWPWR3_14BIT              #Continuous measurement,Low-Power Mode 3(14-bit resolution)
//                CONT_LOWPWR2_14BIT              #Continuous measurement,Low-Power Mode 2(14-bit resolution)
//                CONT_LOWPWR1_12BIT              #Continuous measurement,Low-Power Mode 1(12-bit resolution)
//                SING_LELOWPWR4_14BIT            #Single data conversion on demand mode,Low-Power Mode 4(14-bit resolution)
//                SING_LELOWPWR3_14BIT            #Single data conversion on demand mode,Low-Power Mode 3(14-bit resolution
//                SING_LELOWPWR2_14BIT            #Single data conversion on demand mode,Low-Power Mode 2(14-bit resolution)
//                SING_LELOWPWR1_12BIT            #Single data conversion on demand mode,Low-Power Mode 1(12-bit resolution)
//                HIGHP_ERFORMANCELOW_NOISE_14BIT #High-Performance Mode,Low-noise enabled
//                CONT_LOWPWRLOWNOISE4_14BIT      #Continuous measurement,Low-Power Mode 4(14-bit resolution,Low-noise enabled)
//                CONT_LOWPWRLOWNOISE3_14BIT      #Continuous measurement,Low-Power Mode 3(14-bit resolution,Low-noise enabled)
//                CONT_LOWPWRLOWNOISE2_14BIT      #Continuous measurement,Low-Power Mode 2(14-bit resolution,Low-noise enabled)
//                CONT_LOWPWRLOWNOISE1_12BIT      #Continuous measurement,Low-Power Mode 1(14-bit resolution,Low-noise enabled)
//                SINGLE_LOWPWRLOWNOISE4_14BIT    #Single data conversion on demand mode,Low-Power Mode 4(14-bit resolution),Low-noise enabled
//                SINGLE_LOWPWRLOWNOISE3_14BIT    #Single data conversion on demand mode,Low-Power Mode 3(14-bit resolution),Low-noise enabled
//                SINGLE_LOWPWRLOWNOISE2_14BIT    #Single data conversion on demand mode,Low-Power Mode 2(14-bit resolution),Low-noise enabled
//                SINGLE_LOWPWRLOWNOISE1_12BIT    #Single data conversion on demand mode,Low-Power Mode 1(12-bit resolution),Low-noise enabled
//
auto BadgeAcceler::setPowerMode(uint8_t mode) -> void {
    uint8_t result = readRegister(REG_CTRL_REG1);
    result = result & (~0x0F); // Zero out PM bits
    result |= (mode & 0x0F);
    writeRegister(REG_CTRL_REG1, result);

    int16_t enable = mode >> 4;
    result = readRegister(REG_CTRL_REG6);
    result &= (~(1 << 2)); // Zero out MODE bits and LP_MODE bits
    // Since we zeroed out the bit, we set LOW_NOISE_DISABLE
    //If enable == 1 then LOW_NOISE_ENABLE
    result |= (enable << 2);
    writeRegister(REG_CTRL_REG6, result);
}

void BadgeAcceler::setup( void ) {
    Wire.begin();
    delay(100);

    Wire.beginTransmission(ADXLAddress); //start communicating with the accelerometer
    Wire.write(REG_CARD_ID); // tell the accelerometer to access 0x0f
    Wire.endTransmission(); // tell the accelerometer were done

    Wire.requestFrom(ADXLAddress, 1); // request 1 byte from the accelerometer

    //int16_t readCheck = 0;
    //if anything is available
    //if (Wire.available()) {
    //    readCheck = Wire.read(); // Reads the byte int X0
    //}
    //Serial.println("BACCELER: The following value should be 0x44 (dec 68).");
    //Serial.print("BACCELER: readCheck= "); //print out the value of x0 should always be 0x44 (dec 68)
    //Serial.println(readCheck);

    softReset();
    setDataRate(RATE100HZ);  // Originally RATE400HZ
    setRange(RANGE_2G);

    continRefresh(true);
    setFilterPath(LPF);
    setPowerMode(HIGH_PERFORMANCE_14BIT);

    // Disable wakeup events, that might be in clonflict with freefall events
    enableWakeUpDetection(false);

    // Enable freefall detection with increased sensitivity
    enableFreeFallDetection(true);

    //writeRegister(0x22, 0x10); //set latch
    // Freefall example: file:///home/wain/Downloads/dt0100-setting-up-freefall-recognition-with-sts-mems-accelerometers-stmicroelectronics.pdf
    writeRegister(0x30, 0x0C); //set tap threshold x // Freefall example write 0x95 to this reg
    writeRegister(0x31, 0xEC); //set tap threshold y
    writeRegister(0x32, 0xEC); //set tap threshold z // Freefall example write 0x16 to this reg
    writeRegister(0x33, 0x7F); //set duration, quiet, and shock time window // Freefall example write 0x03 to this reg
    writeRegister(0x34, 0x80); //single and double tap enable
    setFreeFallConfig(0x00, 0x0F); // Reg 0x36, Set threshold to XXX125 mg and duration to 20 ms (Assuming 400HZ ODR)
    writeRegister(0x23, 0x58); //free-fall, single tap, and double tap sent to int1 pin (0x48 for Single+Double tap)
    writeRegister(0x3F, 0x20); //enable interrupts
}

void BadgeAcceler::runAccel( void ) {
    tickIndex++;
    /*
    int8_t tapstatus = BACCELER::tap_detect();
    if (tapstatus == 1) {
        #ifdef BADGEDEBUG
            Serial.println("ACCEL: double tap");
        #endif // BADGEDEBUG
            } else if ( tapstatus == 0) {
        #ifdef BADGEDEBUG
            Serial.println("ACCEL: single tap");
        #endif // BADGEDEBUG
    }
*/
    if (tickIndex % 100 == 0) {
        //Serial.println("ACCEL: Tick*100");
        /*
        int16_t X0 = readFloatAccelX();
        int16_t Y0 = readFloatAccelY();
        int16_t Z0 = readFloatAccelZ();
        Serial.println("\nAccelerometer:");
        Serial.print("  X= ");
        Serial.print(X0);
        Serial.print("  Y= ");
        Serial.print(Y0);
        Serial.print("  Z= ");
        Serial.println(Z0);
        Serial.println("Thermometer:");
        Serial.print(" Degrees C = ");
        Serial.println(readTempCLowRes());
        Serial.print(" Degrees F = ");
        Serial.println(readTempFLowRes());
        Serial.print(" Tap Status = ");
        Serial.println(tap_detect());
        */
    }
}

void BadgeAcceler::update( void ){
    if (accelCheckTime(ACCEL_CHECK_TIME_INTERVAL)) {
        runAccel();
    }
}

void BadgeAcceler::taskCallback() {
    update();
}
