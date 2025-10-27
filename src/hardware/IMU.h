#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>
#include "../config/Config.h"

/**
 * IMU klasse - Håndterer MPU-9250/6050 orienterings sensor
 *
 * Denne klasse læser accelerometer og gyroscope data for at
 * bestemme robottens heading (retning) og orientering.
 */
class IMU {
public:
    /**
     * Constructor
     */
    IMU();

    /**
     * Initialiserer IMU sensor
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Kalibrerer IMU sensor
     * Robot skal stå stille under kalibrering!
     */
    void calibrate();

    /**
     * Opdaterer IMU målinger
     * Kalder denne regelmæssigt i loop()
     */
    void update();

    /**
     * Hent nuværende heading (retning)
     * @return Heading i grader (0-360)
     */
    float getHeading();

    /**
     * Hent yaw (rotation omkring Z-akse)
     * @return Yaw i grader
     */
    float getYaw();

    /**
     * Hent pitch (rotation omkring Y-akse)
     * @return Pitch i grader
     */
    float getPitch();

    /**
     * Hent roll (rotation omkring X-akse)
     * @return Roll i grader
     */
    float getRoll();

    /**
     * Nulstiller heading til 0
     */
    void resetHeading();

    /**
     * Tjek om IMU er kalibreret
     * @return true hvis kalibreret
     */
    bool isCalibrated();

    /**
     * Tjek om robotten er væltet (baseret på pitch/roll)
     * @return true hvis væltet
     */
    bool isTilted();

    /**
     * Print IMU værdier til Serial (debug)
     */
    void printValues();

private:
    /**
     * Læser rå accelerometer data
     */
    void readAccelerometer();

    /**
     * Læser rå gyroscope data
     */
    void readGyroscope();

    /**
     * Beregner orientering fra sensor data
     */
    void calculateOrientation();

    /**
     * Normaliserer vinkel til 0-360 grader
     * @param angle Vinkel at normalisere
     * @return Normaliseret vinkel
     */
    float normalizeAngle(float angle);

    /**
     * Komplementær filter for sensor fusion
     * @param gyroValue Gyroscope værdi
     * @param accelValue Accelerometer værdi
     * @param alpha Filter koefficient (0-1)
     * @return Filtreret værdi
     */
    float complementaryFilter(float gyroValue, float accelValue, float alpha);

    // MPU6050/9250 I2C adresse
    const uint8_t MPU_ADDR = 0x68;

    // Register adresser
    const uint8_t PWR_MGMT_1 = 0x6B;
    const uint8_t ACCEL_XOUT_H = 0x3B;
    const uint8_t GYRO_XOUT_H = 0x43;

    // Rå sensor værdier
    int16_t accelX, accelY, accelZ;
    int16_t gyroX, gyroY, gyroZ;

    // Beregnede værdier
    float heading;
    float yaw;
    float pitch;
    float roll;

    // Kalibrerings værdier (offsets)
    float gyroXOffset;
    float gyroYOffset;
    float gyroZOffset;

    // Timing
    unsigned long lastUpdate;
    float deltaTime;

    // Tilstand
    bool initialized;
    bool calibrated;

    // Initial heading offset
    float headingOffset;
};

#endif // IMU_H
