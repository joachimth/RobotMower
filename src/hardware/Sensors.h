#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * Sensors klasse - Håndterer alle tre ultralyd sensorer
 *
 * Denne klasse læser HC-SR04 ultralyd sensorer non-blocking.
 * Sensorer opdateres med jævne mellemrum og data caches.
 */
class Sensors {
public:
    /**
     * Constructor
     */
    Sensors();

    /**
     * Initialiserer sensor pins
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Opdaterer sensor målinger (non-blocking)
     * Kalder denne regelmæssigt i loop()
     */
    void update();

    /**
     * Hent venstre sensor afstand
     * @return Afstand i cm (0 hvis fejl)
     */
    float getLeftDistance();

    /**
     * Hent midter sensor afstand
     * @return Afstand i cm (0 hvis fejl)
     */
    float getMiddleDistance();

    /**
     * Hent højre sensor afstand
     * @return Afstand i cm (0 hvis fejl)
     */
    float getRightDistance();

    /**
     * Tjek om der er forhindring til venstre
     * @return true hvis forhindring indenfor threshold
     */
    bool isObstacleLeft();

    /**
     * Tjek om der er forhindring foran
     * @return true hvis forhindring indenfor threshold
     */
    bool isObstacleMiddle();

    /**
     * Tjek om der er forhindring til højre
     * @return true hvis forhindring indenfor threshold
     */
    bool isObstacleRight();

    /**
     * Tjek om der er nogen forhindring
     * @return true hvis mindst én sensor detekterer forhindring
     */
    bool isAnyObstacle();

    /**
     * Hent minimum afstand af alle sensorer
     * @return Minimum afstand i cm
     */
    float getMinDistance();

    /**
     * Print sensor værdier til Serial (debug)
     */
    void printValues();

private:
    /**
     * Måler afstand med en ultralyd sensor
     * @param trigPin Trigger pin nummer
     * @param echoPin Echo pin nummer
     * @return Afstand i cm (0 hvis timeout/fejl)
     */
    float measureDistance(int trigPin, int echoPin);

    /**
     * Validerer en målt afstand
     * @param distance Afstand at validere
     * @return true hvis gyldig, false hvis fejl
     */
    bool isValidDistance(float distance);

    // Cachede sensor værdier
    float leftDistance;
    float middleDistance;
    float rightDistance;

    // Timing
    unsigned long lastUpdate;

    // Sensor tilstand
    bool sensorsInitialized;
};

#endif // SENSORS_H
