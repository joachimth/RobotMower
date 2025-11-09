#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>
#include "../config/Config.h"
#include "Logger.h"

/**
 * RobotState enum - Definerer alle robot tilstande
 */
enum RobotState {
    STATE_IDLE,         // Inaktiv, venter på kommando
    STATE_MANUAL,       // Manuel kontrol
    STATE_CALIBRATING,  // Kalibrerer sensorer
    STATE_MOWING,       // Klipper aktivt
    STATE_TURNING,      // Drejer
    STATE_AVOIDING,     // Undgår forhindring
    STATE_RETURNING,    // Vender tilbage til base
    STATE_CHARGING,     // Lader batteri
    STATE_ERROR         // Fejltilstand
};

/**
 * StateManager klasse - Håndterer robot state machine
 *
 * Denne klasse kontrollerer robot tilstande og overgange mellem dem.
 */
class StateManager {
public:
    /**
     * Constructor
     */
    StateManager();

    /**
     * Initialiserer state manager
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Opdaterer state machine
     * Kalder denne regelmæssigt i loop()
     */
    void update();

    /**
     * Sætter ny robot tilstand
     * @param newState Ny tilstand
     */
    void setState(RobotState newState);

    /**
     * Hent nuværende tilstand
     * @return Nuværende state
     */
    RobotState getState();

    /**
     * Hent forrige tilstand
     * @return Forrige state
     */
    RobotState getPreviousState();

    /**
     * Hent tilstands navn som string
     * @return String navn
     */
    String getStateName();

    /**
     * Hent tilstands navn for specifik state
     * @param state Tilstand
     * @return String navn
     */
    String getStateName(RobotState state);

    /**
     * Starter klipning
     */
    void startMowing();

    /**
     * Pauser klipning
     */
    void pauseMowing();

    /**
     * Stopper klipning
     */
    void stopMowing();

    /**
     * Håndterer fejl
     * @param errorMessage Fejlbesked
     */
    void handleError(String errorMessage);

    /**
     * Genoptager fra fejl tilstand
     */
    void recoverFromError();

    /**
     * Tjek om robotten er aktiv (mower, turning, avoiding)
     * @return true hvis aktiv
     */
    bool isActive();

    /**
     * Tjek om der er fejl
     * @return true hvis i fejltilstand
     */
    bool hasError();

    /**
     * Hent fejlbesked
     * @return Fejlbesked (tom hvis ingen fejl)
     */
    String getErrorMessage();

    /**
     * Hent tid i nuværende tilstand
     * @return Millisekunder i tilstand
     */
    unsigned long getTimeInState();

private:
    /**
     * Kaldes når state ændres
     * @param oldState Gammel tilstand
     * @param newState Ny tilstand
     */
    void onStateEnter(RobotState oldState, RobotState newState);

    /**
     * Kaldes når state forlades
     * @param state Tilstand der forlades
     */
    void onStateExit(RobotState state);

    /**
     * Tjek for state timeout
     */
    void checkStateTimeout();

    // Tilstande
    RobotState currentState;
    RobotState previousState;

    // Timing
    unsigned long stateStartTime;

    // Fejl info
    String lastError;

    // Initialization flag
    bool initialized;
};

#endif // STATE_MANAGER_H
