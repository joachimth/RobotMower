#include "Mpu9250Heading.h"
#include <math.h>

// MPU-9250 registers
static const uint8_t MPU_REG_PWR_MGMT_1   = 0x6B;
static const uint8_t MPU_REG_SMPLRT_DIV   = 0x19;
static const uint8_t MPU_REG_CONFIG       = 0x1A;
static const uint8_t MPU_REG_GYRO_CONFIG  = 0x1B;
static const uint8_t MPU_REG_ACCEL_CONFIG = 0x1C;
static const uint8_t MPU_REG_ACCEL_CONFIG2= 0x1D;
static const uint8_t MPU_REG_INT_PIN_CFG  = 0x37;
static const uint8_t MPU_REG_ACCEL_XOUT_H = 0x3B;

// AK8963 registers
static const uint8_t AK8963_REG_ST1   = 0x02;
static const uint8_t AK8963_REG_HXL   = 0x03;
static const uint8_t AK8963_REG_CNTL1 = 0x0A;

Mpu9250Heading::Mpu9250Heading()
: _wire(nullptr),
  _mpuAddr(0x68),
  _magAddr(0x0C),
  _gyroBiasX(0), _gyroBiasY(0), _gyroBiasZ(0),
  _magBiasX(0), _magBiasY(0), _magBiasZ(0),
  _magScaleX(1), _magScaleY(1), _magScaleZ(1),
  _ax(0), _ay(0), _az(0),
  _gx(0), _gy(0), _gz(0),
  _mx(0), _my(0), _mz(0),
  _axLPF(0), _ayLPF(0), _azLPF(1),
  _alpha(0.98f),
  _accAlpha(0.1f),
  _declination(0.0f),
  _heading(0.0f),
  _lastMicros(0)
{
}

bool Mpu9250Heading::begin(TwoWire &wire, uint32_t i2cFrequency,
                           uint8_t mpuAddress, uint8_t magAddress)
{
    _wire = &wire;
    _mpuAddr = mpuAddress;
    _magAddr = magAddress;

    _wire->begin();
    _wire->setClock(i2cFrequency);

    if (!initMPU()) return false;
    if (!initMag()) return false;

    _lastMicros = micros();
    return true;
}

void Mpu9250Heading::calibrateGyro(uint16_t samples)
{
    float sx = 0, sy = 0, sz = 0;

    for (uint16_t i = 0; i < samples; i++) {
        if (!readAccelGyro()) continue;
        sx += _gx;
        sy += _gy;
        sz += _gz;
        delay(2);
    }

    _gyroBiasX = sx / samples;
    _gyroBiasY = sy / samples;
    _gyroBiasZ = sz / samples;
}

void Mpu9250Heading::setAlpha(float a)
{
    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;
    _alpha = a;
}

void Mpu9250Heading::setDeclination(float declRad)
{
    _declination = declRad;
}

void Mpu9250Heading::setMagCalibration(float biasX, float biasY, float biasZ,
                                       float scaleX, float scaleY, float scaleZ)
{
    _magBiasX = biasX;
    _magBiasY = biasY;
    _magBiasZ = biasZ;

    _magScaleX = scaleX;
    _magScaleY = scaleY;
    _magScaleZ = scaleZ;
}

bool Mpu9250Heading::update()
{
    unsigned long now = micros();
    float dt = (now - _lastMicros) * 1e-6f;
    if (dt <= 0.0f || dt > 1.0f) {
        dt = 0.01f; // fallback
    }
    _lastMicros = now;

    if (!readAccelGyro()) return false;
    if (!readMag()) return false;

    // Apply gyro bias
    float gx = _gx - _gyroBiasX;
    float gy = _gy - _gyroBiasY;
    float gz = _gz - _gyroBiasZ;

    // Low-pass accelerometer
    _axLPF = _axLPF + _accAlpha * (_ax - _axLPF);
    _ayLPF = _ayLPF + _accAlpha * (_ay - _ayLPF);
    _azLPF = _azLPF + _accAlpha * (_az - _azLPF);

    // Compute roll and pitch from accel
    float roll  = atan2f(_ayLPF, _azLPF);
    float pitch = atan2f(-_axLPF, sqrtf(_ayLPF*_ayLPF + _azLPF*_azLPF));

    // Calibrated magnetometer
    float mx = (_mx - _magBiasX) * _magScaleX;
    float my = (_my - _magBiasY) * _magScaleY;
    float mz = (_mz - _magBiasZ) * _magScaleZ;

    // Tilt-compensated mag
    float mx2 = mx * cosf(pitch) + mz * sinf(pitch);
    float my2 = mx * sinf(roll) * sinf(pitch) + my * cosf(roll) - mz * sinf(roll) * cosf(pitch);

    float yawMag = atan2f(-my2, mx2) + _declination;
    yawMag = wrapAngle(yawMag);

    // Gyro integration (assume Z is vertical)
    float yawGyro = _heading + gz * dt;
    yawGyro = wrapAngle(yawGyro);

    // Complementary fusion
    _heading = wrapAngle(_alpha * yawGyro + (1.0f - _alpha) * yawMag);

    return true;
}

float Mpu9250Heading::getHeadingRad() const
{
    return _heading;
}

