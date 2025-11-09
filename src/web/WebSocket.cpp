#include "WebSocket.h"

WebSocketHandler::WebSocketHandler() {
    ws = nullptr;
    lastBroadcast = 0;
    initialized = false;
    stateManagerPtr = nullptr;
    motorsPtr = nullptr;
    cuttingMechPtr = nullptr;
}

bool WebSocketHandler::begin(WebServer* webServer) {
    if (webServer == nullptr) {
        Logger::error("WebSocket: Invalid WebServer pointer");
        return false;
    }

    // Opret WebSocket på /ws endpoint
    ws = new AsyncWebSocket("/ws");

    if (ws == nullptr) {
        Logger::error("Failed to create WebSocket");
        return false;
    }

    // Opsæt event handler (lambda med this capture)
    ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onEvent(server, client, type, arg, data, len);
    });

    // Tilføj WebSocket handler til server
    AsyncWebServer* server = webServer->getServer();
    if (server != nullptr) {
        server->addHandler(ws);
    }

    initialized = true;

    Logger::info("WebSocket initialized on /ws");

    return true;
}

void WebSocketHandler::setHardwareReferences(StateManager* state, Motors* mot, CuttingMechanism* cut) {
    stateManagerPtr = state;
    motorsPtr = mot;
    cuttingMechPtr = cut;

    Logger::info("WebSocket hardware references set");
}

void WebSocketHandler::update() {
    if (!initialized || ws == nullptr) {
        return;
    }

    // Cleanup disconnected clients
    ws->cleanupClients();
}

void WebSocketHandler::broadcastStatus(String statusJSON) {
    if (!initialized || ws == nullptr) {
        return;
    }

    #if ENABLE_WEBSOCKET
    // Opret WebSocket besked
    StaticJsonDocument<64> doc;
    doc["type"] = "status";

    String message;
    serializeJson(doc, message);

    // Tilføj status data
    message = message.substring(0, message.length() - 1); // Fjern sidste }
    message += ",\"data\":";
    message += statusJSON;
    message += "}";

    ws->textAll(message);
    #endif
}

void WebSocketHandler::broadcastSensorData(float left, float middle, float right) {
    if (!initialized || ws == nullptr) {
        return;
    }

    #if ENABLE_WEBSOCKET
    StaticJsonDocument<128> doc;
    doc["type"] = "sensor";

    JsonObject data = doc.createNestedObject("data");
    data["left"] = left;
    data["middle"] = middle;
    data["right"] = right;

    String message;
    serializeJson(doc, message);

    ws->textAll(message);
    #endif
}

void WebSocketHandler::broadcastLog(String message) {
    if (!initialized || ws == nullptr) {
        return;
    }

    #if ENABLE_WEBSOCKET && LOG_TO_WEBSOCKET
    StaticJsonDocument<256> doc;
    doc["type"] = "log";
    doc["message"] = message;
    doc["timestamp"] = millis();

    String output;
    serializeJson(doc, output);

    ws->textAll(output);
    #endif
}

int WebSocketHandler::getClientCount() {
    if (ws == nullptr) {
        return 0;
    }
    return ws->count();
}

void WebSocketHandler::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Logger::info("WebSocket client #" + String(client->id()) + " connected");
            break;

        case WS_EVT_DISCONNECT:
            Logger::info("WebSocket client #" + String(client->id()) + " disconnected");
            break;

        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;

        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebSocketHandler::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; // Null terminate

        // Parse JSON command
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            Logger::error("WebSocket: Failed to parse message");
            return;
        }

        // Håndter kommandoer
        String command = doc["command"];

        #if DEBUG_WEBSOCKET
        Logger::debug("WebSocket command: " + command);
        #endif

        // Håndter automatisk kontrol kommandoer
        if (command == "start" && stateManagerPtr != nullptr) {
            stateManagerPtr->startMowing();
            Logger::info("WS: Start mowing");
        }
        else if (command == "stop" && stateManagerPtr != nullptr) {
            stateManagerPtr->stopMowing();
            Logger::info("WS: Stop mowing");
        }
        else if (command == "pause" && stateManagerPtr != nullptr) {
            stateManagerPtr->pauseMowing();
            Logger::info("WS: Pause mowing");
        }
        else if (command == "calibrate") {
            Logger::info("WS: Calibration requested");
            // Calibration håndteres i main loop
        }
        // Håndter manuel kontrol kommandoer
        else if (command == "forward" && motorsPtr != nullptr) {
            if (stateManagerPtr != nullptr) {
                stateManagerPtr->setState(STATE_MANUAL);
            }
            motorsPtr->forward(MOTOR_CRUISE_SPEED);
            Logger::info("WS: Manual forward");
        }
        else if (command == "backward" && motorsPtr != nullptr) {
            if (stateManagerPtr != nullptr) {
                stateManagerPtr->setState(STATE_MANUAL);
            }
            motorsPtr->backward(MOTOR_CRUISE_SPEED);
            Logger::info("WS: Manual backward");
        }
        else if (command == "left" && motorsPtr != nullptr) {
            if (stateManagerPtr != nullptr) {
                stateManagerPtr->setState(STATE_MANUAL);
            }
            motorsPtr->turnLeft(MOTOR_TURN_SPEED);
            Logger::info("WS: Manual left");
        }
        else if (command == "right" && motorsPtr != nullptr) {
            if (stateManagerPtr != nullptr) {
                stateManagerPtr->setState(STATE_MANUAL);
            }
            motorsPtr->turnRight(MOTOR_TURN_SPEED);
            Logger::info("WS: Manual right");
        }
        else if (command == "manualStop" && motorsPtr != nullptr) {
            motorsPtr->stop();
            Logger::info("WS: Manual stop");
        }
        // Håndter klippemotor kommandoer
        else if (command == "cuttingStart" && cuttingMechPtr != nullptr) {
            cuttingMechPtr->start();
            Logger::info("WS: Cutting started");
        }
        else if (command == "cuttingStop" && cuttingMechPtr != nullptr) {
            cuttingMechPtr->stop();
            Logger::info("WS: Cutting stopped");
        }
        else {
            Logger::warn("WS: Unknown command: " + command);
        }
    }
}
