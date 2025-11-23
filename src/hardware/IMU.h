#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>
#include "../config/Config.h"

/**
 * IMU klasse - Håndterer MPU-9250 orienterings sensor med magnetometer
 *
 * Denne klasse læser accelerometer, gyroscope og magnetometer data for at
 * bestemme robottens heading (retning) og orientering.
 *
 * Heading beregnes via sensor fusion:
 * - Magnetometer giver absolut heading reference
 * - Gyroscope giver hurtig respons
 * - Complementary filter fusionerer de to
 * - Forberedt for fremtidig encoder-fusion
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
     * Kalibrerer gyroscope (robot skal stå stille!)
     * @param samples Antal samples til kalibrering (default 500)
     */
    void calibrateGyro(uint16_t samples = 500);

    /**
     * Sæt magnetometer kalibrerings værdier (hard/soft iron)
     * @param biasX, biasY, biasZ Hard iron offset
     * @param scaleX, scaleY, scaleZ Soft iron scale faktorer
     */
    void setMagCalibration(float biasX, float biasY, float biasZ,
                           float scaleX, float scaleY, float scaleZ);

    /**
     * Sæt magnetisk deklination for lokation
     * @param declDeg Deklination i grader (positiv = øst)
     */
    void setDeclination(float declDeg);

    /**
     * Sæt complementary filter alpha værdi
     * @param alpha Filter koefficient (0-1), højere = mere gyro, default 0.98
     */
    void setAlpha(float alpha);

    /**
     * Opdaterer IMU målinger og beregner heading
     * Skal kaldes regelmæssigt i loop()
     * @return true hvis opdatering lykkedes
     */
    bool update();

    /**
     * Hent nuværende heading (retning)
     * @return Heading i grader (0-360)
     */
    float getHeading();

    /**
     * Hent heading i radianer
     * @return Heading i radianer (-PI til PI)
     */
    float getHeadingRad();

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
     * Tjek om magnetometer er tilgængeligt
     * @return true hvis magnetometer fundet
     */
    bool hasMagnetometer();

    /**
     * Tjek om robotten er væltet (baseret på pitch/roll)
     * @return true hvis væltet
     */
    bool isTilted();

    /**
     * Print IMU værdier til Serial (debug)
     */
    void printValues();

    /**
     * Hent rå sensor værdier (debug/kalibrering)
     */
    void getRawAccel(float &ax, float &ay, float &az);
    void getRawGyro(float &gx, float &gy, float &gz);
    void getRawMag(float &mx, float &my, float &mz);

    // ========== Encoder Interface (forberedt til fremtidig brug) ==========

    /**
     * Opdater heading fra encoder odometri
     * Kan bruges til sensor fusion når encodere er installeret
     * @param encoderHeadingDeg Heading beregnet fra encodere i grader
     * @param confidence Tillid til encoder måling (0-1)
     */
    void fuseEncoderHeading(float encoderHeadingDeg, float confidence);

    /**
     * Aktiver/deaktiver encoder fusion
     * @param enabled true for at aktivere encoder fusion
     */
    void setEncoderFusionEnabled(bool enabled);

private:
    // ========== I2C Kommunikation ==========
    bool writeRegister(uint8_t addr, uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t addr, uint8_t reg, uint8_t count, uint8_t* dest);

    // ========== Sensor Initialisering ==========
    bool initMPU();
    bool initMagnetometer();
    bool detectMagnetometer();

    // ========== Sensor Læsning ==========
    bool readAccelGyro();
    bool readMagnetometer();

    // ========== Beregninger ==========
    void calculateOrientation();
    float wrapAngle(float angle);      // Wrap til -PI..PI
    float normalizeAngle(float angle); // Normalize til 0..360

    // ========== I2C Adresser ==========
    static const uint8_t MPU_ADDR = 0x68;
    static const uint8_t MAG_ADDR = 0x0C;  // AK8963

    // ========== MPU-9250 Register Adresser ==========
    static const uint8_t PWR_MGMT_1    = 0x6B;
    static const uint8_t SMPLRT_DIV    = 0x19;
    static const uint8_t CONFIG_REG    = 0x1A;
    static const uint8_t GYRO_CONFIG   = 0x1B;
    static const uint8_t ACCEL_CONFIG  = 0x1C;
    static const uint8_t ACCEL_CONFIG2 = 0x1D;
    static const uint8_t INT_PIN_CFG   = 0x37;
    static const uint8_t ACCEL_XOUT_H  = 0x3B;

    // ========== AK8963 Magnetometer Register Adresser ==========
    static const uint8_t AK8963_ST1   = 0x02;
    static const uint8_t AK8963_HXL   = 0x03;
    static const uint8_t AK8963_CNTL1 = 0x0A;
    static const uint8_t AK8963_CNTL2 = 0x0B;

    // ========== Rå Sensor Værdier (fysiske enheder) ==========
    float _ax, _ay, _az;          // Accelerometer i g
    float _gx, _gy, _gz;          // Gyroscope i rad/s
    float _mx, _my, _mz;          // Magnetometer i µT

    // ========== Low-pass Filtrerede Accelerometer Værdier ==========
    float _axLPF, _ayLPF, _azLPF;
    float _accAlpha;              // Accel LPF koefficient (default 0.1)

    // ========== Gyroscope Kalibrering ==========
    float _gyroBiasX, _gyroBiasY, _gyroBiasZ;

    // ========== Magnetometer Kalibrering ==========
    float _magBiasX, _magBiasY, _magBiasZ;     // Hard iron offset
    float _magScaleX, _magScaleY, _magScaleZ;  // Soft iron scale
    float _declination;                         // Magnetisk deklination i rad

    // ========== Beregnede Orienteringsværdier ==========
    float _heading;               // Fused heading i radianer
    float _pitch;                 // Pitch i radianer
    float _roll;                  // Roll i radianer

    // ========== Filter Parametre ==========
    float _alpha;                 // Complementary filter koefficient (default 0.98)

    // ========== Timing ==========
    unsigned long _lastMicros;

    // ========== Tilstand ==========
    bool _initialized;
    bool _gyroCalibrated;
    bool _magnetometerAvailable;
    float _headingOffset;

    // ========== Encoder Fusion (forberedt) ==========
    bool _encoderFusionEnabled;
    float _encoderHeading;
    float _encoderConfidence;

    // ========== Konstanter ==========
    static constexpr float TILT_THRESHOLD = 45.0f;  // Grader
};

#endif // IMU_H
