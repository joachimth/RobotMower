#include "IMU.h"
#include <math.h>

IMU::IMU()
    : _ax(0), _ay(0), _az(0),
      _gx(0), _gy(0), _gz(0),
      _mx(0), _my(0), _mz(0),
      _axLPF(0), _ayLPF(0), _azLPF(1.0f),
      _accAlpha(0.1f),
      _gyroBiasX(0), _gyroBiasY(0), _gyroBiasZ(0),
      _magBiasX(0), _magBiasY(0), _magBiasZ(0),
      _magScaleX(1.0f), _magScaleY(1.0f), _magScaleZ(1.0f),
      _declination(0.0f),
      _heading(0), _pitch(0), _roll(0),
      _alpha(0.98f),
      _lastMicros(0),
      _initialized(false),
      _gyroCalibrated(false),
      _magnetometerAvailable(false),
      _headingOffset(0),
      _encoderFusionEnabled(false),
      _encoderHeading(0),
      _encoderConfidence(0)
{
}

bool IMU::begin() {
    // Initialiser I2C
    Wire.begin(IMU_SDA, IMU_SCL);
    Wire.setClock(400000); // 400kHz I2C hastighed

    delay(100); // Lad IMU stabilisere

    // Initialiser MPU-9250/6050
    if (!initMPU()) {
        Serial.println("[IMU] Failed to initialize MPU");
        return false;
    }

    // Forsøg at initialisere magnetometer (AK8963)
    _magnetometerAvailable = initMagnetometer();

    if (_magnetometerAvailable) {
        Serial.println("[IMU] Magnetometer (AK8963) detected and initialized");
        Serial.println("[IMU] Heading will use magnetometer + gyro fusion");
    } else {
        Serial.println("[IMU] WARNING: No magnetometer detected!");
        Serial.println("[IMU] Heading will use gyro-only (will drift over time)");
        Serial.println("[IMU] Consider using encoder fusion for better stability");
    }

    _lastMicros = micros();
    _initialized = true;

    Serial.println("[IMU] Initialized successfully");
    Serial.println("[IMU] !!! IMPORTANT: Run calibrateGyro() before use !!!");

    // Sæt default magnetisk deklination for Danmark (~3 grader øst)
    setDeclination(3.0f);

    return true;
}

bool IMU::initMPU() {
    // Wake up device og vælg clock source (PLL med X gyro reference)
    if (!writeRegister(MPU_ADDR, PWR_MGMT_1, 0x01)) {
        Serial.println("[IMU] Failed to wake up MPU");
        return false;
    }
    delay(100);

    // Verificer kommunikation ved at læse WHO_AM_I register
    uint8_t whoami;
    if (!readRegisters(MPU_ADDR, 0x75, 1, &whoami)) {
        Serial.println("[IMU] Failed to read WHO_AM_I");
        return false;
    }

    Serial.printf("[IMU] WHO_AM_I: 0x%02X\n", whoami);

    // MPU6050 returnerer 0x68, MPU9250 returnerer 0x71
    if (whoami != 0x68 && whoami != 0x71) {
        Serial.println("[IMU] Warning: Unexpected WHO_AM_I value");
    }

    // Sample rate divider: 1kHz / (1 + 9) = 100Hz
    if (!writeRegister(MPU_ADDR, SMPLRT_DIV, 9)) {
        Serial.println("[IMU] Warning: Failed to set sample rate");
    }
    delay(10);

    // DLPF config: ~42Hz bandwidth (reducerer højfrekvent støj)
    if (!writeRegister(MPU_ADDR, CONFIG_REG, 0x03)) {
        Serial.println("[IMU] Warning: Failed to set DLPF");
    }
    delay(10);

    // Gyroscope: ±250°/s (mest følsom, mindst støj)
    if (!writeRegister(MPU_ADDR, GYRO_CONFIG, 0x00)) {
        Serial.println("[IMU] Warning: Failed to set gyro range");
    }
    delay(10);

    // Accelerometer: ±2g (mest følsom)
    if (!writeRegister(MPU_ADDR, ACCEL_CONFIG, 0x00)) {
        Serial.println("[IMU] Warning: Failed to set accel range");
    }
    delay(10);

    // Accelerometer DLPF
    if (!writeRegister(MPU_ADDR, ACCEL_CONFIG2, 0x03)) {
        Serial.println("[IMU] Warning: Failed to set accel DLPF");
    }
    delay(10);

    // Enable I2C bypass for at tilgå magnetometer direkte
    if (!writeRegister(MPU_ADDR, INT_PIN_CFG, 0x02)) {
        Serial.println("[IMU] Warning: Failed to enable I2C bypass");
    }
    delay(10);

    Serial.println("[IMU] MPU configured: DLPF=42Hz, Gyro=±250°/s, Accel=±2g, Rate=100Hz");
    return true;
}

