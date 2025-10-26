#ifndef CUTTING_MECHANISM_H
#define CUTTING_MECHANISM_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * CuttingMechanism klasse - Kontrollerer klippermotor via relay
 *
 * Denne klasse håndterer tænd/sluk af klippemekanismen via et relay.
 * Vigtige sikkerhedsfunktioner inkluderet.
 */
class CuttingMechanism {
public:
    /**
     * Constructor
     */
    CuttingMechanism();

    /**
     * Initialiserer cutting mechanism
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Starter klippermotor
     */
    void start();

    /**
     * Stopper klippermotor
     */
    void stop();

    /**
     * Tjek om klippermotor kører
     * @return true hvis kører
     */
    bool isRunning();

    /**
     * Nødstop af klippermotor
     */
    void emergencyStop();

    /**
     * Aktiver/deaktiver sikkerhedslock
     * Når locked, kan motor ikke startes
     * @param locked true = locked, false = unlocked
     */
    void setSafetyLock(bool locked);

    /**
     * Tjek om safety lock er aktiv
     * @return true hvis locked
     */
    bool isSafetyLocked();

private:
    bool running;
    bool safetyLocked;
    bool initialized;
};

#endif // CUTTING_MECHANISM_H
