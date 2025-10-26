#include "WebServer.h"

WebServer::WebServer() {
    server = nullptr;
    wifiConnected = false;
    apMode = false;
    ipAddress = "";
    lastWiFiCheck = 0;
    initialized = false;
}

bool WebServer::begin() {
    Logger::info("Starting web server...");

    // Opret server objekt
    server = new AsyncWebServer(WEB_SERVER_PORT);

    if (server == nullptr) {
        Logger::error("Failed to create web server");
        return false;
    }

    // Forbind til WiFi
    if (!connectWiFi()) {
        Logger::warning("WiFi connection failed - starting Access Point");
        setupAccessPoint();
    }

    // Opsæt mDNS
    #if ENABLE_MDNS
    setupMDNS();
    #endif

    // Opsæt routes
    setupRoutes();

    // Start server
    server->begin();

    initialized = true;

    Logger::info("Web server started successfully");
    Logger::info("IP: " + ipAddress);

    return true;
}

void WebServer::update() {
    if (!initialized) {
        return;
    }

    // Tjek WiFi status periodisk
    unsigned long currentTime = millis();
    if (currentTime - lastWiFiCheck > 30000) { // Hver 30 sekund
        lastWiFiCheck = currentTime;

        if (!apMode && WiFi.status() != WL_CONNECTED) {
            Logger::warning("WiFi connection lost - reconnecting...");
            wifiConnected = false;
            connectWiFi();
        }
    }
}

void WebServer::setupRoutes() {
    if (server == nullptr) {
        return;
    }

    // Root route
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleRoot(request);
    });

    // Not found handler
    server->onNotFound([this](AsyncWebServerRequest *request) {
        handleNotFound(request);
    });

    Logger::info("Web routes configured");
}

bool WebServer::isClientConnected() {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

AsyncWebServer* WebServer::getServer() {
    return server;
}

bool WebServer::isWiFiConnected() {
    return wifiConnected;
}

String WebServer::getIPAddress() {
    return ipAddress;
}

bool WebServer::connectWiFi() {
    Logger::info("Connecting to WiFi: " + String(WIFI_SSID));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttempt < WIFI_TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        ipAddress = WiFi.localIP().toString();
        Logger::info("WiFi connected! IP: " + ipAddress);
        return true;
    } else {
        wifiConnected = false;
        Logger::error("WiFi connection failed");
        return false;
    }
}

void WebServer::setupAccessPoint() {
    Logger::info("Setting up Access Point: " + String(AP_SSID));

    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONNECTIONS);

    if (success) {
        apMode = true;
        ipAddress = WiFi.softAPIP().toString();
        Logger::info("AP started! IP: " + ipAddress);
    } else {
        Logger::error("Failed to start Access Point");
    }
}

void WebServer::setupMDNS() {
    if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", WEB_SERVER_PORT);
        Logger::info("mDNS started: " + String(MDNS_HOSTNAME) + ".local");
    } else {
        Logger::error("mDNS failed to start");
    }
}

void WebServer::handleRoot(AsyncWebServerRequest *request) {
    // Send HTML response
    String html = "<!DOCTYPE html><html><head><title>Robot Mower</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "</head><body>";
    html += "<h1>Robot Mower Control</h1>";
    html += "<p>Welcome to Robot Mower web interface!</p>";
    html += "<p><a href='/api/status'>View Status (JSON)</a></p>";
    html += "<p>Full web interface will be served from /data/ folder</p>";
    html += "</body></html>";

    request->send(200, "text/html", html);
}

void WebServer::handleNotFound(AsyncWebServerRequest *request) {
    String message = "404 Not Found\n\n";
    message += "URI: " + request->url() + "\n";
    message += "Method: " + String((request->method() == HTTP_GET) ? "GET" : "POST") + "\n";

    request->send(404, "text/plain", message);
    Logger::warning("404: " + request->url());
}
