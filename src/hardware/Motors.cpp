#include "Motors.h"

Motors::Motors() {
    currentLeftSpeed = 0;
    currentRightSpeed = 0;
    leftMotorCurrent = 0.0;
    rightMotorCurrent = 0.0;
    lastCurrentUpdate = 0;
    emergencyStopped = false;
}

bool Motors::begin() {
    // Konfigurer venstre motor pins
    pinMode(MOTOR_LEFT_RPWM, OUTPUT);
    pinMode(MOTOR_LEFT_LPWM, OUTPUT);
    pinMode(MOTOR_LEFT_R_EN, OUTPUT);
    pinMode(MOTOR_LEFT_L_EN, OUTPUT);
    pinMode(MOTOR_LEFT_R_IS, INPUT);
    pinMode(MOTOR_LEFT_L_IS, INPUT);

    // Konfigurer højre motor pins
    pinMode(MOTOR_RIGHT_RPWM, OUTPUT);
    pinMode(MOTOR_RIGHT_LPWM, OUTPUT);
    pinMode(MOTOR_RIGHT_R_EN, OUTPUT);
    pinMode(MOTOR_RIGHT_L_EN, OUTPUT);
    pinMode(MOTOR_RIGHT_R_IS, INPUT);
    pinMode(MOTOR_RIGHT_L_IS, INPUT);

    // Enable begge sider af BTS7960 drivere
    digitalWrite(MOTOR_LEFT_R_EN, HIGH);
    digitalWrite(MOTOR_LEFT_L_EN, HIGH);
    digitalWrite(MOTOR_RIGHT_R_EN, HIGH);
    digitalWrite(MOTOR_RIGHT_L_EN, HIGH);

    // Konfigurer PWM kanaler (ESP32 Arduino Core 3.0+ API)
    // BTS7960 bruger separate PWM kanaler for fremad/baglæns
    // Det nye API bruger ledcAttach() der returnerer channel nummer automatisk
    ledcAttach(MOTOR_LEFT_RPWM, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcAttach(MOTOR_LEFT_LPWM, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcAttach(MOTOR_RIGHT_RPWM, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);
    ledcAttach(MOTOR_RIGHT_LPWM, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION);

    // Konfigurer ADC for strømsensorer
    analogReadResolution(12); // 12-bit ADC
    analogSetAttenuation(ADC_11db); // 0-3.3V range

    // Initialiser motorer til stop
    stop();

    #if DEBUG_MOTORS
    Serial.println("[Motors] BTS7960 Driver initialized successfully");
    Serial.println("[Motors] Current monitoring enabled");
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
    // Øjeblikkelig stop - stop alle PWM signaler
    ledcWrite(MOTOR_LEFT_RPWM, 0);
    ledcWrite(MOTOR_LEFT_LPWM, 0);
    ledcWrite(MOTOR_RIGHT_RPWM, 0);
    ledcWrite(MOTOR_RIGHT_LPWM, 0);

    // Disable drivers (optional - for ekstra sikkerhed)
    digitalWrite(MOTOR_LEFT_R_EN, LOW);
    digitalWrite(MOTOR_LEFT_L_EN, LOW);
    digitalWrite(MOTOR_RIGHT_R_EN, LOW);
    digitalWrite(MOTOR_RIGHT_L_EN, LOW);

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

float Motors::getLeftCurrent() {
    return leftMotorCurrent;
}

float Motors::getRightCurrent() {
    return rightMotorCurrent;
}

float Motors::getTotalCurrent() {
    return leftMotorCurrent + rightMotorCurrent;
}

void Motors::updateCurrentReadings() {
    // Opdater kun hver 100ms for at undgå for mange ADC læsninger
    unsigned long currentTime = millis();
    if (currentTime - lastCurrentUpdate < 100) {
        return;
    }
    lastCurrentUpdate = currentTime;

    // Læs strøm fra begge motorer
    // Vælg den højeste værdi fra R_IS eller L_IS afhængig af retning
    if (currentLeftSpeed >= 0) {
        leftMotorCurrent = readCurrent(MOTOR_LEFT_R_IS);
    } else {
        leftMotorCurrent = readCurrent(MOTOR_LEFT_L_IS);
    }

    if (currentRightSpeed >= 0) {
        rightMotorCurrent = readCurrent(MOTOR_RIGHT_R_IS);
    } else {
        rightMotorCurrent = readCurrent(MOTOR_RIGHT_L_IS);
    }

    #if DEBUG_MOTORS
    static int currentLogCounter = 0;
    if (++currentLogCounter >= 50) { // Log hver 5 sekund
        Serial.printf("[Motors] Current - Left: %.2fA, Right: %.2fA, Total: %.2fA\n",
                     leftMotorCurrent, rightMotorCurrent, getTotalCurrent());
        currentLogCounter = 0;
    }
    #endif
}

bool Motors::isCurrentWarning() {
    return (leftMotorCurrent > MOTOR_CURRENT_WARNING ||
            rightMotorCurrent > MOTOR_CURRENT_WARNING);
}

void Motors::setLeftMotor(int speed) {
    speed = constrainSpeed(speed);

    if (speed > 0) {
        // Fremad - brug RPWM (Right PWM)
        ledcWrite(MOTOR_LEFT_RPWM, speed);
        ledcWrite(MOTOR_LEFT_LPWM, 0);
    } else if (speed < 0) {
        // Baglæns - brug LPWM (Left PWM)
        ledcWrite(MOTOR_LEFT_RPWM, 0);
        ledcWrite(MOTOR_LEFT_LPWM, abs(speed));
    } else {
        // Stop - begge PWM til 0
        ledcWrite(MOTOR_LEFT_RPWM, 0);
        ledcWrite(MOTOR_LEFT_LPWM, 0);
    }
}

void Motors::setRightMotor(int speed) {
    speed = constrainSpeed(speed);

    if (speed > 0) {
        // Fremad - brug RPWM (Right PWM)
        ledcWrite(MOTOR_RIGHT_RPWM, speed);
        ledcWrite(MOTOR_RIGHT_LPWM, 0);
    } else if (speed < 0) {
        // Baglæns - brug LPWM (Left PWM)
        ledcWrite(MOTOR_RIGHT_RPWM, 0);
        ledcWrite(MOTOR_RIGHT_LPWM, abs(speed));
    } else {
        // Stop - begge PWM til 0
        ledcWrite(MOTOR_RIGHT_RPWM, 0);
        ledcWrite(MOTOR_RIGHT_LPWM, 0);
    }
}

int Motors::constrainSpeed(int speed) {
    return constrain(speed, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
}

float Motors::readCurrent(int pin) {
    // Læs ADC værdi med gennemsnit af flere samples
    float sum = 0;
    for (int i = 0; i < MOTOR_CURRENT_SAMPLES; i++) {
        sum += analogRead(pin);
        delayMicroseconds(100);
    }
    float avgADC = sum / MOTOR_CURRENT_SAMPLES;

    // Konverter ADC værdi til spænding
    float voltage = (avgADC / CURRENT_ADC_MAX) * CURRENT_ADC_VREF;

    // Konverter spænding til strøm
    // BTS7960 current sense: 10mV/A (0.01V/A)
    float current = voltage / CURRENT_SENSE_RATIO;

    return current;
}
