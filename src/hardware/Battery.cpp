#include "Battery.h"

Battery::Battery() {
    voltage = 0.0;
    percentage = 0;
    lastUpdate = 0;
    initialized = false;
    lowWarningShown = false;
    criticalWarningShown = false;
}

bool Battery::begin() {
    // Konfigurer ADC pin
    pinMode(BATTERY_PIN, INPUT);

    // ESP32 ADC konfiguration
    analogReadResolution(12); // 12-bit ADC (0-4095)
    analogSetAttenuation(ADC_11db); // Fuld 3.3V range

    // Tag første måling
    delay(100);
    voltage = readVoltage();
    percentage = calculatePercentage(voltage);

    initialized = true;

    Serial.println("[Battery] Initialized successfully");
    printStatus();

    return true;
}

void Battery::update() {
    if (!initialized) {
        return;
    }

    // Tjek om det er tid til opdatering
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate < BATTERY_CHECK_INTERVAL) {
        return; // Ikke tid endnu
    }

    // Læs ny spænding
    voltage = readVoltage();
    percentage = calculatePercentage(voltage);

    lastUpdate = currentTime;

    // Advarsler
    if (isCritical() && !criticalWarningShown) {
        Serial.println("[Battery] !!! CRITICAL BATTERY LEVEL !!!");
        criticalWarningShown = true;
    } else if (isLow() && !lowWarningShown) {
        Serial.println("[Battery] WARNING: Low battery level");
        lowWarningShown = true;
    }

    // Debug output
    #if DEBUG_MODE
    static int updateCount = 0;
    updateCount++;
    if (updateCount >= 12) { // Ca. hver minut (5s interval * 12)
        printStatus();
        updateCount = 0;
    }
    #endif
}

float Battery::getVoltage() {
    return voltage;
}

int Battery::getPercentage() {
    return percentage;
}

bool Battery::isLow() {
    return (voltage < BATTERY_LOW_VOLTAGE && voltage > 0);
}

bool Battery::isCritical() {
    return (voltage < BATTERY_CRITICAL_VOLTAGE && voltage > 0);
}

void Battery::printStatus() {
    Serial.printf("[Battery] Voltage: %.2fV | Percentage: %d%% | Status: ",
                  voltage, percentage);

    if (isCritical()) {
        Serial.println("CRITICAL");
    } else if (isLow()) {
        Serial.println("LOW");
    } else {
        Serial.println("OK");
    }
}

int Battery::getEstimatedTimeRemaining(float currentDraw) {
    // Simpel estimering baseret på batteri kapacitet
    // Antager 3S LiPo med 5000mAh kapacitet (juster efter dit batteri)
    const float BATTERY_CAPACITY = 5.0; // Ah

    if (currentDraw <= 0) {
        return 9999; // Uendelig tid hvis ingen forbrug
    }

    // Resterende kapacitet baseret på spænding
    float remainingCapacity = (percentage / 100.0) * BATTERY_CAPACITY;

    // Tid = Kapacitet / Forbrug
    float hours = remainingCapacity / currentDraw;
    int minutes = (int)(hours * 60);

    return minutes;
}

float Battery::readVoltage() {
    // Tag flere målinger og lav gennemsnit for stabilitet
    const int NUM_SAMPLES = 10;
    long sum = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += analogRead(BATTERY_PIN);
        delay(10);
    }

    int avgADC = sum / NUM_SAMPLES;

    // Konverter ADC værdi til spænding
    float adcVoltage = (avgADC / BATTERY_ADC_MAX) * BATTERY_ADC_VREF;

    // Beregn faktisk batteri spænding gennem voltage divider
    // Vout = Vin * R2 / (R1 + R2)
    // Vin = Vout * (R1 + R2) / R2
    float batteryVoltage = adcVoltage * (BATTERY_R1 + BATTERY_R2) / BATTERY_R2;

    return batteryVoltage;
}

int Battery::calculatePercentage(float voltage) {
    // LiPo spændingskurve er ikke-lineær, men vi bruger en simpel lineær approximation
    if (voltage >= BATTERY_MAX_VOLTAGE) {
        return 100;
    }
    if (voltage <= BATTERY_MIN_VOLTAGE) {
        return 0;
    }

    // Lineær mapping mellem min og max
    int percent = map((int)(voltage * 100),
                      (int)(BATTERY_MIN_VOLTAGE * 100),
                      (int)(BATTERY_MAX_VOLTAGE * 100),
                      0, 100);

    return constrain(percent, 0, 100);
}
