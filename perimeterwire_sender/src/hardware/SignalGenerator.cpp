#include "SignalGenerator.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

SignalGenerator::SignalGenerator()
    : _running(false)
    , _initialized(false)
    , _state(SENDER_OFF)
    , _lastBitTime(0)
    , _currentBitIndex(0)
    , _currentPolarity(true)
    , _currentMA(0)
    , _currentSampleIndex(0)
    , _lastCurrentCheck(0)
    , _startTime(0)
    , _cycleCount(0)
{
    memset(_currentSamples, 0, sizeof(_currentSamples));
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

bool SignalGenerator::begin() {
    if (_initialized) return true;

    Serial.println("[SignalGen] Initializing signal generator...");

    // Konfigurer PWM pins
    pinMode(DRIVER_PWM_A, OUTPUT);
    pinMode(DRIVER_PWM_B, OUTPUT);
    pinMode(DRIVER_ENABLE, OUTPUT);

    // Konfigurer strømmåling
    pinMode(CURRENT_SENSE_PIN, INPUT);

    // Konfigurer status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Setup LEDC PWM kanaler
    ledcSetup(PWM_CHANNEL_A, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_B, PWM_FREQUENCY, PWM_RESOLUTION);

    // Attach PWM til pins
    ledcAttachPin(DRIVER_PWM_A, PWM_CHANNEL_A);
    ledcAttachPin(DRIVER_PWM_B, PWM_CHANNEL_B);

    // Start med output off
    outputOff();

    // Disable driver
    digitalWrite(DRIVER_ENABLE, LOW);

    _initialized = true;
    _state = SENDER_OFF;

    Serial.println("[SignalGen] Initialization complete");
    return true;
}

void SignalGenerator::start() {
    if (!_initialized) {
        Serial.println("[SignalGen] ERROR: Not initialized!");
        return;
    }

    if (_running) {
        Serial.println("[SignalGen] Already running");
        return;
    }

    Serial.println("[SignalGen] Starting signal output...");

    _state = SENDER_STARTING;
    _currentBitIndex = 0;
    _currentPolarity = true;
    _cycleCount = 0;

    // Enable driver
    digitalWrite(DRIVER_ENABLE, HIGH);
    delay(10);  // Vent på driver stabilisering

    // Start signal
    _running = true;
    _startTime = millis();
    _lastBitTime = micros();
    _state = SENDER_RUNNING;

    // Tænd status LED
    digitalWrite(STATUS_LED_PIN, HIGH);

    Serial.println("[SignalGen] Signal output started");
}

void SignalGenerator::stop() {
    if (!_running) return;

    Serial.println("[SignalGen] Stopping signal output...");

    _running = false;
    _state = SENDER_OFF;

    // Stop output
    outputOff();

    // Disable driver
    digitalWrite(DRIVER_ENABLE, LOW);

    // Sluk status LED
    digitalWrite(STATUS_LED_PIN, LOW);

    Serial.println("[SignalGen] Signal output stopped");
}

void SignalGenerator::update() {
    if (!_initialized) return;

    // Opdater strømmåling
    unsigned long now = millis();
    if (now - _lastCurrentCheck >= CURRENT_CHECK_INTERVAL) {
        updateCurrentReading();
        _lastCurrentCheck = now;

        // Tjek for overstrøm
        if (_running) {
            checkOvercurrent();
        }
    }

    // Generer signal hvis kørende
    if (_running && _state == SENDER_RUNNING) {
        generateNextBit();
    }
}

String SignalGenerator::getStateString() const {
    switch (_state) {
        case SENDER_OFF:         return "OFF";
        case SENDER_STARTING:    return "STARTING";
        case SENDER_RUNNING:     return "RUNNING";
        case SENDER_ERROR:       return "ERROR";
        case SENDER_OVERCURRENT: return "OVERCURRENT";
        default:                 return "UNKNOWN";
    }
}

void SignalGenerator::reset() {
    Serial.println("[SignalGen] Resetting...");

    stop();
    _state = SENDER_OFF;
    _currentMA = 0;
    _cycleCount = 0;

    // Nulstil strømsamples
    memset(_currentSamples, 0, sizeof(_currentSamples));
    _currentSampleIndex = 0;
}

unsigned long SignalGenerator::getRuntime() const {
    if (!_running || _startTime == 0) return 0;
    return millis() - _startTime;
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

void SignalGenerator::generateNextBit() {
    unsigned long nowMicros = micros();

    // Beregn tid siden sidste bit
    unsigned long elapsed = nowMicros - _lastBitTime;

    // Hent nuværende bit fra koden
    uint8_t currentBit = SIGNAL_CODE[_currentBitIndex];

    // Beregn bit varighed baseret på frekvens
    unsigned long bitDuration;
    if (currentBit == 1) {
        bitDuration = 1000000 / SIGNAL_HIGH_FREQ;  // ~208 us
    } else {
        bitDuration = 1000000 / SIGNAL_LOW_FREQ;   // ~416 us
    }

    // Skift polaritet når bit varighed er nået
    if (elapsed >= bitDuration) {
        _lastBitTime = nowMicros;

        if (_currentPolarity) {
            // Positiv halvperiode: A høj, B lav
            ledcWrite(PWM_CHANNEL_A, SIGNAL_DUTY_CYCLE);
            ledcWrite(PWM_CHANNEL_B, 0);
        } else {
            // Negativ halvperiode: A lav, B høj
            ledcWrite(PWM_CHANNEL_A, 0);
            ledcWrite(PWM_CHANNEL_B, SIGNAL_DUTY_CYCLE);

            // Gå til næste bit efter fuld periode
            _currentBitIndex++;
            if (_currentBitIndex >= SIGNAL_CODE_LENGTH) {
                _currentBitIndex = 0;
                _cycleCount++;
            }
        }

        _currentPolarity = !_currentPolarity;
    }
}

void SignalGenerator::outputHighFrequency() {
    // Output ved høj frekvens (4808 Hz)
    ledcWrite(PWM_CHANNEL_A, SIGNAL_DUTY_CYCLE);
    ledcWrite(PWM_CHANNEL_B, 0);
}

void SignalGenerator::outputLowFrequency() {
    // Output ved lav frekvens (2404 Hz)
    ledcWrite(PWM_CHANNEL_A, 0);
    ledcWrite(PWM_CHANNEL_B, SIGNAL_DUTY_CYCLE);
}

void SignalGenerator::outputOff() {
    ledcWrite(PWM_CHANNEL_A, 0);
    ledcWrite(PWM_CHANNEL_B, 0);
}

void SignalGenerator::updateCurrentReading() {
    // Læs ADC værdi
    int adcValue = analogRead(CURRENT_SENSE_PIN);

    // Konverter til spænding (mV)
    float voltage_mV = (adcValue / 4095.0) * 3300.0;

    // Konverter til strøm (mA) - baseret på ACS712-5A sensor
    // Output er 2.5V ved 0A, 185mV per A
    float current_mA = (voltage_mV - CURRENT_SENSE_OFFSET) / (CURRENT_SENSE_MV_PER_A / 1000.0);

    // Gem i samples array
    _currentSamples[_currentSampleIndex] = abs(current_mA);
    _currentSampleIndex = (_currentSampleIndex + 1) % CURRENT_SAMPLES;

    // Beregn gennemsnit
    float sum = 0;
    for (int i = 0; i < CURRENT_SAMPLES; i++) {
        sum += _currentSamples[i];
    }
    _currentMA = sum / CURRENT_SAMPLES;

    #if DEBUG_CURRENT
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug >= 5000) {
        Serial.printf("[SignalGen] Current: %.1f mA (ADC: %d, V: %.1f mV)\n",
                     _currentMA, adcValue, voltage_mV);
        lastDebug = millis();
    }
    #endif
}

void SignalGenerator::checkOvercurrent() {
    if (_currentMA > CURRENT_MAX_MA) {
        Serial.printf("[SignalGen] OVERCURRENT! %.1f mA > %d mA\n",
                     _currentMA, CURRENT_MAX_MA);

        #if OVERCURRENT_SHUTDOWN
        stop();
        _state = SENDER_OVERCURRENT;
        Serial.println("[SignalGen] Emergency shutdown due to overcurrent!");
        #endif
    } else if (_currentMA > CURRENT_WARNING_MA) {
        Serial.printf("[SignalGen] Current warning: %.1f mA\n", _currentMA);
    }
}
