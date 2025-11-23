/**
 * ============================================================================
 * PERIMETER WIRE SENDER - ESP32
 * ============================================================================
 * Controller til perimeterkabel signalgenerator
 *
 * Hardware:
 * - ESP32 (DevKit eller lignende)
 * - Motor driver (MC33926 eller L298N)
 * - DC/DC converter (justerbar output)
 * - Perimeterkabel (100m+)
 *
 * Signal:
 * - 3.2 kHz carrier frekvens
 * - Pseudo-random kodning (Ardumower kompatibel)
 * - Justerbar strøm via potentiometer (maks 1A)
 *
 * API Endpoints:
 * - GET  /api/status - Hent status
 * - POST /api/start  - Start signal
 * - POST /api/stop   - Stop signal
 * - POST /api/reset  - Reset efter fejl
 * ============================================================================
 */

#include <Arduino.h>
#include "config/Config.h"
#include "hardware/SignalGenerator.h"
#include "web/WebServer.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

SignalGenerator signalGen;
PerimeterWebServer webServer;

// Timing
unsigned long lastStatusUpdate = 0;
unsigned long lastHealthCheck = 0;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void printStartupBanner();
void updateStatus();
void checkSignalHealth();

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize Serial
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    printStartupBanner();

    // Initialize Signal Generator
    Serial.println("[Main] Initializing signal generator...");
    if (!signalGen.begin()) {
        Serial.println("[Main] ERROR: Failed to initialize signal generator!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            delay(100);
        }
    }
    Serial.println("[Main] Signal generator ready");

    // Initialize Web Server
    Serial.println("[Main] Initializing web server...");
    if (!webServer.begin(&signalGen)) {
        Serial.println("[Main] ERROR: Failed to initialize web server!");
    }
    Serial.println("[Main] Web server ready");

    // Print access information
    Serial.println();
    Serial.println("============================================");
    Serial.printf("Web Interface: http://%s\n", webServer.getIPAddress().c_str());
    Serial.printf("mDNS: http://%s.local\n", MDNS_HOSTNAME);
    Serial.println("============================================");
    Serial.println();

    Serial.println("[Main] System ready!");
    Serial.println("[Main] Use web interface or API to control signal");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Update signal generator
    signalGen.update();

    // Update web server
    webServer.update();

    // Periodic status update
    unsigned long now = millis();
    if (now - lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
        updateStatus();
        lastStatusUpdate = now;
    }

    // Signal health check
    if (now - lastHealthCheck >= SIGNAL_HEALTH_INTERVAL) {
        checkSignalHealth();
        lastHealthCheck = now;
    }

    // Small delay to prevent watchdog
    delay(1);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void printStartupBanner() {
    Serial.println();
    Serial.println("============================================");
    Serial.println("   PERIMETER WIRE SENDER - ESP32");
    Serial.println("============================================");
    Serial.printf("   Signal: %d Hz carrier\n", SIGNAL_CARRIER_FREQ);
    Serial.printf("   Max current: %d mA\n", CURRENT_MAX_MA);
    Serial.println("============================================");
    Serial.println();
}

void updateStatus() {
    #if DEBUG_MODE
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 5000) {
        Serial.printf("[Status] State: %s | Current: %.0f mA | Runtime: %lu s | Cycles: %lu\n",
                     signalGen.getStateString().c_str(),
                     signalGen.getCurrentMA(),
                     signalGen.getRuntime() / 1000,
                     signalGen.getCycleCount());
        lastPrint = millis();
    }
    #endif
}

void checkSignalHealth() {
    if (!signalGen.isRunning()) return;

    // Tjek for lav strøm (kan indikere kabelbrud)
    float current = signalGen.getCurrentMA();
    if (current < 10 && signalGen.getRuntime() > 5000) {
        Serial.println("[Health] WARNING: Very low current - check cable connection!");
    }

    // Tjek for ustabil strøm
    static float lastCurrent = 0;
    float diff = abs(current - lastCurrent);
    if (diff > 200 && lastCurrent > 0) {
        Serial.printf("[Health] WARNING: Current fluctuation: %.0f mA\n", diff);
    }
    lastCurrent = current;
}
