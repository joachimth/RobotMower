#include "WebAPI.h"
#include "../system/StateManager.h"
#include "../hardware/Battery.h"
#include "../hardware/Sensors.h"
#include "../hardware/IMU.h"
#include "../hardware/Motors.h"
#include "../hardware/CuttingMechanism.h"
#if ENABLE_PERIMETER
#include "../hardware/PerimeterReceiver.h"
#include "../system/PerimeterClient.h"
#endif

// External kalibrerings funktioner fra main.cpp
extern void requestGyroCalibration();
extern void requestMagCalibration();

WebAPI::WebAPI() {
    webServerPtr = nullptr;
    stateManagerPtr = nullptr;
    batteryPtr = nullptr;
    sensorsPtr = nullptr;
    imuPtr = nullptr;
    motorsPtr = nullptr;
    cuttingMechPtr = nullptr;
    #if ENABLE_PERIMETER
    perimeterReceiverPtr = nullptr;
    perimeterClientPtr = nullptr;
    #endif
    initialized = false;
}

bool WebAPI::begin(MowerWebServer* webServer) {
    if (webServer == nullptr) {
        Logger::error("WebAPI: Invalid MowerWebServer pointer");
        return false;
    }

    webServerPtr = webServer;
    initialized = true;

    Logger::info("WebAPI initialized");

    return true;
}

void WebAPI::setHardwareReferences(StateManager* state, Battery* batt,
                                   Sensors* sens, IMU* imuSensor, Motors* mot,
                                   CuttingMechanism* cut) {
    stateManagerPtr = state;
    batteryPtr = batt;
    sensorsPtr = sens;
    imuPtr = imuSensor;
    motorsPtr = mot;
    cuttingMechPtr = cut;

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

    // POST /api/calibrate (gyro kalibrering)
    server->on("/api/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCalibrate(request);
    });

    // POST /api/calibrate/mag (magnetometer kalibrering)
    server->on("/api/calibrate/mag", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCalibrateMag(request);
    });

    // GET /api/logs
    server->on("/api/logs", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetLogs(request);
    });

    // GET /api/settings
    server->on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetSettings(request);
    });

    // POST /api/settings
    server->on("/api/settings", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleUpdateSettings(request);
    });

    // Manuel kontrol endpoints
    // POST /api/manual/forward
    server->on("/api/manual/forward", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleManualForward(request);
    });

    // POST /api/manual/backward
    server->on("/api/manual/backward", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleManualBackward(request);
    });

    // POST /api/manual/left
    server->on("/api/manual/left", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleManualLeft(request);
    });

    // POST /api/manual/right
    server->on("/api/manual/right", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleManualRight(request);
    });

    // POST /api/manual/stop
    server->on("/api/manual/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleManualStop(request);
    });

    // POST /api/manual/speed
    server->on("/api/manual/speed", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleManualSetSpeed(request);
    });

    // POST /api/cutting/start
    server->on("/api/cutting/start", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCuttingStart(request);
    });

    // POST /api/cutting/stop
    server->on("/api/cutting/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCuttingStop(request);
    });

    // GET /api/current
    server->on("/api/current", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleGetCurrent(request);
    });

    #if ENABLE_PERIMETER
    // Perimeter endpoints
    server->on("/api/perimeter/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handlePerimeterStatus(request);
    });

    server->on("/api/perimeter/start", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handlePerimeterStart(request);
    });

    server->on("/api/perimeter/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handlePerimeterStop(request);
    });

    server->on("/api/perimeter/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handlePerimeterCalibrate(request);
    });

    server->on("/api/return-to-base", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleReturnToBase(request);
    });
    #endif

    Logger::info("API routes configured (including manual control)");
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
    Logger::info("API: Gyro calibration requested");
    requestGyroCalibration();
    request->send(200, "application/json", "{\"status\":\"calibrating\",\"type\":\"gyro\"}");
}

