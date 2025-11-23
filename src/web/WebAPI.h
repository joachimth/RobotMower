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
class CuttingMechanism;
#if ENABLE_PERIMETER
class PerimeterReceiver;
class PerimeterClient;
#endif

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
     * @param webServer Pointer til MowerWebServer objekt
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin(MowerWebServer* webServer);

    /**
     * Sætter hardware references (kaldes fra main)
     * @param state StateManager pointer
     * @param batt Battery pointer
     * @param sens Sensors pointer
     * @param imuSensor IMU pointer
     * @param mot Motors pointer
     * @param cut CuttingMechanism pointer
     */
    void setHardwareReferences(StateManager* state, Battery* batt,
                              Sensors* sens, IMU* imuSensor, Motors* mot,
                              CuttingMechanism* cut);

    /**
     * Opsætter alle API routes
     */
    void setupRoutes();

    /**
     * Opret status JSON (public for WebSocket brug)
     * @return JSON string med robot status
     */
    String createStatusJSON();

    /**
     * Opret logs JSON
     * @param count Antal log entries
     * @return JSON string med logs
     */
    String createLogsJSON(int count);

    /**
     * Opret settings JSON
     * @return JSON string med indstillinger
     */
    String createSettingsJSON();

private:
    // HTTP request handlers
    void handleGetStatus(AsyncWebServerRequest *request);
    void handleStart(AsyncWebServerRequest *request);
    void handleStop(AsyncWebServerRequest *request);
    void handlePause(AsyncWebServerRequest *request);
    void handleCalibrate(AsyncWebServerRequest *request);
    void handleCalibrateMag(AsyncWebServerRequest *request);
    void handleGetLogs(AsyncWebServerRequest *request);
    void handleGetSettings(AsyncWebServerRequest *request);
    void handleUpdateSettings(AsyncWebServerRequest *request);

    // Manuel kontrol handlers
    void handleManualForward(AsyncWebServerRequest *request);
    void handleManualBackward(AsyncWebServerRequest *request);
    void handleManualLeft(AsyncWebServerRequest *request);
    void handleManualRight(AsyncWebServerRequest *request);
    void handleManualStop(AsyncWebServerRequest *request);
    void handleManualSetSpeed(AsyncWebServerRequest *request);
    void handleCuttingStart(AsyncWebServerRequest *request);
    void handleCuttingStop(AsyncWebServerRequest *request);
    void handleGetCurrent(AsyncWebServerRequest *request);

    #if ENABLE_PERIMETER
    // Perimeter handlers
    void handlePerimeterStatus(AsyncWebServerRequest *request);
    void handlePerimeterStart(AsyncWebServerRequest *request);
    void handlePerimeterStop(AsyncWebServerRequest *request);
    void handlePerimeterCalibrate(AsyncWebServerRequest *request);
    #endif

    // Hardware pointers
    MowerWebServer* webServerPtr;
    StateManager* stateManagerPtr;
    Battery* batteryPtr;
    Sensors* sensorsPtr;
    IMU* imuPtr;
    Motors* motorsPtr;
    CuttingMechanism* cuttingMechPtr;
    #if ENABLE_PERIMETER
    PerimeterReceiver* perimeterReceiverPtr;
    PerimeterClient* perimeterClientPtr;
    #endif

    // State
    bool initialized;

public:
    #if ENABLE_PERIMETER
    /**
     * Sætter perimeter references (kaldes fra main)
     */
    void setPerimeterReferences(PerimeterReceiver* receiver, PerimeterClient* client);
    #endif
};

#endif // WEBAPI_H
