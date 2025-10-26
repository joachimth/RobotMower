#include "ObstacleAvoidance.h"

ObstacleAvoidance::ObstacleAvoidance() {
    obstacleDetected = false;
    criticalObstacle = false;
    avoidanceDirection = AVOID_LEFT;
    closestObstacleDistance = ULTRASONIC_MAX_DISTANCE;
    sensorsPtr = nullptr;
    lastDetectionTime = 0;
    initialized = false;
}

bool ObstacleAvoidance::begin() {
    reset();
    initialized = true;

    Logger::info("ObstacleAvoidance initialized");

    return true;
}

void ObstacleAvoidance::update(Sensors* sensors) {
    if (!initialized || sensors == nullptr) {
        return;
    }

    sensorsPtr = sensors;

    // Hent sensor data
    float leftDist = sensors->getLeftDistance();
    float middleDist = sensors->getMiddleDistance();
    float rightDist = sensors->getRightDistance();

    // Analyser forhindringer
    analyzeObstacle(leftDist, middleDist, rightDist);

    if (obstacleDetected) {
        lastDetectionTime = millis();

        #if DEBUG_NAVIGATION
        Logger::warning("Obstacle detected - Direction: " +
                       String(avoidanceDirection == AVOID_LEFT ? "LEFT" :
                              avoidanceDirection == AVOID_RIGHT ? "RIGHT" : "BACK") +
                       " | Distance: " + String(closestObstacleDistance) + " cm");
        #endif
    }
}

bool ObstacleAvoidance::hasObstacle() {
    return obstacleDetected;
}

AvoidanceDirection ObstacleAvoidance::getAvoidanceDirection() {
    return avoidanceDirection;
}

float ObstacleAvoidance::getSafeDistance() {
    return closestObstacleDistance;
}

bool ObstacleAvoidance::isCriticalObstacle() {
    return criticalObstacle;
}

void ObstacleAvoidance::reset() {
    obstacleDetected = false;
    criticalObstacle = false;
    avoidanceDirection = AVOID_LEFT;
    closestObstacleDistance = ULTRASONIC_MAX_DISTANCE;
}

int ObstacleAvoidance::getRecommendedSpeed(int normalSpeed) {
    if (!obstacleDetected) {
        return normalSpeed;
    }

    // Reducer hastighed baseret på afstand til forhindring
    if (criticalObstacle) {
        return 0; // Stop øjeblikkeligt
    }

    if (closestObstacleDistance < OBSTACLE_THRESHOLD) {
        // Jo tættere på, jo langsommere
        float speedFactor = closestObstacleDistance / OBSTACLE_THRESHOLD;
        speedFactor = constrain(speedFactor, 0.3, 1.0);
        return (int)(normalSpeed * speedFactor);
    }

    return normalSpeed;
}

void ObstacleAvoidance::analyzeObstacle(float left, float middle, float right) {
    // Find minimum afstand
    closestObstacleDistance = min(min(left, middle), right);

    // Tjek for forhindringer
    bool leftObstacle = (left < OBSTACLE_THRESHOLD && left > 0);
    bool middleObstacle = (middle < OBSTACLE_THRESHOLD && middle > 0);
    bool rightObstacle = (right < OBSTACLE_THRESHOLD && right > 0);

    obstacleDetected = leftObstacle || middleObstacle || rightObstacle;

    // Tjek for kritisk forhindring
    criticalObstacle = (closestObstacleDistance < OBSTACLE_CRITICAL_DISTANCE && closestObstacleDistance > 0);

    if (!obstacleDetected) {
        return;
    }

    // Bestem bedste undgåelses retning
    avoidanceDirection = findBestDirection(left, middle, right);
}

AvoidanceDirection ObstacleAvoidance::findBestDirection(float left, float middle, float right) {
    // Hvis forhindring direkte foran
    if (middle < OBSTACLE_THRESHOLD && middle > 0) {
        // Vælg side med mest plads
        if (left > right) {
            return AVOID_LEFT;
        } else {
            return AVOID_RIGHT;
        }
    }

    // Hvis forhindring til venstre
    if (left < OBSTACLE_THRESHOLD && left > 0) {
        // Er højre fri?
        if (right > OBSTACLE_THRESHOLD) {
            return AVOID_RIGHT;
        } else {
            return AVOID_BACK; // Begge sider blokeret
        }
    }

    // Hvis forhindring til højre
    if (right < OBSTACLE_THRESHOLD && right > 0) {
        // Er venstre fri?
        if (left > OBSTACLE_THRESHOLD) {
            return AVOID_LEFT;
        } else {
            return AVOID_BACK; // Begge sider blokeret
        }
    }

    // Hvis alle sensorer ser forhindringer, bak
    if (left < OBSTACLE_THRESHOLD && middle < OBSTACLE_THRESHOLD && right < OBSTACLE_THRESHOLD) {
        return AVOID_BACK;
    }

    // Default: drej venstre
    return AVOID_LEFT;
}
