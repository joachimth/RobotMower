#include "WebAPI.h"
#include "../system/StateManager.h"
#include "../hardware/Battery.h"
#include "../hardware/Sensors.h"
#include "../hardware/IMU.h"
#include "../hardware/Motors.h"

WebAPI::WebAPI() {
    webServerPtr = nullptr;
    stateManagerPtr = nullptr;
    batteryPtr = nullptr;
    sensorsPtr = nullptr;
    imuPtr = nullptr;
    motorsPtr = nullptr;
    initialized = false;
}

bool WebAPI::begin(WebServer* webServer) {
    if (webServer == nullptr) {
        Logger::error("WebAPI: Invalid WebServer pointer");
        return false;
    }

    webServerPtr = webServer;
    initialized = true;

    Logger::info("WebAPI initialized");

    return true;
}

void WebAPI::setHardwareReferences(StateManager* state, Battery* batt,
                                   Sensors* sens, IMU* imuSensor, Motors* mot) {
    stateManagerPtr = state;
    batteryPtr = batt;
    sensorsPtr = sens;
    imuPtr = imuSensor;
    motorsPtr = mot;

    Logger::info("WebAPI hardware references set");
}

void WebAPI::setupRoutes() {
    if (!initialized || webServerPtr == nullptr) {
        return;
    }

    AsyncWebServer* server = webServerPtr->getServer();
    if (server == nullptr) {
        return;
    }

    // GET /api/status
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetStatus(request);
    });

    // POST /api/start
    server->on("/api/start", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleStart(request);
    });

    // POST /api/stop
    server->on("/api/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleStop(request);
    });

    // POST /api/pause
    server->on("/api/pause", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handlePause(request);
    });

    // POST /api/calibrate
    server->on("/api/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCalibrate(request);
    });

    // GET /api/logs
    server->on("/api/logs", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetLogs(request);
    });

    // GET /api/settings
    server->on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetSettings(request);
    });

    Logger::info("API routes configured");
}

void WebAPI::handleGetStatus(AsyncWebServerRequest *request) {
    String json = createStatusJSON();
    request->send(200, "application/json", json);
}

void WebAPI::handleStart(AsyncWebServerRequest *request) {
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->startMowing();
        request->send(200, "application/json", "{\"status\":\"started\"}");
        Logger::info("API: Start mowing requested");
    } else {
        request->send(500, "application/json", "{\"error\":\"System not ready\"}");
    }
}

void WebAPI::handleStop(AsyncWebServerRequest *request) {
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->stopMowing();
        request->send(200, "application/json", "{\"status\":\"stopped\"}");
        Logger::info("API: Stop mowing requested");
    } else {
        request->send(500, "application/json", "{\"error\":\"System not ready\"}");
    }
}

void WebAPI::handlePause(AsyncWebServerRequest *request) {
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->pauseMowing();
        request->send(200, "application/json", "{\"status\":\"paused\"}");
        Logger::info("API: Pause mowing requested");
    } else {
        request->send(500, "application/json", "{\"error\":\"System not ready\"}");
    }
}

void WebAPI::handleCalibrate(AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"calibrating\"}");
    Logger::info("API: Calibration requested");
    // Calibration vil blive håndteret i main loop
}

void WebAPI::handleGetLogs(AsyncWebServerRequest *request) {
    int count = 50; // Default

    if (request->hasParam("count")) {
        count = request->getParam("count")->value().toInt();
    }

    String json = createLogsJSON(count);
    request->send(200, "application/json", json);
}

void WebAPI::handleGetSettings(AsyncWebServerRequest *request) {
    String json = createSettingsJSON();
    request->send(200, "application/json", json);
}

void WebAPI::handleUpdateSettings(AsyncWebServerRequest *request) {
    // TODO: Implement settings update
    request->send(200, "application/json", "{\"status\":\"updated\"}");
}

String WebAPI::createStatusJSON() {
    // Opret JSON status objekt
    StaticJsonDocument<512> doc;

    // State
    if (stateManagerPtr != nullptr) {
        doc["state"] = stateManagerPtr->getStateName();
        doc["timeInState"] = stateManagerPtr->getTimeInState();
    } else {
        doc["state"] = "UNKNOWN";
    }

    // Battery
    if (batteryPtr != nullptr) {
        JsonObject battery = doc.createNestedObject("battery");
        battery["voltage"] = batteryPtr->getVoltage();
        battery["percentage"] = batteryPtr->getPercentage();
        battery["isLow"] = batteryPtr->isLow();
        battery["isCritical"] = batteryPtr->isCritical();
    }

    // IMU
    if (imuPtr != nullptr) {
        doc["heading"] = imuPtr->getHeading();
        doc["pitch"] = imuPtr->getPitch();
        doc["roll"] = imuPtr->getRoll();
    }

    // Sensors
    if (sensorsPtr != nullptr) {
        JsonObject sensors = doc.createNestedObject("sensors");
        sensors["left"] = sensorsPtr->getLeftDistance();
        sensors["middle"] = sensorsPtr->getMiddleDistance();
        sensors["right"] = sensorsPtr->getRightDistance();
    }

    // Motors
    if (motorsPtr != nullptr) {
        JsonObject motors = doc.createNestedObject("motors");
        motors["left"] = motorsPtr->getLeftSpeed();
        motors["right"] = motorsPtr->getRightSpeed();
        motors["isMoving"] = motorsPtr->isMoving();
    }

    // System info
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();

    String output;
    serializeJson(doc, output);
    return output;
}

String WebAPI::createLogsJSON(int count) {
    // Hent logs fra Logger
    return Logger::getRecentLogs(count);
}

String WebAPI::createSettingsJSON() {
    StaticJsonDocument<256> doc;

    doc["obstacleThreshold"] = OBSTACLE_THRESHOLD;
    doc["motorCruiseSpeed"] = MOTOR_CRUISE_SPEED;
    doc["rowWidth"] = MOWING_PATTERN_WIDTH;
    doc["turnAngle"] = TURN_ANGLE;

    String output;
    serializeJson(doc, output);
    return output;
}
