#include <Wire.h>
#include "Mpu9250Heading.h"

Mpu9250Heading heading;

void setup() {
    Serial.begin(115200);
    delay(2000);

    if (!heading.begin(Wire)) {
        Serial.println("Failed to init MPU9250!");
        while (1) delay(100);
    }

    Serial.println("Calibrating gyro, keep robot still...");
    heading.calibrateGyro(500);
    Serial.println("Done.");

    // Sæt evt. komplementært filter
    heading.setAlpha(0.98f);

    // Sæt magnetisk deklination for din lokation (eksempel: 3 grader øst)
    float declDeg = 3.0f;
    heading.setDeclination(declDeg * PI / 180.0f);

    // Sæt magnetometer-kalibrering (eksempelværdier - udskift med dine egne!)
    // biasX, biasY, biasZ, scaleX, scaleY, scaleZ
    heading.setMagCalibration(0.0f, 0.0f, 0.0f,
                              1.0f, 1.0f, 1.0f);
}

void loop() {
    if (heading.update()) {
        float hdgDeg = heading.getHeadingDeg();
        Serial.print("Heading: ");
        Serial.print(hdgDeg, 1);
        Serial.println(" deg");

        delay(50);
    } else {
        Serial.println("Read failed");
        delay(100);
    }
}