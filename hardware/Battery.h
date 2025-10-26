#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * Battery klasse - Overvåger batteri status
 *
 * Denne klasse læser batteri spænding via ADC og beregner
 * batteri niveau og status.
 */
class Battery {
public:
    /**
     * Constructor
     */
    Battery();

    /**
     * Initialiserer batteri monitoring
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Opdaterer batteri målinger
     * Kalder denne regelmæssigt i loop()
     */
    void update();

    /**
     * Hent batteri spænding
     * @return Spænding i Volt
     */
    float getVoltage();

    /**
     * Hent batteri niveau i procent
     * @return Procent (0-100)
     */
    int getPercentage();

    /**
     * Tjek om batteri er lavt
     * @return true hvis under LOW_VOLTAGE threshold
     */
    bool isLow();

    /**
     * Tjek om batteri er kritisk lavt
     * @return true hvis under CRITICAL_VOLTAGE threshold
     */
    bool isCritical();

    /**
     * Print batteri info til Serial (debug)
     */
    void printStatus();

    /**
     * Hent estimeret resterende tid i minutter
     * @param currentDraw Strømforbrug i Ampere
     * @return Estimeret minutter tilbage
     */
    int getEstimatedTimeRemaining(float currentDraw);

private:
    /**
     * Læser rå ADC værdi og konverterer til spænding
     * @return Spænding i Volt
     */
    float readVoltage();

    /**
     * Beregner procent fra spænding
     * @param voltage Spænding
     * @return Procent (0-100)
     */
    int calculatePercentage(float voltage);

    // Nuværende batteri data
    float voltage;
    int percentage;

    // Timing
    unsigned long lastUpdate;

    // Tilstand
    bool initialized;
    bool lowWarningShown;
    bool criticalWarningShown;
};

#endif // BATTERY_H