void WebAPI::handleCalibrateMag(AsyncWebServerRequest *request) {
    if (imuPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"IMU not available\"}");
        return;
    }

    if (!imuPtr->hasMagnetometer()) {
        Logger::error("API: Magnetometer calibration requested but no magnetometer available");
        request->send(400, "application/json", "{\"error\":\"No magnetometer detected\"}");
        return;
    }

    Logger::info("API: Magnetometer calibration requested");
    requestMagCalibration();
    request->send(200, "application/json",
        "{\"status\":\"calibrating\",\"type\":\"magnetometer\",\"duration\":30,\"instructions\":\"Rotate robot slowly in all directions\"}");
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
    // Opret JSON status objekt - øget størrelse for strømdata
    StaticJsonDocument<768> doc;

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
        JsonObject imu = doc.createNestedObject("imu");
        imu["heading"] = imuPtr->getHeading();
        imu["pitch"] = imuPtr->getPitch();
        imu["roll"] = imuPtr->getRoll();
        imu["hasMagnetometer"] = imuPtr->hasMagnetometer();
        imu["magCalibrated"] = imuPtr->isMagCalibrated();
        imu["gyroCalibrated"] = imuPtr->isCalibrated();

        // Legacy felter for bagudkompatibilitet
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

        // Strømdata
        JsonObject current = motors.createNestedObject("current");
        current["left"] = motorsPtr->getLeftCurrent();
        current["right"] = motorsPtr->getRightCurrent();
        current["total"] = motorsPtr->getTotalCurrent();
        current["warning"] = motorsPtr->isCurrentWarning();
    }

    // Cutting mechanism
    if (cuttingMechPtr != nullptr) {
        JsonObject cutting = doc.createNestedObject("cutting");
        cutting["running"] = cuttingMechPtr->isRunning();
        cutting["safetyLocked"] = cuttingMechPtr->isSafetyLocked();
    }

    // Perimeter
    #if ENABLE_PERIMETER
    if (perimeterReceiverPtr != nullptr) {
        JsonObject perimeter = doc.createNestedObject("perimeter");
        perimeter["state"] = perimeterReceiverPtr->getStateString();
        perimeter["hasSignal"] = perimeterReceiverPtr->hasSignal();
        perimeter["isInside"] = perimeterReceiverPtr->isInside();
        perimeter["signalStrength"] = perimeterReceiverPtr->getSignalStrength();
        perimeter["direction"] = perimeterReceiverPtr->getDirectionString();
        perimeter["distanceToCable"] = perimeterReceiverPtr->getDistanceToCable();

        if (perimeterClientPtr != nullptr) {
            perimeter["senderConnected"] = perimeterClientPtr->isConnected();
            perimeter["senderRunning"] = perimeterClientPtr->isSenderRunning();
            perimeter["senderState"] = perimeterClientPtr->getSenderState();
        }
    }
    #endif

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

// ============================================================================
// Manuel kontrol handlers
// ============================================================================

void WebAPI::handleManualForward(AsyncWebServerRequest *request) {
    if (motorsPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Motors not initialized\"}");
        return;
    }

    // Skift til manuel kontrol tilstand
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->setState(STATE_MANUAL);
    }

    int speed = MOTOR_CRUISE_SPEED;
    if (request->hasParam("speed", true)) {
        speed = request->getParam("speed", true)->value().toInt();
    }

    motorsPtr->forward(speed);
    request->send(200, "application/json", "{\"status\":\"forward\",\"speed\":" + String(speed) + "}");
    Logger::info("API: Manual forward - speed: " + String(speed));
}

void WebAPI::handleManualBackward(AsyncWebServerRequest *request) {
    if (motorsPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Motors not initialized\"}");
        return;
    }

    // Skift til manuel kontrol tilstand
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->setState(STATE_MANUAL);
    }

    int speed = MOTOR_CRUISE_SPEED;
    if (request->hasParam("speed", true)) {
        speed = request->getParam("speed", true)->value().toInt();
    }

    motorsPtr->backward(speed);
    request->send(200, "application/json", "{\"status\":\"backward\",\"speed\":" + String(speed) + "}");
    Logger::info("API: Manual backward - speed: " + String(speed));
}

