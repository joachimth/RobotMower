#include "PathPlanner.h"

PathPlanner::PathPlanner() {
    rowWidth = MOWING_PATTERN_WIDTH;
    currentRow = 0;
    totalRows = MAX_ROWS;
    targetHeading = 0.0;
    turningRight = true;
    turning = false;
    nextTurnDir = RIGHT;
    distanceTraveled = 0.0;
    rowStartTime = 0;
    patternActive = false;
    initialized = false;
}

bool PathPlanner::begin() {
    reset();
    initialized = true;

    Logger::info("PathPlanner initialized");

    return true;
}

void PathPlanner::startNewPattern() {
    if (!initialized) {
        return;
    }

    reset();
    patternActive = true;
    rowStartTime = millis();

    Logger::info("Starting new mowing pattern");
    Logger::info("Row width: " + String(rowWidth) + " cm");
}

void PathPlanner::nextRow() {
    if (!patternActive) {
        return;
    }

    currentRow++;
    distanceTraveled = 0.0;
    rowStartTime = millis();

    // Alternér drejningsretning
    turningRight = !turningRight;
    nextTurnDir = turningRight ? RIGHT : LEFT;

    // Beregn ny heading
    calculateNextHeading();

    Logger::info("Moving to row " + String(currentRow) + " - Heading: " + String(targetHeading));

    if (currentRow >= totalRows) {
        Logger::info("Pattern complete!");
        patternActive = false;
    }
}

bool PathPlanner::shouldTurn() {
    if (!patternActive || turning) {
        return false;
    }

    // Tjek om vi har kørt langt nok i nuværende række
    // Baseret på tid eller distance (her bruger vi tid som approksimation)
    unsigned long timeInRow = millis() - rowStartTime;

    // Beregn estimeret distance baseret på tid og hastighed
    // Antager MOTOR_CRUISE_SPEED giver ca. 20 cm/s (juster efter dit setup)
    const float ESTIMATED_SPEED = 20.0; // cm/s
    distanceTraveled = (timeInRow / 1000.0) * ESTIMATED_SPEED;

    // Drej når vi har kørt længde nok eller efter max tid
    if (distanceTraveled >= ROW_LENGTH_MAX || timeInRow >= 30000) { // Max 30 sekunder per række
        return true;
    }

    return false;
}

Direction PathPlanner::getTurnDirection() {
    return nextTurnDir;
}

float PathPlanner::getTargetHeading() {
    return targetHeading;
}

bool PathPlanner::isPatternComplete() {
    return (currentRow >= totalRows || !patternActive);
}

void PathPlanner::setRowWidth(float width) {
    rowWidth = width;
    Logger::info("Row width set to: " + String(width) + " cm");
}

int PathPlanner::getCurrentRow() {
    return currentRow;
}

int PathPlanner::getTotalRows() {
    return totalRows;
}

void PathPlanner::reset() {
    currentRow = 0;
    targetHeading = 0.0;
    turningRight = true;
    turning = false;
    nextTurnDir = RIGHT;
    distanceTraveled = 0.0;
    rowStartTime = 0;
    patternActive = false;

    Logger::debug("PathPlanner reset");
}

void PathPlanner::update() {
    if (!initialized || !patternActive) {
        return;
    }

    // Opdater distance tracking baseret på tid
    unsigned long timeInRow = millis() - rowStartTime;
    const float ESTIMATED_SPEED = 20.0; // cm/s
    distanceTraveled = (timeInRow / 1000.0) * ESTIMATED_SPEED;
}

void PathPlanner::startTurn() {
    turning = true;
    Logger::debug("Turn started - Direction: " + String(nextTurnDir == RIGHT ? "RIGHT" : "LEFT"));
}

void PathPlanner::completeTurn() {
    turning = false;
    Logger::debug("Turn completed");
}

bool PathPlanner::isTurning() {
    return turning;
}

void PathPlanner::calculateNextHeading() {
    // Beregn ny heading baseret på drejningsretning
    // Efter hver række drejer vi 180 grader (90° + fremad + 90°)

    if (currentRow % 2 == 0) {
        // Lige rækker - kør i én retning
        targetHeading = 0.0;
    } else {
        // Ulige rækker - kør i modsat retning
        targetHeading = 180.0;
    }
}
