#include "Sensors.h"

Sensors::Sensors() {
    leftDistance = 0.0;
    middleDistance = 0.0;
    rightDistance = 0.0;
    lastUpdate = 0;
    sensorsInitialized = false;
}

bool Sensors::begin() {
    // Konfigurer sensor pins
    pinMode(SENSOR_LEFT_TRIG, OUTPUT);
    pinMode(SENSOR_LEFT_ECHO, INPUT);

    pinMode(SENSOR_MIDDLE_TRIG, OUTPUT);
    pinMode(SENSOR_MIDDLE_ECHO, INPUT);

    pinMode(SENSOR_RIGHT_TRIG, OUTPUT);
    pinMode(SENSOR_RIGHT_ECHO, INPUT);

    // Sikre at trigger pins er LOW
    digitalWrite(SENSOR_LEFT_TRIG, LOW);
    digitalWrite(SENSOR_MIDDLE_TRIG, LOW);
    digitalWrite(SENSOR_RIGHT_TRIG, LOW);

    // Første måling
    delay(50); // Kort delay for at stabilisere sensorer
    update();

    sensorsInitialized = true;

    #if DEBUG_SENSORS
    Serial.println("[Sensors] Initialized successfully");
    printValues();
    #endif

    return true;
}

void Sensors::update() {
    // Tjek om det er tid til opdatering
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate < SENSOR_UPDATE_INTERVAL) {
        return; // Ikke tid endnu
    }

    // Mål alle tre sensorer
    leftDistance = measureDistance(SENSOR_LEFT_TRIG, SENSOR_LEFT_ECHO);
    delay(10); // Kort pause mellem målinger

    middleDistance = measureDistance(SENSOR_MIDDLE_TRIG, SENSOR_MIDDLE_ECHO);
    delay(10);

    rightDistance = measureDistance(SENSOR_RIGHT_TRIG, SENSOR_RIGHT_ECHO);

    lastUpdate = currentTime;

    #if DEBUG_SENSORS
    // Debug output hver 10. opdatering (ca. 1 gang per sekund)
    static int updateCount = 0;
    updateCount++;
    if (updateCount >= 10) {
        printValues();
        updateCount = 0;
    }
    #endif
}

float Sensors::getLeftDistance() {
    return leftDistance;
}

float Sensors::getMiddleDistance() {
    return middleDistance;
}

float Sensors::getRightDistance() {
    return rightDistance;
}

bool Sensors::isObstacleLeft() {
    return isValidDistance(leftDistance) && (leftDistance < OBSTACLE_THRESHOLD);
}

bool Sensors::isObstacleMiddle() {
    return isValidDistance(middleDistance) && (middleDistance < OBSTACLE_THRESHOLD);
}

bool Sensors::isObstacleRight() {
    return isValidDistance(rightDistance) && (rightDistance < OBSTACLE_THRESHOLD);
}

bool Sensors::isAnyObstacle() {
    return isObstacleLeft() || isObstacleMiddle() || isObstacleRight();
}

float Sensors::getMinDistance() {
    float minDist = ULTRASONIC_MAX_DISTANCE;

    if (isValidDistance(leftDistance) && leftDistance < minDist) {
        minDist = leftDistance;
    }
    if (isValidDistance(middleDistance) && middleDistance < minDist) {
        minDist = middleDistance;
    }
    if (isValidDistance(rightDistance) && rightDistance < minDist) {
        minDist = rightDistance;
    }

    return minDist;
}

void Sensors::printValues() {
    Serial.printf("[Sensors] L: %.1f cm, M: %.1f cm, R: %.1f cm | Obstacles: ",
                  leftDistance, middleDistance, rightDistance);

    if (isObstacleLeft()) Serial.print("L ");
    if (isObstacleMiddle()) Serial.print("M ");
    if (isObstacleRight()) Serial.print("R ");
    if (!isAnyObstacle()) Serial.print("None");

    Serial.println();
}

float Sensors::measureDistance(int trigPin, int echoPin) {
    // Sikre trigger er LOW
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Send 10us puls
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Læs echo pin med timeout
    long duration = pulseIn(echoPin, HIGH, SENSOR_TIMEOUT);

    // Hvis timeout (duration = 0), returner max distance
    if (duration == 0) {
        return ULTRASONIC_MAX_DISTANCE;
    }

    // Beregn afstand i cm
    // Lydens hastighed: 343 m/s = 0.0343 cm/us
    // Distance = (duration / 2) * 0.0343
    float distance = (duration / 2.0) * 0.0343;

    // Begræns til max range
    if (distance > ULTRASONIC_MAX_DISTANCE) {
        distance = ULTRASONIC_MAX_DISTANCE;
    }

    return distance;
}

bool Sensors::isValidDistance(float distance) {
    // Afstand er gyldig hvis den er > 0 og < max distance
    return (distance > 0.0 && distance <= ULTRASONIC_MAX_DISTANCE);
}
