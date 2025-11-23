#ifndef PERIMETER_RECEIVER_H
#define PERIMETER_RECEIVER_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * PerimeterReceiver - Modtager perimeter wire signal
 *
 * Bruger en coil (100-150 mH) og LM386 forstærker til at
 * detektere og afkode perimeterkabel signalet.
 *
 * Funktioner:
 * - Detekterer om robotten er inden for eller uden for perimeteren
 * - Måler signalstyrke for afstandsestimering
 * - Detekterer signalretning (til venstre/højre for kablet)
 */

// Perimeter tilstande
enum PerimeterState {
    PERIMETER_INSIDE,       // Inden for perimeteren
    PERIMETER_OUTSIDE,      // Uden for perimeteren
    PERIMETER_ON_WIRE,      // På selve kablet
    PERIMETER_NO_SIGNAL,    // Intet signal (sender slukket eller for langt væk)
    PERIMETER_ERROR         // Fejl
};

// Signal retning
enum PerimeterDirection {
    PERIMETER_LEFT,         // Kabel er til venstre
    PERIMETER_RIGHT,        // Kabel er til højre
    PERIMETER_CENTER,       // På kablet
    PERIMETER_UNKNOWN       // Ukendt retning
};

class PerimeterReceiver {
public:
    PerimeterReceiver();

    /**
     * Initialiserer modtageren
     * @return true hvis initialisering lykkedes
     */
    bool begin();

    /**
     * Opdaterer signal læsninger (skal kaldes ofte i loop)
     */
    void update();

    /**
     * Henter nuværende perimeter tilstand
     */
    PerimeterState getState() const { return _state; }

    /**
     * Henter tilstand som tekst
     */
    String getStateString() const;

    /**
     * Tjekker om robotten er inden for perimeteren
     */
    bool isInside() const { return _state == PERIMETER_INSIDE; }

    /**
     * Tjekker om robotten er uden for perimeteren
     */
    bool isOutside() const { return _state == PERIMETER_OUTSIDE; }

    /**
     * Tjekker om der er signal
     */
    bool hasSignal() const { return _state != PERIMETER_NO_SIGNAL && _state != PERIMETER_ERROR; }

    /**
     * Henter signal styrke (0-100%)
     */
    int getSignalStrength() const { return _signalStrength; }

    /**
     * Henter rå signal magnitude
     */
    int getSignalMagnitude() const { return _signalMagnitude; }

    /**
     * Henter signal retning relativt til robotten
     */
    PerimeterDirection getDirection() const { return _direction; }

    /**
     * Henter retning som tekst
     */
    String getDirectionString() const;

    /**
     * Henter estimeret afstand til kabel (cm)
     * Bemærk: Dette er et groft estimat baseret på signalstyrke
     */
    int getDistanceToCable() const { return _distanceToCable; }

    /**
     * Kalibrerer modtageren (skal udføres på kablet)
     */
    void calibrate();

    /**
     * Nulstiller modtageren
     */
    void reset();

    /**
     * Henter debug information
     */
    String getDebugInfo() const;

private:
    bool _initialized;
    PerimeterState _state;
    PerimeterDirection _direction;

    // Signal data
    int _signalStrength;        // 0-100%
    int _signalMagnitude;       // Rå magnitude
    int _smoothedMagnitude;     // Glattet magnitude
    int _distanceToCable;       // Estimeret afstand i cm

    // Kalibrering
    int _calibrationValue;      // Kalibreret max signal
    bool _calibrated;

    // Signal behandling
    static const int SAMPLE_COUNT = 128;
    int _samples[SAMPLE_COUNT];
    int _sampleIndex;
    unsigned long _lastSampleTime;

    // Timing
    unsigned long _lastUpdate;
    unsigned long _lastSignalTime;

    // Konfiguration
    static const int SIGNAL_TIMEOUT_MS = 1000;      // Timeout for signal tab
    static const int MIN_SIGNAL_THRESHOLD = 50;     // Minimum signal for detektion
    static const int WIRE_THRESHOLD = 200;          // Tærskel for "på kablet"
    static const int SAMPLE_INTERVAL_US = 52;       // Sample interval (19.2 kHz)

    // Private metoder
    void sampleSignal();
    void processSignal();
    void detectState();
    void detectDirection();
    int calculateMagnitude();
    void updateSmoothedMagnitude();
};

#endif // PERIMETER_RECEIVER_H
