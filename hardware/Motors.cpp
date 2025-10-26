#include "Motors.h"

Motors::Motors() {
    currentLeftSpeed = 0;
    currentRightSpeed = 0;
    emergencyStopped = false;
}

bool Motors::begin() {
    // Konfigurer motor pins som outputs
    pinMode(MOTOR_LEFT_PWM, OUTPUT);
    pinMode(MOTOR_LEFT_IN1, OUTPUT);
    pinMode(MOTOR_LEFT_IN2, OUTPUT);

    pinMode(MOTOR_RIGHT_PWM, OUTPUT);
    pinMode(MOTOR_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR_RIGHT_IN2, OUTPUT);

    // Konfigurer PWM kanaler (ESP32 specifik)
    // Venstre motor - PWM kanal 0
    ledcSetup(0, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcAttachPin(MOTOR_LEFT_PWM, 0);

    // Højre motor - PWM kanal 1
    ledcSetup(1, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcAttachPin(MOTOR_RIGHT_PWM, 1);

    // Initialiser motorer til stop
    stop();

    #if DEBUG_MOTORS
    Serial.println("[Motors] Initialized successfully");
    #endif

    return true;
}

void Motors::setSpeed(int leftSpeed, int rightSpeed) {
    // Tjek for emergency stop
    if (emergencyStopped) {
        #if DEBUG_MOTORS
        Serial.println("[Motors] Emergency stop active - ignoring command");
        #endif
        return;
    }

    // Begrænse hastigheder
    leftSpeed = constrainSpeed(leftSpeed);
    rightSpeed = constrainSpeed(rightSpeed);

    // Anvend minimum hastighed threshold for at overvinde friktion
    if (abs(leftSpeed) > 0 && abs(leftSpeed) < MOTOR_MIN_SPEED) {
        leftSpeed = (leftSpeed > 0) ? MOTOR_MIN_SPEED : -MOTOR_MIN_SPEED;
    }
    if (abs(rightSpeed) > 0 && abs(rightSpeed) < MOTOR_MIN_SPEED) {
        rightSpeed = (rightSpeed > 0) ? MOTOR_MIN_SPEED : -MOTOR_MIN_SPEED;
    }

    // Sæt motor hastigheder
    setLeftMotor(leftSpeed);
    setRightMotor(rightSpeed);

    currentLeftSpeed = leftSpeed;
    currentRightSpeed = rightSpeed;

    #if DEBUG_MOTORS
    Serial.printf("[Motors] Set speed - Left: %d, Right: %d\n", leftSpeed, rightSpeed);
    #endif
}

void Motors::forward(int speed) {
    speed = abs(speed); // Sikrer positiv værdi
    setSpeed(speed, speed);
}

void Motors::backward(int speed) {
    speed = abs(speed); // Sikrer positiv værdi
    setSpeed(-speed, -speed);
}

void Motors::turnLeft(int speed) {
    speed = abs(speed);
    // Venstre motor bakker/stopper, højre motor fremad
    setSpeed(-speed/2, speed);
}

void Motors::turnRight(int speed) {
    speed = abs(speed);
    // Højre motor bakker/stopper, venstre motor fremad
    setSpeed(speed, -speed/2);
}

void Motors::stop() {
    setSpeed(0, 0);

    #if DEBUG_MOTORS
    Serial.println("[Motors] Stopped");
    #endif
}

void Motors::emergencyStop() {
    // Øjeblikkelig stop
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);

    ledcWrite(0, 0);
    ledcWrite(1, 0);

    currentLeftSpeed = 0;
    currentRightSpeed = 0;
    emergencyStopped = true;

    Serial.println("[Motors] !!! EMERGENCY STOP ACTIVATED !!!");
}

bool Motors::isMoving() {
    return (currentLeftSpeed != 0 || currentRightSpeed != 0);
}

int Motors::getLeftSpeed() {
    return currentLeftSpeed;
}

int Motors::getRightSpeed() {
    return currentRightSpeed;
}

void Motors::setLeftMotor(int speed) {
    speed = constrainSpeed(speed);

    if (speed > 0) {
        // Fremad
        digitalWrite(MOTOR_LEFT_IN1, HIGH);
        digitalWrite(MOTOR_LEFT_IN2, LOW);
        ledcWrite(0, speed);
    } else if (speed < 0) {
        // Baglæns
        digitalWrite(MOTOR_LEFT_IN1, LOW);
        digitalWrite(MOTOR_LEFT_IN2, HIGH);
        ledcWrite(0, abs(speed));
    } else {
        // Stop
        digitalWrite(MOTOR_LEFT_IN1, LOW);
        digitalWrite(MOTOR_LEFT_IN2, LOW);
        ledcWrite(0, 0);
    }
}

void Motors::setRightMotor(int speed) {
    speed = constrainSpeed(speed);

    if (speed > 0) {
        // Fremad
        digitalWrite(MOTOR_RIGHT_IN1, HIGH);
        digitalWrite(MOTOR_RIGHT_IN2, LOW);
        ledcWrite(1, speed);
    } else if (speed < 0) {
        // Baglæns
        digitalWrite(MOTOR_RIGHT_IN1, LOW);
        digitalWrite(MOTOR_RIGHT_IN2, HIGH);
        ledcWrite(1, abs(speed));
    } else {
        // Stop
        digitalWrite(MOTOR_RIGHT_IN1, LOW);
        digitalWrite(MOTOR_RIGHT_IN2, LOW);
        ledcWrite(1, 0);
    }
}

int Motors::constrainSpeed(int speed) {
    return constrain(speed, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
}
