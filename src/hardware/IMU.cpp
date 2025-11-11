#include "IMU.h"

IMU::IMU() {
    accelX = accelY = accelZ = 0;
    gyroX = gyroY = gyroZ = 0;
    heading = yaw = pitch = roll = 0.0;
    gyroXOffset = gyroYOffset = gyroZOffset = 0.0;
    gyroBiasZ = 0.0;
    stationaryTime = 0;
    lastUpdate = 0;
    deltaTime = 0.0;
    initialized = false;
    calibrated = false;
    headingOffset = 0.0;
    gyroZBufferIndex = 0;
    for (int i = 0; i < GYRO_BUFFER_SIZE; i++) {
        gyroZBuffer[i] = 0.0;
    }
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

    // Konfigurer Digital Low Pass Filter (DLPF)
    // DLPF_CFG = 3: Bandwidth 44Hz (gyro), 44Hz (accel), 1kHz sample rate
    // Dette reducerer højfrekvent støj betydeligt
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(CONFIG);
    Wire.write(0x03); // DLPF_CFG = 3
    if (Wire.endTransmission() != 0) {
        Serial.println("[IMU] Warning: Failed to set DLPF");
    }
    delay(10);

    // Konfigurer Gyroscope range: ±250°/s (mest følsom, mindst støj)
    // FS_SEL = 0: ±250°/s, LSB Sensitivity = 131 LSB/(°/s)
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(GYRO_CONFIG);
    Wire.write(0x00); // FS_SEL = 0
    if (Wire.endTransmission() != 0) {
        Serial.println("[IMU] Warning: Failed to set gyro range");
    }
    delay(10);

    // Konfigurer Accelerometer range: ±2g (mest følsom)
    // AFS_SEL = 0: ±2g, LSB Sensitivity = 16384 LSB/g
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(ACCEL_CONFIG);
    Wire.write(0x00); // AFS_SEL = 0
    if (Wire.endTransmission() != 0) {
        Serial.println("[IMU] Warning: Failed to set accel range");
    }
    delay(10);

    // Konfigurer Sample Rate Divider
    // Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
    // Med DLPF enabled: Gyro Output = 1kHz, så 1kHz/(1+9) = 100Hz
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(SMPLRT_DIV);
    Wire.write(0x09); // Divider = 9, giver 100Hz sample rate
    if (Wire.endTransmission() != 0) {
        Serial.println("[IMU] Warning: Failed to set sample rate");
    }
    delay(10);

    Serial.println("[IMU] MPU6050 configured: DLPF=44Hz, Gyro=±250°/s, Accel=±2g, Rate=100Hz");

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

    // Opdater drift kompensation
    updateGyroBias();

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
    // Validér deltaTime - forhindrer spring i beregningerne
    if (deltaTime <= 0.0 || deltaTime > 1.0) {
        // Ugyldig deltaTime - skip denne opdatering
        Serial.printf("[IMU] Warning: Invalid deltaTime: %.3f\n", deltaTime);
        return;
    }

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

    // Anvend smoothing, offset OG bias kompensation for Z gyro
    float gyroZrateRaw = getSmoothedGyroZ(); // Brug smoothed værdi i grader/sek

    // Validér at gyro værdi er rimelig (ikke sensor fejl)
    // ±250°/s range, så absolut værdi bør ikke overstige ~200°/s i normal brug
    if (abs(gyroZrateRaw) > 200.0) {
        Serial.printf("[IMU] Warning: Extreme gyro value: %.1f deg/s - ignoring\n", gyroZrateRaw);
        return;
    }

    float gyroZrate = (gyroZrateRaw - gyroBiasZ) * deltaTime; // Anvend bias korrektion

    // Komplementær filter for pitch og roll (gyro + accel)
    pitch = complementaryFilter(pitch + gyroYrate, accelPitch, 0.98);
    roll = complementaryFilter(roll + gyroXrate, accelRoll, 0.98);

    // Yaw/Heading fra gyroscope Z-akse (kun gyro, ingen magnetometer i brug)
    // Nu med drift kompensation
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

bool IMU::isStationary() {
    // Tjek om gyro værdier er små (næsten ingen rotation)
    float gyroZrate = abs((gyroZ - gyroZOffset) / 131.0); // grader/sek

    return (gyroZrate < GYRO_STATIONARY_THRESHOLD);
}

float IMU::getSmoothedGyroZ() {
    // Tilføj ny måling til ring buffer
    gyroZBuffer[gyroZBufferIndex] = (gyroZ - gyroZOffset) / 131.0; // grader/sek
    gyroZBufferIndex = (gyroZBufferIndex + 1) % GYRO_BUFFER_SIZE;

    // Beregn gennemsnit
    float sum = 0.0;
    for (int i = 0; i < GYRO_BUFFER_SIZE; i++) {
        sum += gyroZBuffer[i];
    }
    return sum / GYRO_BUFFER_SIZE;
}

void IMU::updateGyroBias() {
    if (!calibrated) {
        return;
    }

    unsigned long currentTime = millis();

    if (isStationary()) {
        // Robot er stille - akkumuler tid
        if (stationaryTime == 0) {
            stationaryTime = currentTime;
        }

        // Hvis robotten har været stille i tilstrækkelig lang tid
        if ((currentTime - stationaryTime) > BIAS_UPDATE_TIME) {
            // Opdater bias estimat med exponential moving average
            // Dette antager at gennemsnitlig rotation over tid skal være 0
            float rawGyroZ = getSmoothedGyroZ(); // Brug smoothed værdi

            // Hurtigere opdatering af bias (alpha = 0.05 betyder 5% ny, 95% gammel)
            // Dette gør at bias opdateres hurtigere end før
            gyroBiasZ = 0.95 * gyroBiasZ + 0.05 * rawGyroZ;

            #if DEBUG_IMU
            if (abs(gyroBiasZ) > 0.05) {
                Serial.printf("[IMU] Gyro bias Z updated: %.3f deg/s\n", gyroBiasZ);
            }
            #endif
        }
    } else {
        // Robot bevæger sig - nulstil stationary timer
        stationaryTime = 0;
    }
}
