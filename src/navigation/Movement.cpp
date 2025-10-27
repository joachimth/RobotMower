#include "Movement.h"

Movement::Movement() {
    motorsPtr = nullptr;
    imuPtr = nullptr;
    targetHeading = 0.0;
    turningActive = false;
    movingForward = false;
    movingBackward = false;
    lastError = 0.0;
    integralError = 0.0;
    lastUpdate = 0;
    initialized = false;
}

bool Movement::begin(Motors* motors, IMU* imu) {
    if (motors == nullptr || imu == nullptr) {
        Logger::error("Movement: Invalid hardware pointers");
        return false;
    }

    motorsPtr = motors;
    imuPtr = imu;
    initialized = true;

    Logger::info("Movement controller initialized");

    return true;
}

void Movement::driveStraight(int speed) {
    if (!initialized) {
        return;
    }

    movingForward = true;
    movingBackward = false;
    turningActive = false;

    // Hent nuværende heading
    float currentHeading = imuPtr->getHeading();

    // Korrigér kurs for at holde target heading
    correctDrift(currentHeading, targetHeading);

    #if DEBUG_NAVIGATION
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 1000) {
        Logger::debug("Driving straight - Target: " + String(targetHeading) +
                     "° | Current: " + String(currentHeading) + "°");
        lastDebug = millis();
    }
    #endif
}

bool Movement::turnToHeading(float targetHeading) {
    if (!initialized) {
        return false;
    }

    this->targetHeading = targetHeading;
    turningActive = true;
    movingForward = false;
    movingBackward = false;

    // Hent nuværende heading
    float currentHeading = imuPtr->getHeading();

    // Beregn heading difference
    float headingError = MowerMath::angleDifference(currentHeading, targetHeading);

    #if DEBUG_NAVIGATION
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Logger::debug("Turning - Target: " + String(targetHeading) +
                     "° | Current: " + String(currentHeading) +
                     "° | Error: " + String(headingError) + "°");
        lastDebug = millis();
    }
    #endif

    // Tjek om vi har nået target heading
    if (abs(headingError) < HEADING_TOLERANCE) {
        motorsPtr->stop();
        turningActive = false;
        Logger::debug("Turn complete - Heading reached");
        return true; // Heading nået
    }

    // Drej i korrekt retning
    if (headingError > 0) {
        // Drej til højre (clockwise)
        motorsPtr->turnRight(MOTOR_TURN_SPEED);
    } else {
        // Drej til venstre (counter-clockwise)
        motorsPtr->turnLeft(MOTOR_TURN_SPEED);
    }

    return false; // Stadig drejer
}

void Movement::backUp(int distance) {
    if (!initialized) {
        return;
    }

    movingBackward = true;
    movingForward = false;
    turningActive = false;

    // Estimér tid baseret på distance og hastighed
    // Antager MOTOR_BACKUP_SPEED giver ca. 15 cm/s (juster efter behov)
    const float BACKUP_SPEED_CM_PER_SEC = 15.0;
    int backupTime = (distance / BACKUP_SPEED_CM_PER_SEC) * 1000; // ms

    Logger::debug("Backing up " + String(distance) + " cm");

    motorsPtr->backward(MOTOR_BACKUP_SPEED);
    delay(backupTime); // Simpel blocking delay for backup
    motorsPtr->stop();

    movingBackward = false;

    Logger::debug("Backup complete");
}

void Movement::stop() {
    if (!initialized) {
        return;
    }

    motorsPtr->stop();
    movingForward = false;
    movingBackward = false;
    turningActive = false;

    // Nulstil PID
    lastError = 0.0;
    integralError = 0.0;
}

void Movement::turnInPlace(Direction direction, int speed) {
    if (!initialized) {
        return;
    }

    turningActive = true;
    movingForward = false;
    movingBackward = false;

    if (direction == LEFT) {
        motorsPtr->turnLeft(speed);
    } else if (direction == RIGHT) {
        motorsPtr->turnRight(speed);
    }
}

bool Movement::isMoving() {
    return (movingForward || movingBackward || turningActive);
}

bool Movement::isTurning() {
    return turningActive;
}

void Movement::setTargetHeading(float heading) {
    targetHeading = MowerMath::normalizeAngle(heading);
    Logger::debug("Target heading set to: " + String(targetHeading) + "°");
}

float Movement::getTargetHeading() {
    return targetHeading;
}

void Movement::update() {
    if (!initialized) {
        return;
    }

    lastUpdate = millis();

    // Opdater kun hvis vi kører lige fremad
    if (movingForward && !turningActive) {
        float currentHeading = imuPtr->getHeading();
        correctDrift(currentHeading, targetHeading);
    }
}

void Movement::correctDrift(float currentHeading, float targetHeading) {
    // Beregn heading fejl
    float error = MowerMath::angleDifference(currentHeading, targetHeading);

    // PID korrektion
    int correction = calculatePIDCorrection(error);

    // Begrænse korrektion
    correction = constrain(correction, -100, 100);

    // Anvend korrektion til motor hastigheder
    int baseSpeed = MOTOR_CRUISE_SPEED;
    int leftSpeed = baseSpeed - correction;
    int rightSpeed = baseSpeed + correction;

    // Begrænse motor hastigheder
    leftSpeed = constrain(leftSpeed, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);
    rightSpeed = constrain(rightSpeed, MOTOR_MIN_SPEED, MOTOR_MAX_SPEED);

    motorsPtr->setSpeed(leftSpeed, rightSpeed);
}

int Movement::calculatePIDCorrection(float error) {
    // Proportional
    float P = MOTOR_KP * error;

    // Integral (akkumuleret fejl)
    integralError += error;
    integralError = constrain(integralError, -100, 100); // Anti-windup
    float I = MOTOR_KI * integralError;

    // Derivative (ændring i fejl)
    float D = MOTOR_KD * (error - lastError);

    lastError = error;

    // Samlet PID output
    float pidOutput = P + I + D;

    return (int)pidOutput;
}