bool IMU::initMagnetometer() {
    // Tjek om magnetometer er tilgængelig
    if (!detectMagnetometer()) {
        return false;
    }

    // Power down magnetometer først
    if (!writeRegister(MAG_ADDR, AK8963_CNTL1, 0x00)) {
        return false;
    }
    delay(10);

    // Soft reset
    if (!writeRegister(MAG_ADDR, AK8963_CNTL2, 0x01)) {
        Serial.println("[IMU] Warning: Mag soft reset failed");
    }
    delay(10);

    // 16-bit output, continuous measurement mode 2 (100Hz)
    // Mode 2: 0x16 = 0001 0110 = 16-bit (bit4=1), mode 2 (bits 3:0 = 0110)
    if (!writeRegister(MAG_ADDR, AK8963_CNTL1, 0x16)) {
        Serial.println("[IMU] Failed to configure magnetometer");
        return false;
    }
    delay(10);

    return true;
}

bool IMU::detectMagnetometer() {
    // Forsøg at læse WIA (Who I Am) register fra AK8963
    // AK8963 WIA register er 0x00 og skal returnere 0x48
    uint8_t wia;

    Wire.beginTransmission(MAG_ADDR);
    Wire.write(0x00); // WIA register
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    Wire.requestFrom(MAG_ADDR, (uint8_t)1);
    if (Wire.available()) {
        wia = Wire.read();
        if (wia == 0x48) {
            Serial.printf("[IMU] AK8963 detected (WIA: 0x%02X)\n", wia);
            return true;
        }
    }

    return false;
}

void IMU::calibrateGyro(uint16_t samples) {
    if (!_initialized) {
        Serial.println("[IMU] Error: Not initialized");
        return;
    }

    Serial.println("[IMU] Starting gyro calibration...");
    Serial.println("[IMU] Keep robot STILL!");

    delay(1000); // Giv bruger tid til at stoppe bevægelse

    float sumX = 0, sumY = 0, sumZ = 0;
    uint16_t validSamples = 0;

    for (uint16_t i = 0; i < samples; i++) {
        if (readAccelGyro()) {
            sumX += _gx;
            sumY += _gy;
            sumZ += _gz;
            validSamples++;
        }
        delay(2);

        // Progress indicator
        if (i % 50 == 0) {
            Serial.print(".");
        }
    }
    Serial.println();

    if (validSamples > 0) {
        _gyroBiasX = sumX / validSamples;
        _gyroBiasY = sumY / validSamples;
        _gyroBiasZ = sumZ / validSamples;

        Serial.printf("[IMU] Gyro calibration complete - Bias: X=%.4f, Y=%.4f, Z=%.4f rad/s\n",
                      _gyroBiasX, _gyroBiasY, _gyroBiasZ);

        _gyroCalibrated = true;
        _heading = 0;
        _headingOffset = 0;
    } else {
        Serial.println("[IMU] Calibration failed - no valid samples");
    }
}

void IMU::setMagCalibration(float biasX, float biasY, float biasZ,
                            float scaleX, float scaleY, float scaleZ) {
    _magBiasX = biasX;
    _magBiasY = biasY;
    _magBiasZ = biasZ;
    _magScaleX = scaleX;
    _magScaleY = scaleY;
    _magScaleZ = scaleZ;

    Serial.printf("[IMU] Mag calibration set - Bias: (%.1f, %.1f, %.1f), Scale: (%.2f, %.2f, %.2f)\n",
                  biasX, biasY, biasZ, scaleX, scaleY, scaleZ);
}

void IMU::setDeclination(float declDeg) {
    _declination = declDeg * PI / 180.0f;
    Serial.printf("[IMU] Magnetic declination set: %.1f degrees\n", declDeg);
}

void IMU::setAlpha(float alpha) {
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    _alpha = alpha;
    Serial.printf("[IMU] Complementary filter alpha set: %.2f\n", alpha);
}

