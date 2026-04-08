#pragma once

#include <stdint.h>

class BadgeAcceler {
public:
    /**
     * @brief Retrieves the singleton instance of the BadgeAcceler class.
     *
     * This function provides access to the single instance of the `BadgeAcceler` class,
     * ensuring that only one instance exists throughout the program's lifecycle.
     *
     * @return Reference to the singleton `BadgeAcceler` instance.
     *
     * @details
     * - The `BadgeAcceler` class follows the singleton design pattern to ensure that
     *   only one instance of the class is created and shared.
     * - This function is thread-safe and guarantees that the instance is initialized
     *   only once.
     *
     * @example
     * ```cpp
     * BadgeAcceler& accelInstance = BadgeAcceler::getInstance();
     * accelInstance.setup();
     * ```
     */
    static BadgeAcceler& getInstance();

    /**
     * @brief Deleted copy constructor to prevent copying of the singleton instance.
     */
    BadgeAcceler(const BadgeAcceler&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying of the singleton instance.
     */
    BadgeAcceler& operator=(const BadgeAcceler&) = delete;

    /**
     * @brief Deleted move constructor to prevent moving of the singleton instance.
     */
    BadgeAcceler(BadgeAcceler&&) = delete;

    /**
     * @brief Deleted move assignment operator to prevent moving of the singleton instance.
     */
    BadgeAcceler& operator=(BadgeAcceler&&) = delete;

    static void setup();
    static void update();
    void taskCallback();

    static int8_t tapDetect();
    static float readFloatAccelX();
    static float readFloatAccelY();
    static float readFloatAccelZ();

    static int8_t getTempLoResCelcius( void );
    static int8_t getTempLoResFarenheit( void );

    static float getTempHiResCelcius( void );
    static float getTempHiResFarenheit( void );
    static float getTempHiResKelvin( void );

    static bool isTapInterruptAttached();
    static unsigned long getLastAccelerDetachTime();
    static void setLastAccelerDetachTime(unsigned long time);
    static void setTapInterruptAttached(bool value);
    static uint8_t getInterruptStatus( void );

    static bool isFreeFallDetected( void );
    static float calculateFreeFallDistance( void );

    // Tap detection helpers
    static int8_t getTapDirection();

protected:
    /**
     * @brief Default destructor for the BadgeAcceler class.
     *
     * The destructor is declared as `protected` to prevent direct deletion of the
     * singleton instance. This ensures that the lifecycle of the singleton is
     * managed internally by the class.
     */
    ~BadgeAcceler() = default;

private:
    /**
     * @brief Default constructor for the BadgeAcceler class.
     *
     * The constructor is declared as `private` to enforce the singleton design
     * pattern, ensuring that no additional instances of the class can be created
     * outside of the `getInstance()` method.
     */
    BadgeAcceler() = default;

    static bool tapInterruptAttached;
    static unsigned long lastAccelerDetachTime;

    static unsigned long freeFallStartTime;
    static unsigned long freeFallEndTime;


    // Private member functions
    static int16_t readRegisterInt16(uint8_t offset);
    static uint8_t readRegister(uint8_t reg);
    static void writeRegister(int reg, int data);

    static void runAccel();
    static float calcAccel(int16_t input);
    static bool accelCheckTime(uint16_t duration);


    // Raw acceleration readers
    static int16_t readRawAccelX( void );
    static int16_t readRawAccelY( void );
    static int16_t readRawAccelZ( void );

    // Raw Temperature readers
    static int8_t readTempRaw8( void );
    static int16_t readTempRaw12( void );

    // Tap/freefall/wakeup detection helpers
    static void enableTapDetectionOnX(bool enable);
    static void enableTapDetectionOnY(bool enable);
    static void enableTapDetectionOnZ(bool enable);
    static void enableFreeFallDetection(bool enable);
    static void enableWakeUpDetection(bool enable);

    // Configuration methods
    static void setInt1Event(uint8_t event);
    static void setInt2Event(uint8_t event);

    static void continRefresh(bool enable);
    static void softReset();
    static void setRange(uint8_t range);
    static void setDataRate(uint8_t rate);
    static void setFilterPath(uint8_t path);
    static void setPowerMode(uint8_t mode);

    static void setTapThresholdOnX(float th);
    static void setTapThresholdOnY(float th);
    static void setTapThresholdOnZ(float th);
    static void setTapQuiet(int quiet);
    static void setTapShock(int shock);
    static void setTapDuration(int dur);
    static void setTapMode(int mode);
    static void setFreeFallConfig(uint8_t threshold, uint8_t duration);
};