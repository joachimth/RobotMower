#include "IMU.h"

IMU::IMU() {
    accelX = accelY = accelZ = 0;
    gyroX = gyroY = gyroZ = 0;
    heading = yaw = pitch = roll = 0.0;
    gyroXOffset = gyroYOffset = gyroZOffset = 0.0;
    lastUpdate = 0;
    deltaTime = 0.0;
    initialized = false;
    calibrated = false;
    headingOffset = 0.0;
}

bool IMU::begin() {
    // Initialiser I2C
    Wire.begin(IMU_SDA, IMU_SCL);
    Wire.setClock(400000); // 400kHz I2C hastighed

    delay(100); // Lad IMU stabilisere

    // Wake up MPU6050/9250
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(PWR_MGMT_1);
    Wire.write(0); // Wake up
    byte error = Wire.endTransmission();

    if (error != 0) {
        Serial.printf("[IMU] Failed to initialize - I2C error: %d\n", error);
        return false;
    }

    delay(100);

    // Verificer kommunikation ved at læse WHO_AM_I register
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x75); // WHO_AM_I register
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, (uint8_t)1);

    if (Wire.available()) {
        uint8_t whoami = Wire.read();
        Serial.printf("[IMU] WHO_AM_I: 0x%02X\n", whoami);

        // MPU6050 returnerer 0x68, MPU9250 returnerer 0x71
        if (whoami != 0x68 && whoami != 0x71) {
            Serial.println("[IMU] Warning: Unexpected WHO_AM_I value");
        }
    }

    initialized = true;
    lastUpdate = millis();

    Serial.println("[IMU] Initialized successfully");
    Serial.println("[IMU] !!! IMPORTANT: Run calibration before use !!!");

    return true;
}

void IMU::calibrate() {
    if (!initialized) {
        Serial.println("[IMU] Error: Not initialized");
        return;
    }

    Serial.println("[IMU] Starting calibration...");
    Serial.println("[IMU] Keep robot STILL for 5 seconds!");

    delay(2000); // Giv bruger tid til at stoppe bevægelse

    // Nulstil offsets
    gyroXOffset = 0.0;
    gyroYOffset = 0.0;
    gyroZOffset = 0.0;

    // Tag flere målinger
    long sumX = 0, sumY = 0, sumZ = 0;

    for (int i = 0; i < IMU_CALIBRATION_SAMPLES; i++) {
        readGyroscope();
        sumX += gyroX;
        sumY += gyroY;
        sumZ += gyroZ;
        delay(10);

        // Progress indicator
        if (i % 20 == 0) {
            Serial.print(".");
        }
    }

    Serial.println();

    // Beregn gennemsnitlige offsets
    gyroXOffset = (float)sumX / IMU_CALIBRATION_SAMPLES;
    gyroYOffset = (float)sumY / IMU_CALIBRATION_SAMPLES;
    gyroZOffset = (float)sumZ / IMU_CALIBRATION_SAMPLES;

    Serial.printf("[IMU] Calibration complete - Offsets: X=%.2f, Y=%.2f, Z=%.2f\n",
                  gyroXOffset, gyroYOffset, gyroZOffset);

    // Nulstil heading
    heading = 0.0;
    yaw = 0.0;
    headingOffset = 0.0;

    calibrated = true;
}

void IMU::update() {
    if (!initialized) {
        return;
    }

    unsigned long currentTime = millis();
    deltaTime = (currentTime - lastUpdate) / 1000.0; // Sekunder
    lastUpdate = currentTime;

    // Læs sensorer
    readAccelerometer();
    readGyroscope();

    // Beregn orientering
    calculateOrientation();
}

float IMU::getHeading() {
    return normalizeAngle(heading - headingOffset);
}

float IMU::getYaw() {
    return yaw;
}

float IMU::getPitch() {
    return pitch;
}

float IMU::getRoll() {
    return roll;
}

void IMU::resetHeading() {
    headingOffset = heading;
    Serial.printf("[IMU] Heading reset - New offset: %.2f\n", headingOffset);
}

bool IMU::isCalibrated() {
    return calibrated;
}

bool IMU::isTilted() {
    // Tjek om robot er væltet baseret på pitch eller roll
    return (abs(pitch) > TILT_ANGLE_THRESHOLD || abs(roll) > TILT_ANGLE_THRESHOLD);
}

void IMU::printValues() {
    Serial.printf("[IMU] Heading: %.1f° | Pitch: %.1f° | Roll: %.1f° | Yaw: %.1f°\n",
                  getHeading(), pitch, roll, yaw);
}

void IMU::readAccelerometer() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, (uint8_t)6);

    if (Wire.available() >= 6) {
        accelX = (Wire.read() << 8) | Wire.read();
        accelY = (Wire.read() << 8) | Wire.read();
        accelZ = (Wire.read() << 8) | Wire.read();
    }
}

void IMU::readGyroscope() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(GYRO_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, (uint8_t)6);

    if (Wire.available() >= 6) {
        gyroX = (Wire.read() << 8) | Wire.read();
        gyroY = (Wire.read() << 8) | Wire.read();
        gyroZ = (Wire.read() << 8) | Wire.read();
    }
}

void IMU::calculateOrientation() {
    // Konverter accelerometer værdier til g (assuming ±2g range)
    float accelXg = accelX / 16384.0;
    float accelYg = accelY / 16384.0;
    float accelZg = accelZ / 16384.0;

    // Beregn pitch og roll fra accelerometer
    float accelPitch = atan2(-accelXg, sqrt(accelYg * accelYg + accelZg * accelZg)) * 180.0 / PI;
    float accelRoll = atan2(accelYg, accelZg) * 180.0 / PI;

    // Konverter gyroscope værdier til grader/sekund (assuming ±250°/s range)
    // og anvend kalibrerings offsets
    float gyroXrate = ((gyroX - gyroXOffset) / 131.0) * deltaTime;
    float gyroYrate = ((gyroY - gyroYOffset) / 131.0) * deltaTime;
    float gyroZrate = ((gyroZ - gyroZOffset) / 131.0) * deltaTime;

    // Komplementær filter for pitch og roll (gyro + accel)
    pitch = complementaryFilter(pitch + gyroYrate, accelPitch, 0.98);
    roll = complementaryFilter(roll + gyroXrate, accelRoll, 0.98);

    // Yaw/Heading fra gyroscope Z-akse (kun gyro, ingen magnetometer i brug)
    yaw += gyroZrate;
    yaw = normalizeAngle(yaw);

    // Heading er samme som yaw for 2D navigation
    heading = yaw;
}

float IMU::normalizeAngle(float angle) {
    while (angle < 0.0) {
        angle += 360.0;
    }
    while (angle >= 360.0) {
        angle -= 360.0;
    }
    return angle;
}

float IMU::complementaryFilter(float gyroValue, float accelValue, float alpha) {
    // Komplementær filter: alpha * gyro + (1-alpha) * accel
    // alpha tæt på 1 = stoler mere på gyro (mindre noise, men drifter)
    // alpha tæt på 0 = stoler mere på accel (mere noise, men drifter ikke)
    return alpha * gyroValue + (1.0 - alpha) * accelValue;
}