bool IMU::update() {
    if (!_initialized) {
        return false;
    }

    // Beregn delta time
    unsigned long now = micros();
    float dt = (now - _lastMicros) * 1e-6f;

    // Validér deltaTime
    if (dt <= 0.0f || dt > 1.0f) {
        dt = 0.01f; // Fallback til 100Hz
    }
    _lastMicros = now;

    // Læs sensorer
    if (!readAccelGyro()) {
        return false;
    }

    // Læs magnetometer hvis tilgængelig
    if (_magnetometerAvailable) {
        readMagnetometer(); // Fejl her er ikke kritisk
    }

    // Anvend gyro bias korrektion
    float gx = _gx - _gyroBiasX;
    float gy = _gy - _gyroBiasY;
    float gz = _gz - _gyroBiasZ;

    // Low-pass filter på accelerometer
    _axLPF = _axLPF + _accAlpha * (_ax - _axLPF);
    _ayLPF = _ayLPF + _accAlpha * (_ay - _ayLPF);
    _azLPF = _azLPF + _accAlpha * (_az - _azLPF);

    // Beregn roll og pitch fra accelerometer
    _roll = atan2f(_ayLPF, _azLPF);
    _pitch = atan2f(-_axLPF, sqrtf(_ayLPF * _ayLPF + _azLPF * _azLPF));

    // Beregn heading
    float yawGyro = _heading + gz * dt;
    yawGyro = wrapAngle(yawGyro);

    if (_magnetometerAvailable) {
        // Kalibreret magnetometer
        float mx = (_mx - _magBiasX) * _magScaleX;
        float my = (_my - _magBiasY) * _magScaleY;
        float mz = (_mz - _magBiasZ) * _magScaleZ;

        // Tilt-kompenseret magnetometer heading
        float cosPitch = cosf(_pitch);
        float sinPitch = sinf(_pitch);
        float cosRoll = cosf(_roll);
        float sinRoll = sinf(_roll);

        float mx2 = mx * cosPitch + mz * sinPitch;
        float my2 = mx * sinRoll * sinPitch + my * cosRoll - mz * sinRoll * cosPitch;

        float yawMag = atan2f(-my2, mx2) + _declination;
        yawMag = wrapAngle(yawMag);

        // Complementary filter: fuser gyro og magnetometer
        // Håndter wrap-around ved ±PI
        float diff = yawMag - yawGyro;
        if (diff > PI) diff -= 2.0f * PI;
        if (diff < -PI) diff += 2.0f * PI;

        _heading = wrapAngle(yawGyro + (1.0f - _alpha) * diff);
    } else {
        // Ingen magnetometer - brug kun gyro (vil drifte!)
        _heading = yawGyro;
    }

    // Encoder fusion (hvis aktiveret)
    if (_encoderFusionEnabled && _encoderConfidence > 0) {
        float encoderRad = _encoderHeading * PI / 180.0f;
        float diff = encoderRad - _heading;
        if (diff > PI) diff -= 2.0f * PI;
        if (diff < -PI) diff += 2.0f * PI;

        // Fuser baseret på confidence (0-1)
        // Lav confidence = lille korrektion
        float encoderAlpha = 0.1f * _encoderConfidence;
        _heading = wrapAngle(_heading + encoderAlpha * diff);
    }

    return true;
}

bool IMU::readAccelGyro() {
    uint8_t buf[14];
    if (!readRegisters(MPU_ADDR, ACCEL_XOUT_H, 14, buf)) {
        return false;
    }

    int16_t ax_raw = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t ay_raw = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t az_raw = (int16_t)((buf[4] << 8) | buf[5]);
    // buf[6], buf[7] er temperatur - ikke brugt
    int16_t gx_raw = (int16_t)((buf[8] << 8) | buf[9]);
    int16_t gy_raw = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t gz_raw = (int16_t)((buf[12] << 8) | buf[13]);

    // Konverter til fysiske enheder
    // ±2g: 16384 LSB/g
    _ax = (float)ax_raw / 16384.0f;
    _ay = (float)ay_raw / 16384.0f;
    _az = (float)az_raw / 16384.0f;

    // ±250°/s: 131 LSB/(°/s) -> konverter til rad/s
    const float deg2rad = PI / 180.0f;
    _gx = ((float)gx_raw / 131.0f) * deg2rad;
    _gy = ((float)gy_raw / 131.0f) * deg2rad;
    _gz = ((float)gz_raw / 131.0f) * deg2rad;

    return true;
}

