#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * Motors klasse - Håndterer begge drive motorer
 *
 * Denne klasse kontrollerer venstre og højre motor via Double BTS7960 43A H-bridge.
 * Hastighed fra -255 (fuld baglæns) til 255 (fuld fremad).
 * Inkluderer strømovervågning via current sense pins.
 */
class Motors {
public:
    /**
     * Constructor
     */
    Motors();

    /**
     * Initialiserer motor pins og PWM
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Sætter individuel hastighed for begge motorer
     * @param leftSpeed Venstre motor hastighed (-255 til 255)
     * @param rightSpeed Højre motor hastighed (-255 til 255)
     */
    void setSpeed(int leftSpeed, int rightSpeed);

    /**
     * Kører begge motorer fremad
     * @param speed Hastighed (0-255)
     */
    void forward(int speed);

    /**
     * Kører begge motorer baglæns
     * @param speed Hastighed (0-255)
     */
    void backward(int speed);

    /**
     * Drejer til venstre (venstre motor langsom/back, højre motor fremad)
     * @param speed Hastighed (0-255)
     */
    void turnLeft(int speed);

    /**
     * Drejer til højre (højre motor langsom/back, venstre motor fremad)
     * @param speed Hastighed (0-255)
     */
    void turnRight(int speed);

    /**
     * Stopper begge motorer
     */
    void stop();

    /**
     * Nødstop - stopper øjeblikkeligt og deaktiverer motorer
     */
    void emergencyStop();

    /**
     * Tjek om motorerne kører
     * @return true hvis mindst én motor har hastighed > 0
     */
    bool isMoving();

    /**
     * Hent nuværende venstre motor hastighed
     * @return Hastighed (-255 til 255)
     */
    int getLeftSpeed();

    /**
     * Hent nuværende højre motor hastighed
     * @return Hastighed (-255 til 255)
     */
    int getRightSpeed();

    /**
     * Hent venstre motor strøm
     * @return Strøm i Ampere
     */
    float getLeftCurrent();

    /**
     * Hent højre motor strøm
     * @return Strøm i Ampere
     */
    float getRightCurrent();

    /**
     * Opdater strømmålinger
     * Kalder denne regelmæssigt i loop()
     */
    void updateCurrentReadings();

    /**
     * Tjek om strømmen er for høj
     * @return true hvis over advarsel tærskel
     */
    bool isCurrentWarning();

    /**
     * Hent total strøm (begge motorer)
     * @return Total strøm i Ampere
     */
    float getTotalCurrent();

private:
    /**
     * Sætter venstre motor hastighed og retning
     * @param speed Hastighed (-255 til 255)
     */
    void setLeftMotor(int speed);

    /**
     * Sætter højre motor hastighed og retning
     * @param speed Hastighed (-255 til 255)
     */
    void setRightMotor(int speed);

    /**
     * Begrænser hastighed til gyldigt interval
     * @param speed Hastighed at begrænse
     * @return Begrænset hastighed (-255 til 255)
     */
    int constrainSpeed(int speed);

    /**
     * Læs strøm fra analog pin med gennemsnit
     * @param pin ADC pin at læse
     * @return Strøm i Ampere
     */
    float readCurrent(int pin);

    // Nuværende motor hastigheder
    int currentLeftSpeed;
    int currentRightSpeed;

    // Strømmålinger
    float leftMotorCurrent;
    float rightMotorCurrent;
    unsigned long lastCurrentUpdate;

    // Emergency stop flag
    bool emergencyStopped;

    // PWM kanaler (ESP32 har 16 PWM kanaler 0-15)
    static const uint8_t PWM_CHANNEL_LEFT_RPWM = 0;
    static const uint8_t PWM_CHANNEL_LEFT_LPWM = 1;
    static const uint8_t PWM_CHANNEL_RIGHT_RPWM = 2;
    static const uint8_t PWM_CHANNEL_RIGHT_LPWM = 3;
};

#endif // MOTORS_H
