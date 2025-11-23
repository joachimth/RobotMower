#include "StateManager.h"

StateManager::StateManager() {
    currentState = STATE_IDLE;
    previousState = STATE_IDLE;
    stateStartTime = 0;
    lastError = "";
    initialized = false;
}

bool StateManager::begin() {
    currentState = STATE_IDLE;
    previousState = STATE_IDLE;
    stateStartTime = millis();
    initialized = true;

    Logger::info("StateManager initialized - State: IDLE");

    return true;
}

void StateManager::update() {
    if (!initialized) {
        return;
    }

    // Tjek for timeout i nuværende state
    checkStateTimeout();

    // State-specifik logik kan tilføjes her hvis nødvendigt
}

void StateManager::setState(RobotState newState) {
    if (!initialized) {
        return;
    }

    if (currentState == newState) {
        return; // Ingen ændring
    }

    // Gem forrige state
    previousState = currentState;

    // Exit handler for gammel state
    onStateExit(currentState);

    // Skift til ny state
    currentState = newState;
    stateStartTime = millis();

    // Enter handler for ny state
    onStateEnter(previousState, currentState);

    // Log state change
    String logMsg = "State changed: ";
    logMsg += getStateName(previousState);
    logMsg += " -> ";
    logMsg += getStateName(currentState);
    Logger::info(logMsg);
}

RobotState StateManager::getState() {
    return currentState;
}

RobotState StateManager::getPreviousState() {
    return previousState;
}

String StateManager::getStateName() {
    return getStateName(currentState);
}

String StateManager::getStateName(RobotState state) {
    switch (state) {
        case STATE_IDLE:            return "IDLE";
        case STATE_MANUAL:          return "MANUAL";
        case STATE_CALIBRATING:     return "CALIBRATING";
        case STATE_MOWING:          return "MOWING";
        case STATE_TURNING:         return "TURNING";
        case STATE_AVOIDING:        return "AVOIDING";
        case STATE_RETURNING:       return "RETURNING";
        case STATE_SEARCHING_SIGNAL: return "SEARCHING_SIGNAL";
        case STATE_CHARGING:        return "CHARGING";
        case STATE_ERROR:           return "ERROR";
        default:                    return "UNKNOWN";
    }
}

void StateManager::startMowing() {
    if (currentState == STATE_ERROR) {
        Logger::warning("Cannot start mowing - Robot in ERROR state");
        return;
    }

    setState(STATE_MOWING);
}

void StateManager::pauseMowing() {
    if (currentState == STATE_MOWING ||
        currentState == STATE_TURNING ||
        currentState == STATE_AVOIDING) {
        setState(STATE_IDLE);
        Logger::info("Mowing paused");
    }
}

void StateManager::stopMowing() {
    if (currentState != STATE_IDLE && currentState != STATE_ERROR) {
        setState(STATE_IDLE);
        Logger::info("Mowing stopped");
    }
}

void StateManager::returnToBase() {
    if (currentState == STATE_ERROR) {
        Logger::warning("Cannot return to base - Robot in ERROR state");
        return;
    }

    setState(STATE_RETURNING);
    Logger::info("Starting return to base - following perimeter");
}

void StateManager::searchForSignal() {
    if (currentState == STATE_ERROR) {
        Logger::warning("Cannot search for signal - Robot in ERROR state");
        return;
    }

    setState(STATE_SEARCHING_SIGNAL);
    Logger::info("Starting signal search pattern");
}

void StateManager::handleError(String errorMessage) {
    lastError = errorMessage;
    setState(STATE_ERROR);
    Logger::error("Robot error: " + errorMessage);
}

void StateManager::recoverFromError() {
    if (currentState == STATE_ERROR) {
        lastError = "";
        setState(STATE_IDLE);
        Logger::info("Recovered from error state");
    }
}

bool StateManager::isActive() {
    return (currentState == STATE_MOWING ||
            currentState == STATE_TURNING ||
            currentState == STATE_AVOIDING ||
            currentState == STATE_RETURNING ||
            currentState == STATE_SEARCHING_SIGNAL);
}

bool StateManager::hasError() {
    return (currentState == STATE_ERROR);
}

String StateManager::getErrorMessage() {
    return lastError;
}

unsigned long StateManager::getTimeInState() {
    return millis() - stateStartTime;
}

void StateManager::onStateEnter(RobotState oldState, RobotState newState) {
    // Handling når der skiftes til ny state

    switch (newState) {
        case STATE_IDLE:
            // Robot går i idle - stop alle aktioner
            break;

        case STATE_MANUAL:
            Logger::info("Entering manual control mode");
            break;

        case STATE_CALIBRATING:
            Logger::info("Starting calibration...");
            break;

        case STATE_MOWING:
            Logger::info("Starting mowing operation");
            break;

        case STATE_TURNING:
            Logger::debug("Executing turn maneuver");
            break;

        case STATE_AVOIDING:
            Logger::warning("Obstacle detected - avoiding");
            break;

        case STATE_RETURNING:
            Logger::info("Returning to base - following perimeter wire");
            break;

        case STATE_SEARCHING_SIGNAL:
            Logger::info("Searching for perimeter signal");
            break;

        case STATE_CHARGING:
            Logger::info("Charging battery");
            break;

        case STATE_ERROR:
            Logger::error("Entered ERROR state");
            break;

        default:
            break;
    }
}

void StateManager::onStateExit(RobotState state) {
    // Handling når der forlades en state

    switch (state) {
        case STATE_MOWING:
            Logger::info("Exiting mowing state");
            break;

        case STATE_CALIBRATING:
            Logger::info("Calibration complete");
            break;

        default:
            break;
    }
}

void StateManager::checkStateTimeout() {
    unsigned long timeInState = getTimeInState();

    // Tjek for timeout i visse states
    if (currentState == STATE_CALIBRATING && timeInState > CALIBRATION_TIMEOUT) {
        handleError("Calibration timeout");
    }

    if (currentState != STATE_IDLE &&
        currentState != STATE_MANUAL &&
        currentState != STATE_CHARGING &&
        currentState != STATE_ERROR) {

        if (timeInState > STATE_TIMEOUT) {
            handleError("State timeout - stuck in " + getStateName());
        }
    }
}