float Mpu9250Heading::getHeadingDeg() const
{
    float deg = _heading * 180.0f / PI;
    if (deg < 0) deg += 360.0f;
    return deg;
}

void Mpu9250Heading::getRawAccel(float &ax, float &ay, float &az) const
{
    ax = _ax; ay = _ay; az = _az;
}

void Mpu9250Heading::getRawGyro(float &gx, float &gy, float &gz) const
{
    gx = _gx; gy = _gy; gz = _gz;
}

void Mpu9250Heading::getRawMag(float &mx, float &my, float &mz) const
{
    mx = _mx; my = _my; mz = _mz;
}

// --- Internal helpers ---

bool Mpu9250Heading::writeRegister(uint8_t addr, uint8_t reg, uint8_t value)
{
    _wire->beginTransmission(addr);
    _wire->write(reg);
    _wire->write(value);
    return (_wire->endTransmission() == 0);
}

bool Mpu9250Heading::readRegisters(uint8_t addr, uint8_t reg, uint8_t count, uint8_t* dest)
{
    _wire->beginTransmission(addr);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0) {
        return false;
    }

    uint8_t i = 0;
    _wire->requestFrom(addr, count);
    while (_wire->available() && i < count) {
        dest[i++] = _wire->read();
    }
    return (i == count);
}

bool Mpu9250Heading::initMPU()
{
    // Wake up device and select clock source
    if (!writeRegister(_mpuAddr, MPU_REG_PWR_MGMT_1, 0x01)) return false;
    delay(100);

    // Sample rate divider (1kHz / (1 + div)) -> 100 Hz
    if (!writeRegister(_mpuAddr, MPU_REG_SMPLRT_DIV, 9)) return false;

    // DLPF config: set to ~42 Hz
    if (!writeRegister(_mpuAddr, MPU_REG_CONFIG, 0x03)) return false;

    // Gyro full scale ±250 dps
    if (!writeRegister(_mpuAddr, MPU_REG_GYRO_CONFIG, 0x00)) return false;

    // Accel full scale ±2g
    if (!writeRegister(_mpuAddr, MPU_REG_ACCEL_CONFIG, 0x00)) return false;

    // Accel DLPF
    if (!writeRegister(_mpuAddr, MPU_REG_ACCEL_CONFIG2, 0x03)) return false;

    // Enable I2C bypass to access magnetometer directly
    if (!writeRegister(_mpuAddr, MPU_REG_INT_PIN_CFG, 0x02)) return false;

    return true;
}

bool Mpu9250Heading::initMag()
{
    // Power down magnetometer
    if (!writeRegister(_magAddr, AK8963_REG_CNTL1, 0x00)) return false;
    delay(10);
    // 16-bit output, continuous measurement mode 2 (100 Hz)
    if (!writeRegister(_magAddr, AK8963_REG_CNTL1, 0x16)) return false;
    delay(10);
    return true;
}

bool Mpu9250Heading::readAccelGyro()
{
    uint8_t buf[14];
    if (!readRegisters(_mpuAddr, MPU_REG_ACCEL_XOUT_H, 14, buf)) {
        return false;
    }

    int16_t ax_raw = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t ay_raw = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t az_raw = (int16_t)((buf[4] << 8) | buf[5]);

    // int16_t temp_raw = (int16_t)((buf[6] << 8) | buf[7]); // not used

    int16_t gx_raw = (int16_t)((buf[8] << 8) | buf[9]);
    int16_t gy_raw = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t gz_raw = (int16_t)((buf[12] << 8) | buf[13]);

    // Convert to physical units
    // For ±2g: 16384 LSB/g
    _ax = (float)ax_raw / 16384.0f;
    _ay = (float)ay_raw / 16384.0f;
    _az = (float)az_raw / 16384.0f;

    // For ±250 dps: 131 LSB/deg/s -> convert to rad/s
    const float deg2rad = PI / 180.0f;
    _gx = ((float)gx_raw / 131.0f) * deg2rad;
    _gy = ((float)gy_raw / 131.0f) * deg2rad;
    _gz = ((float)gz_raw / 131.0f) * deg2rad;

    return true;
}

bool Mpu9250Heading::readMag()
{
    uint8_t st1;
    if (!readRegisters(_magAddr, AK8963_REG_ST1, 1, &st1)) {
        return false;
    }
    if (!(st1 & 0x01)) {
        // No new data
        return true; // keep old values
    }

    uint8_t buf[7];
    if (!readRegisters(_magAddr, AK8963_REG_HXL, 7, buf)) {
        return false;
    }

    // Little-endian
    int16_t mx_raw = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t my_raw = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t mz_raw = (int16_t)((buf[5] << 8) | buf[4]);

    // buf[6] is ST2, can be used for overflow flag if needed

    // Convert to some consistent units.
    // For AK8963 16-bit: 0.15 uT per LSB.
    const float scale = 0.15f; // microTesla
    _mx = (float)mx_raw * scale;
    _my = (float)my_raw * scale;
    _mz = (float)mz_raw * scale;

    return true;
}

float Mpu9250Heading::wrapAngle(float x)
{
    while (x > PI)  x -= 2.0f * PI;
    while (x < -PI) x += 2.0f * PI;
    return x;
}