#include "CuttingMechanism.h"

CuttingMechanism::CuttingMechanism() {
    running = false;
    safetyLocked = true; // Start med safety lock aktiveret
    initialized = false;
}

bool CuttingMechanism::begin() {
    // Konfigurer relay pin som output
    pinMode(CUTTING_RELAY, OUTPUT);

    // Sikre at relay er OFF ved opstart
    digitalWrite(CUTTING_RELAY, LOW);

    initialized = true;

    Serial.println("[CuttingMechanism] Initialized successfully");
    Serial.println("[CuttingMechanism] Safety lock is ACTIVE - unlock before starting");

    return true;
}

void CuttingMechanism::start() {
    if (!initialized) {
        Serial.println("[CuttingMechanism] Error: Not initialized");
        return;
    }

    if (safetyLocked) {
        Serial.println("[CuttingMechanism] Cannot start - Safety lock is active!");
        return;
    }

    if (running) {
        Serial.println("[CuttingMechanism] Already running");
        return;
    }

    // Aktiver relay
    digitalWrite(CUTTING_RELAY, HIGH);
    running = true;

    Serial.println("[CuttingMechanism] Started");
}

void CuttingMechanism::stop() {
    if (!initialized) {
        return;
    }

    if (!running) {
        return;
    }

    // Deaktiver relay
    digitalWrite(CUTTING_RELAY, LOW);
    running = false;

    Serial.println("[CuttingMechanism] Stopped");
}

bool CuttingMechanism::isRunning() {
    return running;
}

void CuttingMechanism::emergencyStop() {
    // Øjeblikkelig stop - ignorer safety lock
    digitalWrite(CUTTING_RELAY, LOW);
    running = false;
    safetyLocked = true; // Aktiver safety lock

    Serial.println("[CuttingMechanism] !!! EMERGENCY STOP ACTIVATED !!!");
}

void CuttingMechanism::setSafetyLock(bool locked) {
    safetyLocked = locked;

    if (locked) {
        // Hvis vi låser, stop automatisk
        stop();
        Serial.println("[CuttingMechanism] Safety lock ACTIVATED");
    } else {
        Serial.println("[CuttingMechanism] Safety lock DEACTIVATED");
    }
}

bool CuttingMechanism::isSafetyLocked() {
    return safetyLocked;
}
