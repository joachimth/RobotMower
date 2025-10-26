#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * Motors klasse - Håndterer begge drive motorer
 *
 * Denne klasse kontrollerer venstre og højre motor via L298N motor driver.
 * Hastighed fra -255 (fuld baglæns) til 255 (fuld fremad).
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

    // Nuværende motor hastigheder
    int currentLeftSpeed;
    int currentRightSpeed;

    // Emergency stop flag
    bool emergencyStopped;
};

#endif // MOTORS_H