void WebAPI::handleManualLeft(AsyncWebServerRequest *request) {
    if (motorsPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Motors not initialized\"}");
        return;
    }

    // Skift til manuel kontrol tilstand
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->setState(STATE_MANUAL);
    }

    int speed = MOTOR_TURN_SPEED;
    if (request->hasParam("speed", true)) {
        speed = request->getParam("speed", true)->value().toInt();
    }

    motorsPtr->turnLeft(speed);
    request->send(200, "application/json", "{\"status\":\"left\",\"speed\":" + String(speed) + "}");
    Logger::info("API: Manual left - speed: " + String(speed));
}

void WebAPI::handleManualRight(AsyncWebServerRequest *request) {
    if (motorsPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Motors not initialized\"}");
        return;
    }

    // Skift til manuel kontrol tilstand
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->setState(STATE_MANUAL);
    }

    int speed = MOTOR_TURN_SPEED;
    if (request->hasParam("speed", true)) {
        speed = request->getParam("speed", true)->value().toInt();
    }

    motorsPtr->turnRight(speed);
    request->send(200, "application/json", "{\"status\":\"right\",\"speed\":" + String(speed) + "}");
    Logger::info("API: Manual right - speed: " + String(speed));
}

void WebAPI::handleManualStop(AsyncWebServerRequest *request) {
    if (motorsPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Motors not initialized\"}");
        return;
    }

    motorsPtr->stop();

    // Bliv i manuel tilstand - lad brugeren bestemme når de vil forlade manuel mode
    // De kan bruge "Stop" knappen i hovedkontrollen for at gå tilbage til IDLE
    if (stateManagerPtr != nullptr && stateManagerPtr->getState() != STATE_MANUAL) {
        stateManagerPtr->setState(STATE_MANUAL);
    }

    request->send(200, "application/json", "{\"status\":\"stopped\"}");
    Logger::info("API: Manual stop");
}

void WebAPI::handleManualSetSpeed(AsyncWebServerRequest *request) {
    if (motorsPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Motors not initialized\"}");
        return;
    }

    if (!request->hasParam("left", true) || !request->hasParam("right", true)) {
        request->send(400, "application/json", "{\"error\":\"Missing left or right speed parameter\"}");
        return;
    }

    // Skift til manuel kontrol tilstand
    if (stateManagerPtr != nullptr) {
        stateManagerPtr->setState(STATE_MANUAL);
    }

    int leftSpeed = request->getParam("left", true)->value().toInt();
    int rightSpeed = request->getParam("right", true)->value().toInt();

    motorsPtr->setSpeed(leftSpeed, rightSpeed);
    request->send(200, "application/json",
                 "{\"status\":\"speed_set\",\"left\":" + String(leftSpeed) +
                 ",\"right\":" + String(rightSpeed) + "}");
    Logger::info("API: Manual set speed - left: " + String(leftSpeed) + ", right: " + String(rightSpeed));
}

void WebAPI::handleCuttingStart(AsyncWebServerRequest *request) {
    if (cuttingMechPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Cutting mechanism not initialized\"}");
        return;
    }

    cuttingMechPtr->start();
    request->send(200, "application/json", "{\"status\":\"cutting_started\"}");
    Logger::info("API: Cutting mechanism started");
}

void WebAPI::handleCuttingStop(AsyncWebServerRequest *request) {
    if (cuttingMechPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Cutting mechanism not initialized\"}");
        return;
    }

    cuttingMechPtr->stop();
    request->send(200, "application/json", "{\"status\":\"cutting_stopped\"}");
    Logger::info("API: Cutting mechanism stopped");
}

void WebAPI::handleGetCurrent(AsyncWebServerRequest *request) {
    if (motorsPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Motors not initialized\"}");
        return;
    }

    StaticJsonDocument<256> doc;
    doc["leftCurrent"] = motorsPtr->getLeftCurrent();
    doc["rightCurrent"] = motorsPtr->getRightCurrent();
    doc["totalCurrent"] = motorsPtr->getTotalCurrent();
    doc["warning"] = motorsPtr->isCurrentWarning();
    doc["maxCurrent"] = MOTOR_CURRENT_MAX;
    doc["warningThreshold"] = MOTOR_CURRENT_WARNING;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