bool IMU::readMagnetometer() {
    // Tjek om data er klar (ST1 register bit 0)
    uint8_t st1;
    if (!readRegisters(MAG_ADDR, AK8963_ST1, 1, &st1)) {
        return false;
    }

    if (!(st1 & 0x01)) {
        // Ingen ny data - behold gamle værdier
        return true;
    }

    // Læs magnetometer data (6 bytes) + ST2 (1 byte for at rydde DRDY)
    uint8_t buf[7];
    if (!readRegisters(MAG_ADDR, AK8963_HXL, 7, buf)) {
        return false;
    }

    // AK8963 er little-endian
    int16_t mx_raw = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t my_raw = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t mz_raw = (int16_t)((buf[5] << 8) | buf[4]);
    // buf[6] er ST2 - kan bruges til overflow check

    // Tjek for overflow (ST2 bit 3)
    if (buf[6] & 0x08) {
        // Overflow - data er ikke valid
        return false;
    }

    // Konverter til µT (16-bit mode: 0.15 µT/LSB)
    const float scale = 0.15f;
    _mx = (float)mx_raw * scale;
    _my = (float)my_raw * scale;
    _mz = (float)mz_raw * scale;

    return true;
}

bool IMU::writeRegister(uint8_t addr, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool IMU::readRegisters(uint8_t addr, uint8_t reg, uint8_t count, uint8_t* dest) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    uint8_t i = 0;
    Wire.requestFrom(addr, count);
    while (Wire.available() && i < count) {
        dest[i++] = Wire.read();
    }
    return (i == count);
}

float IMU::wrapAngle(float angle) {
    while (angle > PI) angle -= 2.0f * PI;
    while (angle < -PI) angle += 2.0f * PI;
    return angle;
}

float IMU::normalizeAngle(float angle) {
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

float IMU::getHeading() {
    float deg = (_heading - _headingOffset) * 180.0f / PI;
    return normalizeAngle(deg);
}

float IMU::getHeadingRad() {
    return wrapAngle(_heading - _headingOffset);
}

float IMU::getPitch() {
    return _pitch * 180.0f / PI;
}

float IMU::getRoll() {
    return _roll * 180.0f / PI;
}

void IMU::resetHeading() {
    _headingOffset = _heading;
    Serial.printf("[IMU] Heading reset - offset: %.2f rad\n", _headingOffset);
}

bool IMU::isCalibrated() {
    return _gyroCalibrated;
}

bool IMU::hasMagnetometer() {
    return _magnetometerAvailable;
}

bool IMU::isTilted() {
    float pitchDeg = fabsf(_pitch * 180.0f / PI);
    float rollDeg = fabsf(_roll * 180.0f / PI);
    return (pitchDeg > TILT_THRESHOLD || rollDeg > TILT_THRESHOLD);
}

void IMU::printValues() {
    Serial.printf("[IMU] Heading: %.1f° | Pitch: %.1f° | Roll: %.1f° | Mag: %s\n",
                  getHeading(), getPitch(), getRoll(),
                  _magnetometerAvailable ? "OK" : "N/A");
}

void IMU::getRawAccel(float &ax, float &ay, float &az) {
    ax = _ax;
    ay = _ay;
    az = _az;
}

void IMU::getRawGyro(float &gx, float &gy, float &gz) {
    gx = _gx;
    gy = _gy;
    gz = _gz;
}

void IMU::getRawMag(float &mx, float &my, float &mz) {
    mx = _mx;
    my = _my;
    mz = _mz;
}

// ========== Encoder Fusion Interface ==========

void IMU::fuseEncoderHeading(float encoderHeadingDeg, float confidence) {
    _encoderHeading = encoderHeadingDeg;
    _encoderConfidence = constrain(confidence, 0.0f, 1.0f);
}

void IMU::setEncoderFusionEnabled(bool enabled) {
    _encoderFusionEnabled = enabled;
    if (enabled) {
        Serial.println("[IMU] Encoder fusion enabled");
    } else {
        Serial.println("[IMU] Encoder fusion disabled");
        _encoderConfidence = 0;
    }
}
