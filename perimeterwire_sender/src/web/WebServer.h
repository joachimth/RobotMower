#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../config/Config.h"
#include "../hardware/SignalGenerator.h"

/**
 * PerimeterWebServer - Web interface til perimeter wire sender
 *
 * Håndterer HTTP requests og serverer web UI til kontrol
 * og overvågning af perimeter wire senderen.
 */
class PerimeterWebServer {
public:
    PerimeterWebServer();

    /**
     * Initialiserer web serveren
     * @param signalGen Reference til signal generator
     * @return true hvis initialisering lykkedes
     */
    bool begin(SignalGenerator* signalGen);

    /**
     * Opdaterer web serveren (ikke nødvendigt for AsyncWebServer)
     */
    void update();

    /**
     * Tjekker om serveren kører
     */
    bool isRunning() const { return _running; }

    /**
     * Henter IP adresse
     */
    String getIPAddress() const;

    /**
     * Tjekker om vi er i AP mode
     */
    bool isAPMode() const { return _apMode; }

private:
    AsyncWebServer _server;
    SignalGenerator* _signalGen;
    bool _running;
    bool _apMode;

    // Route handlers
    void setupRoutes();
    void handleRoot(AsyncWebServerRequest* request);
    void handleStatus(AsyncWebServerRequest* request);
    void handleStart(AsyncWebServerRequest* request);
    void handleStop(AsyncWebServerRequest* request);
    void handleReset(AsyncWebServerRequest* request);
    void handleNotFound(AsyncWebServerRequest* request);

    // WiFi setup
    bool connectWiFi();
    void startAP();

    // Hjælpefunktioner
    String createStatusJSON();
    void addCORSHeaders(AsyncWebServerResponse* response);
};

#endif // WEBSERVER_H