// ============================================================================
// Perimeter handlers
// ============================================================================

#if ENABLE_PERIMETER
void WebAPI::setPerimeterReferences(PerimeterReceiver* receiver, PerimeterClient* client) {
    perimeterReceiverPtr = receiver;
    perimeterClientPtr = client;
    Logger::info("WebAPI perimeter references set");
}

void WebAPI::handlePerimeterStatus(AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;

    // Receiver status
    if (perimeterReceiverPtr != nullptr) {
        JsonObject receiver = doc.createNestedObject("receiver");
        receiver["state"] = perimeterReceiverPtr->getStateString();
        receiver["hasSignal"] = perimeterReceiverPtr->hasSignal();
        receiver["isInside"] = perimeterReceiverPtr->isInside();
        receiver["isOutside"] = perimeterReceiverPtr->isOutside();
        receiver["signalStrength"] = perimeterReceiverPtr->getSignalStrength();
        receiver["signalMagnitude"] = perimeterReceiverPtr->getSignalMagnitude();
        receiver["direction"] = perimeterReceiverPtr->getDirectionString();
        receiver["distanceToCable"] = perimeterReceiverPtr->getDistanceToCable();
    }

    // Sender status
    if (perimeterClientPtr != nullptr) {
        JsonObject sender = doc.createNestedObject("sender");
        sender["connected"] = perimeterClientPtr->isConnected();
        sender["running"] = perimeterClientPtr->isSenderRunning();
        sender["state"] = perimeterClientPtr->getSenderState();
        sender["current_mA"] = perimeterClientPtr->getSenderCurrent();
        sender["runtime_ms"] = perimeterClientPtr->getSenderRuntime();
        sender["ip"] = perimeterClientPtr->getSenderIP();
    }

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebAPI::handlePerimeterStart(AsyncWebServerRequest *request) {
    if (perimeterClientPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Perimeter client not initialized\"}");
        return;
    }

    bool success = perimeterClientPtr->startSignal();
    if (success) {
        request->send(200, "application/json", "{\"status\":\"started\",\"message\":\"Perimeter signal started\"}");
        Logger::info("API: Perimeter signal started");
    } else {
        String error = perimeterClientPtr->getLastError();
        request->send(500, "application/json", "{\"error\":\"" + error + "\"}");
        Logger::error("API: Failed to start perimeter signal: " + error);
    }
}

void WebAPI::handlePerimeterStop(AsyncWebServerRequest *request) {
    if (perimeterClientPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Perimeter client not initialized\"}");
        return;
    }

    bool success = perimeterClientPtr->stopSignal();
    if (success) {
        request->send(200, "application/json", "{\"status\":\"stopped\",\"message\":\"Perimeter signal stopped\"}");
        Logger::info("API: Perimeter signal stopped");
    } else {
        String error = perimeterClientPtr->getLastError();
        request->send(500, "application/json", "{\"error\":\"" + error + "\"}");
        Logger::error("API: Failed to stop perimeter signal: " + error);
    }
}

void WebAPI::handlePerimeterCalibrate(AsyncWebServerRequest *request) {
    if (perimeterReceiverPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"Perimeter receiver not initialized\"}");
        return;
    }

    Logger::info("API: Perimeter calibration requested - place coil on wire!");
    perimeterReceiverPtr->calibrate();
    request->send(200, "application/json",
        "{\"status\":\"calibrated\",\"message\":\"Perimeter calibration complete\"}");
}

void WebAPI::handleReturnToBase(AsyncWebServerRequest *request) {
    if (stateManagerPtr == nullptr) {
        request->send(500, "application/json", "{\"error\":\"State manager not initialized\"}");
        return;
    }

    if (perimeterReceiverPtr == nullptr || !perimeterReceiverPtr->hasSignal()) {
        request->send(400, "application/json",
            "{\"error\":\"No perimeter signal - cannot return to base\"}");
        return;
    }

    stateManagerPtr->returnToBase();
    request->send(200, "application/json",
        "{\"status\":\"returning\",\"message\":\"Robot is returning to base following perimeter wire\"}");
    Logger::info("API: Return to base requested");
}
#endif
