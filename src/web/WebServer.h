#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>

// Define HTTP method constants if not already defined
// These are used by ESPAsyncWebServer but sometimes not properly included
#ifndef HTTP_GET
  #define HTTP_GET     0b00000001
  #define HTTP_POST    0b00000010
  #define HTTP_DELETE  0b00000100
  #define HTTP_PUT     0b00001000
  #define HTTP_PATCH   0b00010000
  #define HTTP_HEAD    0b00100000
  #define HTTP_OPTIONS 0b01000000
  #define HTTP_ANY     0b01111111
#endif

#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include "../config/Config.h"
#include "../config/Credentials.h"
#include "../system/Logger.h"
#include "../system/WiFiManager.h"
#include "../system/UpdateManager.h"

/**
 * MowerWebServer klasse - Håndterer HTTP web server
 *
 * Denne klasse opsætter og håndterer web serveren til
 * robot kontrol interface.
 */
class MowerWebServer {
public:
    /**
     * Constructor
     */
    MowerWebServer();

    /**
     * Initialiserer web server og WiFi
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Opdaterer web server
     * Kalder denne regelmæssigt i loop()
     */
    void update();

    /**
     * Opsætter alle HTTP routes
     */
    void setupRoutes();

    /**
     * Tjek om klient er forbundet
     * @return true hvis forbundet
     */
    bool isClientConnected();

    /**
     * Hent server pointer (til brug i andre moduler)
     * @return Pointer til AsyncWebServer
     */
    AsyncWebServer* getServer();

    /**
     * Hent WiFi status
     * @return true hvis forbundet til WiFi
     */
    bool isWiFiConnected();

    /**
     * Hent IP adresse
     * @return IP adresse som String
     */
    String getIPAddress();

    /**
     * Opsætter OTA (Over-The-Air) opdatering
     * Aktiverer både ArduinoOTA og web upload
     */
    void setupOTA();

    /**
     * Håndterer OTA opdateringer
     * Skal kaldes i loop()
     */
    void handleOTA();

    /**
     * Sætter WiFiManager reference
     * @param wifiMgr Pointer til WiFiManager
     */
    void setWiFiManager(WiFiManager* wifiMgr);

    /**
     * Sætter UpdateManager reference
     * @param updateMgr Pointer til UpdateManager
     */
    void setUpdateManager(UpdateManager* updateMgr);

private:
    /**
     * Forbinder til WiFi netværk
     * @return true hvis succesfuld
     */
    bool connectWiFi();

    /**
     * Opsætter Access Point (fallback)
     */
    void setupAccessPoint();

    /**
     * Opsætter mDNS
     */
    void setupMDNS();

    /**
     * Håndterer root request (/)
     */
    void handleRoot(AsyncWebServerRequest *request);

    /**
     * Håndterer 404 fejl
     */
    void handleNotFound(AsyncWebServerRequest *request);

    // Server objekt
    AsyncWebServer* server;

    // WiFi state
    bool wifiConnected;
    bool apMode;
    String ipAddress;

    // Timing
    unsigned long lastWiFiCheck;

    // State
    bool initialized;

    // Manager references
    WiFiManager* wifiManager;
    UpdateManager* updateManager;
};

#endif // WEBSERVER_H
