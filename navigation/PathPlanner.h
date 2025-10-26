#ifndef PATH_PLANNER_H
#define PATH_PLANNER_H

#include <Arduino.h>
#include "../config/Config.h"
#include "../system/Logger.h"

/**
 * PathPlanner klasse - Planlægger systematisk klipningsmønster
 *
 * Denne klasse implementerer et parallelt række-mønster for
 * systematisk plæneklipning.
 */
class PathPlanner {
public:
    /**
     * Constructor
     */
    PathPlanner();

    /**
     * Initialiserer path planner
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Starter nyt klipningsmønster
     */
    void startNewPattern();

    /**
     * Går til næste række i mønsteret
     */
    void nextRow();

    /**
     * Tjek om robot skal dreje
     * @return true hvis det er tid til at dreje
     */
    bool shouldTurn();

    /**
     * Hent drejningsretning
     * @return LEFT eller RIGHT
     */
    Direction getTurnDirection();

    /**
     * Hent target heading for nuværende række
     * @return Heading i grader (0-360)
     */
    float getTargetHeading();

    /**
     * Tjek om mønster er komplet
     * @return true hvis alle rækker er færdige
     */
    bool isPatternComplete();

    /**
     * Sæt række bredde
     * @param width Bredde mellem rækker i cm
     */
    void setRowWidth(float width);

    /**
     * Hent nuværende række nummer
     * @return Række nummer (0-baseret)
     */
    int getCurrentRow();

    /**
     * Hent total antal rækker
     * @return Antal rækker i mønster
     */
    int getTotalRows();

    /**
     * Nulstil path planner
     */
    void reset();

    /**
     * Opdater path planner state
     */
    void update();

    /**
     * Marker at drejning er startet
     */
    void startTurn();

    /**
     * Marker at drejning er færdig
     */
    void completeTurn();

    /**
     * Tjek om der drejes i øjeblikket
     * @return true hvis drejer
     */
    bool isTurning();

private:
    /**
     * Beregner target heading for næste række
     */
    void calculateNextHeading();

    // Mønster parametre
    float rowWidth;           // Afstand mellem rækker (cm)
    int currentRow;           // Nuværende række nummer
    int totalRows;            // Total antal rækker
    float targetHeading;      // Target heading for nuværende række

    // Drejnings håndtering
    bool turningRight;        // true = drejer til højre, false = venstre
    bool turning;             // true hvis drejer i øjeblikket
    Direction nextTurnDir;    // Næste drejnings retning

    // Distance tracking
    float distanceTraveled;   // Afstand kørt i nuværende række
    unsigned long rowStartTime; // Tidspunkt for række start

    // State
    bool patternActive;
    bool initialized;
};

#endif // PATH_PLANNER_H
