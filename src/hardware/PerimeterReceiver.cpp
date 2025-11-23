#include "PerimeterReceiver.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

PerimeterReceiver::PerimeterReceiver()
    : _initialized(false)
    , _state(PERIMETER_NO_SIGNAL)
    , _direction(PERIMETER_UNKNOWN)
    , _signalStrength(0)
    , _signalMagnitude(0)
    , _smoothedMagnitude(0)
    , _distanceToCable(0)
    , _calibrationValue(1000)
    , _calibrated(false)
    , _sampleIndex(0)
    , _lastSampleTime(0)
    , _lastUpdate(0)
    , _lastSignalTime(0)
{
    memset(_samples, 0, sizeof(_samples));
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

bool PerimeterReceiver::begin() {
    if (_initialized) return true;

    Serial.println("[Perimeter] Initializing receiver...");

    // Konfigurer ADC pin
    pinMode(PERIMETER_SIGNAL_PIN, INPUT);

    // Konfigurer ADC for hurtig sampling
    analogReadResolution(12);  // 12-bit ADC
    analogSetAttenuation(ADC_11db);  // 0-3.3V range

    _initialized = true;
    _state = PERIMETER_NO_SIGNAL;

    Serial.println("[Perimeter] Receiver initialized");
    Serial.printf("[Perimeter] Signal pin: GPIO%d\n", PERIMETER_SIGNAL_PIN);

    return true;
}

void PerimeterReceiver::update() {
    if (!_initialized) return;

    unsigned long now = micros();

    // Sample signal ved fast interval
    if (now - _lastSampleTime >= SAMPLE_INTERVAL_US) {
        sampleSignal();
        _lastSampleTime = now;
    }

    // Process signal med lavere frekvens
    unsigned long nowMs = millis();
    if (nowMs - _lastUpdate >= 20) {  // 50 Hz
        processSignal();
        detectState();
        detectDirection();
        _lastUpdate = nowMs;
    }
}

String PerimeterReceiver::getStateString() const {
    switch (_state) {
        case PERIMETER_INSIDE:      return "INSIDE";
        case PERIMETER_OUTSIDE:     return "OUTSIDE";
        case PERIMETER_ON_WIRE:     return "ON_WIRE";
        case PERIMETER_NO_SIGNAL:   return "NO_SIGNAL";
        case PERIMETER_ERROR:       return "ERROR";
        default:                    return "UNKNOWN";
    }
}

String PerimeterReceiver::getDirectionString() const {
    switch (_direction) {
        case PERIMETER_LEFT:    return "LEFT";
        case PERIMETER_RIGHT:   return "RIGHT";
        case PERIMETER_CENTER:  return "CENTER";
        case PERIMETER_UNKNOWN: return "UNKNOWN";
        default:                return "?";
    }
}

void PerimeterReceiver::calibrate() {
    Serial.println("[Perimeter] Starting calibration...");
    Serial.println("[Perimeter] Place coil directly on the wire!");

    // Sample for 2 sekunder
    int maxMagnitude = 0;
    unsigned long startTime = millis();

    while (millis() - startTime < 2000) {
        sampleSignal();
        delayMicroseconds(SAMPLE_INTERVAL_US);

        if (_sampleIndex == 0) {
            int mag = calculateMagnitude();
            if (mag > maxMagnitude) {
                maxMagnitude = mag;
            }
        }
    }

    if (maxMagnitude > MIN_SIGNAL_THRESHOLD) {
        _calibrationValue = maxMagnitude;
        _calibrated = true;
        Serial.printf("[Perimeter] Calibration complete! Max magnitude: %d\n", _calibrationValue);
    } else {
        Serial.println("[Perimeter] Calibration failed - no signal detected!");
        Serial.println("[Perimeter] Check that sender is running and coil is on wire");
    }
}

void PerimeterReceiver::reset() {
    _state = PERIMETER_NO_SIGNAL;
    _direction = PERIMETER_UNKNOWN;
    _signalStrength = 0;
    _signalMagnitude = 0;
    _smoothedMagnitude = 0;
    _sampleIndex = 0;
    memset(_samples, 0, sizeof(_samples));
}

String PerimeterReceiver::getDebugInfo() const {
    char buf[200];
    snprintf(buf, sizeof(buf),
             "State: %s | Dir: %s | Strength: %d%% | Mag: %d | Smooth: %d | Dist: %dcm | Cal: %d",
             getStateString().c_str(),
             getDirectionString().c_str(),
             _signalStrength,
             _signalMagnitude,
             _smoothedMagnitude,
             _distanceToCable,
             _calibrationValue);
    return String(buf);
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

void PerimeterReceiver::sampleSignal() {
    // Læs ADC værdi
    int value = analogRead(PERIMETER_SIGNAL_PIN);

    // Centrer omkring midten (2048 for 12-bit ADC med signal 0-3.3V)
    // LM386 output er centreret omkring 2.5V når C3 er bypassed
    _samples[_sampleIndex] = value - 2048;

    _sampleIndex++;
    if (_sampleIndex >= SAMPLE_COUNT) {
        _sampleIndex = 0;
    }
}

void PerimeterReceiver::processSignal() {
    // Beregn magnitude
    _signalMagnitude = calculateMagnitude();

    // Opdater glattet magnitude
    updateSmoothedMagnitude();

    // Beregn signal styrke (0-100%)
    if (_calibrated && _calibrationValue > 0) {
        _signalStrength = constrain((_smoothedMagnitude * 100) / _calibrationValue, 0, 100);
    } else {
        // Uden kalibrering, brug fast skala
        _signalStrength = constrain(_smoothedMagnitude / 10, 0, 100);
    }

    // Estimer afstand til kabel
    // Jo stærkere signal, jo tættere på kablet
    if (_signalStrength > 0) {
        // Groft estimat: max signal ved 0cm, ingen signal ved ~100cm
        _distanceToCable = map(100 - _signalStrength, 0, 100, 0, 100);
    } else {
        _distanceToCable = 999;  // Ukendt
    }

    // Opdater sidste signal tid
    if (_signalMagnitude > MIN_SIGNAL_THRESHOLD) {
        _lastSignalTime = millis();
    }
}

int PerimeterReceiver::calculateMagnitude() {
    // Beregn RMS (Root Mean Square) af samples
    long sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        sum += (long)_samples[i] * _samples[i];
    }

    // Returnerer RMS værdi
    return sqrt(sum / SAMPLE_COUNT);
}

void PerimeterReceiver::updateSmoothedMagnitude() {
    // Eksponentiel glatning
    const float alpha = 0.2;
    _smoothedMagnitude = (int)(alpha * _signalMagnitude + (1 - alpha) * _smoothedMagnitude);
}

void PerimeterReceiver::detectState() {
    unsigned long now = millis();

    // Tjek for signal timeout
    if (now - _lastSignalTime > SIGNAL_TIMEOUT_MS) {
        _state = PERIMETER_NO_SIGNAL;
        return;
    }

    // Tjek signal niveau
    if (_smoothedMagnitude < MIN_SIGNAL_THRESHOLD) {
        _state = PERIMETER_NO_SIGNAL;
        return;
    }

    // Tjek om vi er på kablet (meget stærkt signal)
    if (_smoothedMagnitude > WIRE_THRESHOLD) {
        _state = PERIMETER_ON_WIRE;
        return;
    }

    // Bestem om vi er inden for eller uden for
    // Dette baseres på signal polaritet
    // Positiv = inden for, Negativ = uden for
    // (Afhænger af coil orientering og signal fase)

    // Find gennemsnitlig sample værdi for at bestemme polaritet
    long sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        sum += _samples[i];
    }
    int avgSample = sum / SAMPLE_COUNT;

    // Bemærk: Denne logik kan skal justeres baseret på hardware setup
    // Positiv gennemsnit indikerer typisk "inden for"
    if (avgSample > 10) {
        _state = PERIMETER_INSIDE;
    } else if (avgSample < -10) {
        _state = PERIMETER_OUTSIDE;
    } else {
        // Tæt på nul = på kablet
        _state = PERIMETER_ON_WIRE;
    }
}

void PerimeterReceiver::detectDirection() {
    if (_state == PERIMETER_NO_SIGNAL) {
        _direction = PERIMETER_UNKNOWN;
        return;
    }

    if (_state == PERIMETER_ON_WIRE) {
        _direction = PERIMETER_CENTER;
        return;
    }

    // Retningsdetektion baseret på signal fase
    // Dette kræver mere avanceret signal behandling
    // For nu bruger vi en simpel metode baseret på state

    // Find peak-to-peak for at analysere signal asymmetri
    int maxSample = -32768;
    int minSample = 32767;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        if (_samples[i] > maxSample) maxSample = _samples[i];
        if (_samples[i] < minSample) minSample = _samples[i];
    }

    int center = (maxSample + minSample) / 2;

    // Asymmetri i signalet indikerer retning
    if (center > 20) {
        _direction = PERIMETER_LEFT;
    } else if (center < -20) {
        _direction = PERIMETER_RIGHT;
    } else {
        _direction = PERIMETER_CENTER;
    }
}
