#ifndef MPU9250_HEADING_H
#define MPU9250_HEADING_H

#include <Arduino.h>
#include <Wire.h>

class Mpu9250Heading {
public:
    // Constructor
    Mpu9250Heading();

    // Must be called in setup()
    // Returns false if sensor not found
    bool begin(TwoWire &wire = Wire,
               uint32_t i2cFrequency = 400000,
               uint8_t mpuAddress = 0x68,
               uint8_t magAddress = 0x0C);

    // Optional: quick gyro bias calibration. Call while robot is still.
    void calibrateGyro(uint16_t samples = 500);

    // Set complementary filter parameter (0..1), default 0.98
    void setAlpha(float a);

    // Set magnetic declination in radians (e.g. degrees * PI / 180)
    void setDeclination(float declRad);

    // Set magnetometer calibration values (bias in sensor units, scale factor)
    void setMagCalibration(float biasX, float biasY, float biasZ,
                           float scaleX, float scaleY, float scaleZ);

    // Call as often as possible in loop()
    // Returns false if read failed
    bool update();

    // Heading in radians (-PI .. PI)
    float getHeadingRad() const;

    // Heading in degrees (0..360)
    float getHeadingDeg() const;

    // Raw sensor access if needed
    void getRawAccel(float &ax, float &ay, float &az) const;
    void getRawGyro(float &gx, float &gy, float &gz) const;
    void getRawMag(float &mx, float &my, float &mz) const;

private:
    // I2C
    TwoWire* _wire;
    uint8_t _mpuAddr;
    uint8_t _magAddr;

    // Calibration
    float _gyroBiasX, _gyroBiasY, _gyroBiasZ;
    float _magBiasX, _magBiasY, _magBiasZ;
    float _magScaleX, _magScaleY, _magScaleZ;

    // Last measured (in physical units)
    float _ax, _ay, _az;
    float _gx, _gy, _gz;  // rad/s
    float _mx, _my, _mz;

    // Low-pass filtered accel
    float _axLPF, _ayLPF, _azLPF;

    // Filter parameters
    float _alpha;        // complementary factor
    float _accAlpha;     // accel low-pass
    float _declination;  // rad

    // Heading state
    float _heading;            // rad
    unsigned long _lastMicros; // for dt

    // Internal helpers
    bool writeRegister(uint8_t addr, uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t addr, uint8_t reg, uint8_t count, uint8_t* dest);

    bool initMPU();
    bool initMag();

    bool readAccelGyro();
    bool readMag();

    static float wrapAngle(float x);
};

#endif // MPU9250_HEADING_H