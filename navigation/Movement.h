#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <Arduino.h>
#include "../config/Config.h"
#include "../hardware/Motors.h"
#include "../hardware/IMU.h"
#include "../system/Logger.h"
#include "../utils/Math.h"

/**
 * Movement klasse - Eksekverer bevægelseskommandoer
 *
 * Denne klasse håndterer højniveau bevægelser som at køre lige,
 * dreje til specifik heading, osv.
 */
class Movement {
public:
    /**
     * Constructor
     */
    Movement();

    /**
     * Initialiserer movement controller
     * @param motors Pointer til Motors objekt
     * @param imu Pointer til IMU objekt
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin(Motors* motors, IMU* imu);

    /**
     * Kører lige fremad med automatisk kurs korrektion
     * @param speed Hastighed (0-255)
     */
    void driveStraight(int speed);

    /**
     * Drejer til specifik heading
     * @param targetHeading Mål heading i grader (0-360)
     * @return true hvis heading er nået
     */
    bool turnToHeading(float targetHeading);

    /**
     * Bakker en specifik distance
     * @param distance Distance i cm (cirka)
     */
    void backUp(int distance);

    /**
     * Stopper al bevægelse
     */
    void stop();

    /**
     * Drej på stedet
     * @param direction LEFT eller RIGHT
     * @param speed Drejnings hastighed
     */
    void turnInPlace(Direction direction, int speed);

    /**
     * Tjek om robotten bevæger sig
     * @return true hvis bevæger sig
     */
    bool isMoving();

    /**
     * Tjek om robotten drejer
     * @return true hvis drejer
     */
    bool isTurning();

    /**
     * Sæt target heading for lige kørsel
     * @param heading Target heading (0-360)
     */
    void setTargetHeading(float heading);

    /**
     * Hent nuværende target heading
     * @return Target heading
     */
    float getTargetHeading();

    /**
     * Opdater movement controller
     * Kalder denne regelmæssigt i loop()
     */
    void update();

private:
    /**
     * Korrigerer kurs for at holde target heading
     * @param currentHeading Nuværende heading
     * @param targetHeading Mål heading
     */
    void correctDrift(float currentHeading, float targetHeading);

    /**
     * Simpel PID controller til heading korrektion
     * @param error Heading fejl
     * @return Korrektions værdi
     */
    int calculatePIDCorrection(float error);

    // Hardware pointere
    Motors* motorsPtr;
    IMU* imuPtr;

    // Movement state
    float targetHeading;
    bool turningActive;
    bool movingForward;
    bool movingBackward;

    // PID kontrol variabler
    float lastError;
    float integralError;

    // Timing
    unsigned long lastUpdate;

    // State
    bool initialized;
};

#endif // MOVEMENT_H
