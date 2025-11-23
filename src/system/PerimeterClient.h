#ifndef PERIMETER_CLIENT_H
#define PERIMETER_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../config/Config.h"

/**
 * PerimeterClient - HTTP klient til perimeter wire sender
 *
 * Kommunikerer med den separate ESP32 der styrer perimeterkablet.
 * Tillader robot mower at tænde/slukke kablet og overvåge status.
 */
class PerimeterClient {
public:
    PerimeterClient();

    /**
     * Initialiserer klienten
     * @param ip IP adresse på perimeter sender
     * @param port HTTP port (default 80)
     * @return true hvis initialisering lykkedes
     */
    bool begin(const char* ip = PERIMETER_SENDER_IP, int port = PERIMETER_SENDER_PORT);

    /**
     * Starter perimeter wire signalet
     * @return true hvis kommandoen lykkedes
     */
    bool startSignal();

    /**
     * Stopper perimeter wire signalet
     * @return true hvis kommandoen lykkedes
     */
    bool stopSignal();

    /**
     * Nulstiller perimeter sender efter fejl
     * @return true hvis kommandoen lykkedes
     */
    bool resetSender();

    /**
     * Henter status fra perimeter sender
     * @return true hvis status blev hentet
     */
    bool updateStatus();

    /**
     * Tjekker om senderen kører
     */
    bool isSenderRunning() const { return _senderRunning; }

    /**
     * Tjekker om der er forbindelse til senderen
     */
    bool isConnected() const { return _connected; }

    /**
     * Henter sender tilstand som tekst
     */
    String getSenderState() const { return _senderState; }

    /**
     * Henter strømmåling fra sender (mA)
     */
    float getSenderCurrent() const { return _senderCurrent; }

    /**
     * Henter sender runtime (ms)
     */
    unsigned long getSenderRuntime() const { return _senderRuntime; }

    /**
     * Henter sidste fejlmeddelelse
     */
    String getLastError() const { return _lastError; }

    /**
     * Sætter sender IP adresse
     */
    void setSenderIP(const char* ip);

    /**
     * Henter sender IP adresse
     */
    String getSenderIP() const { return _senderIP; }

private:
    String _senderIP;
    int _senderPort;
    bool _initialized;
    bool _connected;
    bool _senderRunning;
    String _senderState;
    float _senderCurrent;
    unsigned long _senderRuntime;
    String _lastError;

    /**
     * Sender HTTP GET request
     * @param endpoint API endpoint (f.eks. "/api/status")
     * @param response Output parameter for response body
     * @return HTTP status kode eller -1 ved fejl
     */
    int httpGet(const char* endpoint, String& response);

    /**
     * Sender HTTP POST request
     * @param endpoint API endpoint (f.eks. "/api/start")
     * @param response Output parameter for response body
     * @return HTTP status kode eller -1 ved fejl
     */
    int httpPost(const char* endpoint, String& response);

    /**
     * Parser status JSON fra sender
     */
    bool parseStatusResponse(const String& json);

    /**
     * Bygger fuld URL fra endpoint
     */
    String buildURL(const char* endpoint);
};

#endif // PERIMETER_CLIENT_H
