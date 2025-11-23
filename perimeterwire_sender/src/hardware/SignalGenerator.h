#ifndef SIGNAL_GENERATOR_H
#define SIGNAL_GENERATOR_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * SignalGenerator - Genererer perimeter wire signal
 *
 * Genererer et pseudo-random signal ved 3.2 kHz carrier frekvens
 * kompatibelt med Ardumower standard.
 *
 * Signalet skifter mellem høj (4808 Hz) og lav (2404 Hz) pulsbredde
 * baseret på en kodet sekvens.
 */
class SignalGenerator {
public:
    SignalGenerator();

    /**
     * Initialiserer signal generatoren
     * @return true hvis initialisering lykkedes
     */
    bool begin();

    /**
     * Starter signal udsendelse
     */
    void start();

    /**
     * Stopper signal udsendelse
     */
    void stop();

    /**
     * Opdaterer signal generatoren (skal kaldes i loop)
     */
    void update();

    /**
     * Tjekker om signalet kører
     * @return true hvis signal udsendes
     */
    bool isRunning() const { return _running; }

    /**
     * Henter nuværende tilstand
     * @return SenderState enum værdi
     */
    SenderState getState() const { return _state; }

    /**
     * Henter tilstand som tekst
     * @return Tilstandsnavn som String
     */
    String getStateString() const;

    /**
     * Henter strømmåling i mA
     * @return Strøm i milliampere
     */
    float getCurrentMA() const { return _currentMA; }

    /**
     * Tjekker for overstrøm
     * @return true hvis strømmen er over grænsen
     */
    bool isOvercurrent() const { return _currentMA > CURRENT_MAX_MA; }

    /**
     * Nulstiller efter fejl
     */
    void reset();

    /**
     * Henter signal statistik
     */
    unsigned long getRuntime() const;
    unsigned long getCycleCount() const { return _cycleCount; }

private:
    bool _running;
    bool _initialized;
    SenderState _state;

    // Signal timing
    unsigned long _lastBitTime;
    uint8_t _currentBitIndex;
    bool _currentPolarity;

    // Strømmåling
    float _currentMA;
    float _currentSamples[CURRENT_SAMPLES];
    uint8_t _currentSampleIndex;
    unsigned long _lastCurrentCheck;

    // Statistik
    unsigned long _startTime;
    unsigned long _cycleCount;

    // Private metoder
    void outputHighFrequency();
    void outputLowFrequency();
    void outputOff();
    void updateCurrentReading();
    void checkOvercurrent();
    void generateNextBit();
};

#endif // SIGNAL_GENERATOR_H
