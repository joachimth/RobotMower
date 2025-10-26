#ifndef WEBAPI_H
#define WEBAPI_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include "../config/Config.h"
#include "../system/Logger.h"
#include "WebServer.h"

// Forward declarations - disse sættes i RobotMower.ino
class StateManager;
class Battery;
class Sensors;
class IMU;
class Motors;

/**
 * WebAPI klasse - Håndterer REST API endpoints
 *
 * Denne klasse eksponerer robot funktionalitet via HTTP API.
 */
class WebAPI {
public:
    /**
     * Constructor
     */
    WebAPI();

    /**
     * Initialiserer Web API
     * @param webServer Pointer til WebServer objekt
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin(WebServer* webServer);

    /**
     * Sætter hardware references (kaldes fra main)
     * @param state StateManager pointer
     * @param batt Battery pointer
     * @param sens Sensors pointer
     * @param imuSensor IMU pointer
     * @param mot Motors pointer
     */
    void setHardwareReferences(StateManager* state, Battery* batt,
                              Sensors* sens, IMU* imuSensor, Motors* mot);

    /**
     * Opsætter alle API routes
     */
    void setupRoutes();

private:
    // HTTP request handlers
    void handleGetStatus(AsyncWebServerRequest *request);
    void handleStart(AsyncWebServerRequest *request);
    void handleStop(AsyncWebServerRequest *request);
    void handlePause(AsyncWebServerRequest *request);
    void handleCalibrate(AsyncWebServerRequest *request);
    void handleGetLogs(AsyncWebServerRequest *request);
    void handleGetSettings(AsyncWebServerRequest *request);
    void handleUpdateSettings(AsyncWebServerRequest *request);

    // JSON helper functions
    String createStatusJSON();
    String createLogsJSON(int count);
    String createSettingsJSON();

    // Hardware pointers
    WebServer* webServerPtr;
    StateManager* stateManagerPtr;
    Battery* batteryPtr;
    Sensors* sensorsPtr;
    IMU* imuPtr;
    Motors* motorsPtr;

    // State
    bool initialized;
};

#endif // WEBAPI_H
