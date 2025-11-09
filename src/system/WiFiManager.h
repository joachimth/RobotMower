#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "../config/Config.h"
#include "Logger.h"

/**
 * WiFiManager klasse - Håndterer WiFi forbindelse med captive portal
 *
 * Features:
 * - Gemmer WiFi credentials i NVS (persistent storage)
 * - Captive portal ved første opstart
 * - Automatisk retry med fallback til AP mode
 * - Credentials overlever firmware updates
 */
class WiFiManager {
public:
    /**
     * Constructor
     */
    WiFiManager();

    /**
     * Initialiserer WiFi Manager
     * @return true hvis WiFi forbindelse lykkedes
     */
    bool begin();

    /**
     * Tjek WiFi status og reconnect hvis nødvendigt
     * Kalder denne i loop()
     */
    void update();

    /**
     * Gemmer WiFi credentials til NVS
     * @param ssid WiFi SSID
     * @param password WiFi password
     * @return true hvis succesfuld
     */
    bool saveCredentials(const String& ssid, const String& password);

    /**
     * Sletter gemte WiFi credentials
     */
    void clearCredentials();

    /**
     * Tjek om credentials er gemt
     * @return true hvis credentials findes
     */
    bool hasStoredCredentials();

    /**
     * Start captive portal (AP mode)
     */
    void startCaptivePortal();

    /**
     * Stop captive portal
     */
    void stopCaptivePortal();

    /**
     * Tjek om i AP mode
     * @return true hvis i AP mode
     */
    bool isAPMode();

    /**
     * Hent WiFi status
     * @return true hvis forbundet
     */
    bool isConnected();

    /**
     * Hent IP adresse
     * @return IP adresse som String
     */
    String getIPAddress();

    /**
     * Hent SSID (nuværende eller AP)
     * @return SSID
     */
    String getSSID();

    /**
     * Tvinger fallback til AP mode
     */
    void forceAPMode();

    /**
     * Håndter captive portal web requests
     * Skal kaldes fra WebServer
     */
    static String getCaptivePortalHTML();

private:
    /**
     * Prøv at forbinde til WiFi
     * @param ssid WiFi SSID
     * @param password WiFi password
     * @return true hvis forbundet
     */
    bool connectWiFi(const String& ssid, const String& password);

    /**
     * Indlæs credentials fra NVS
     * @return true hvis credentials fundet
     */
    bool loadCredentials();

    /**
     * Opsæt captive portal DNS
     */
    void setupCaptiveDNS();

    // Preferences objekt til NVS storage
    Preferences preferences;

    // DNS Server til captive portal
    DNSServer* dnsServer;

    // State
    bool apMode;
    bool hasCredentials;
    int failedAttempts;
    unsigned long lastReconnectAttempt;

    // Stored credentials
    String storedSSID;
    String storedPassword;

    // Constants
    static const int MAX_FAILED_ATTEMPTS = 10;
    static const int RECONNECT_INTERVAL = 30000; // 30 sekunder
};

#endif // WIFIMANAGER_H
