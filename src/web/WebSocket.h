#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "../config/Config.h"
#include "../system/Logger.h"
#include "WebServer.h"

/**
 * WebSocket klasse - Håndterer real-time WebSocket kommunikation
 *
 * Denne klasse broadcaster real-time data til web klienter.
 */
class WebSocketHandler {
public:
    /**
     * Constructor
     */
    WebSocketHandler();

    /**
     * Initialiserer WebSocket
     * @param webServer Pointer til WebServer objekt
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin(WebServer* webServer);

    /**
     * Opdaterer WebSocket
     * Kalder denne regelmæssigt i loop()
     */
    void update();

    /**
     * Broadcaster status til alle klienter
     * @param statusJSON JSON status string
     */
    void broadcastStatus(String statusJSON);

    /**
     * Broadcaster sensor data
     * @param left Venstre sensor
     * @param middle Midter sensor
     * @param right Højre sensor
     */
    void broadcastSensorData(float left, float middle, float right);

    /**
     * Broadcaster log besked
     * @param message Log besked
     */
    void broadcastLog(String message);

    /**
     * Hent antal forbundne klienter
     * @return Antal klienter
     */
    int getClientCount();

    /**
     * Sætter hardware referencer
     * @param state StateManager pointer
     * @param mot Motors pointer
     * @param cut CuttingMechanism pointer
     */
    void setHardwareReferences(class StateManager* state, class Motors* mot, class CuttingMechanism* cut);

private:
    /**
     * WebSocket event handler
     */
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len);

    /**
     * Håndterer WebSocket besked fra klient
     */
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

    // WebSocket objekt
    AsyncWebSocket* ws;

    // Timing
    unsigned long lastBroadcast;

    // State
    bool initialized;

    // Hardware referencer
    class StateManager* stateManagerPtr;
    class Motors* motorsPtr;
    class CuttingMechanism* cuttingMechPtr;
};

#endif // WEBSOCKET_H
