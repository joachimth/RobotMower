#ifndef OBSTACLE_AVOIDANCE_H
#define OBSTACLE_AVOIDANCE_H

#include <Arduino.h>
#include "../config/Config.h"
#include "../hardware/Sensors.h"
#include "../system/Logger.h"

/**
 * ObstacleAvoidance klasse - Detekterer og undgår forhindringer
 *
 * Denne klasse analyserer sensor data og beslutter hvordan
 * forhindringer skal undgås.
 */
class ObstacleAvoidance {
public:
    /**
     * Constructor
     */
    ObstacleAvoidance();

    /**
     * Initialiserer obstacle avoidance
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Opdaterer obstacle detection baseret på sensor data
     * @param sensors Pointer til Sensors objekt
     */
    void update(Sensors* sensors);

    /**
     * Tjek om der er forhindring
     * @return true hvis forhindring detekteret
     */
    bool hasObstacle();

    /**
     * Hent undgåelses retning
     * @return AVOID_LEFT, AVOID_RIGHT, eller AVOID_BACK
     */
    AvoidanceDirection getAvoidanceDirection();

    /**
     * Hent mindste sikre afstand til forhindring
     * @return Afstand i cm
     */
    float getSafeDistance();

    /**
     * Tjek om forhindring er kritisk (meget tæt på)
     * @return true hvis kritisk
     */
    bool isCriticalObstacle();

    /**
     * Nulstil obstacle detection
     */
    void reset();

    /**
     * Hent anbefalet hastighed baseret på forhindringer
     * @param normalSpeed Normal hastighed
     * @return Justeret hastighed
     */
    int getRecommendedSpeed(int normalSpeed);

private:
    /**
     * Analyserer sensor data og beslutter undgåelses strategi
     * @param left Venstre sensor afstand
     * @param middle Midter sensor afstand
     * @param right Højre sensor afstand
     */
    void analyzeObstacle(float left, float middle, float right);

    /**
     * Finder den mest åbne retning
     * @param left Venstre sensor afstand
     * @param middle Midter sensor afstand
     * @param right Højre sensor afstand
     * @return Bedste retning at gå
     */
    AvoidanceDirection findBestDirection(float left, float middle, float right);

    // Obstacle detection state
    bool obstacleDetected;
    bool criticalObstacle;
    AvoidanceDirection avoidanceDirection;
    float closestObstacleDistance;

    // Sensors pointer
    Sensors* sensorsPtr;

    // Timing
    unsigned long lastDetectionTime;

    // State
    bool initialized;
};

#endif // OBSTACLE_AVOIDANCE_H
