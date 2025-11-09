#include "WebServer.h"
#include <LittleFS.h>

WebServer::WebServer() {
    server = nullptr;
    wifiConnected = false;
    apMode = false;
    ipAddress = "";
    lastWiFiCheck = 0;
    initialized = false;
    wifiManager = nullptr;
    updateManager = nullptr;
}

bool WebServer::begin() {
    Logger::info("Starting web server...");

    // Initialize LittleFS for serving static files
    if (!LittleFS.begin(true)) {
        Logger::warning("LittleFS Mount Failed - continuing without static files");
    } else {
        Logger::info("LittleFS mounted successfully");
    }

    // Opret server objekt
    server = new AsyncWebServer(WEB_SERVER_PORT);

    if (server == nullptr) {
        Logger::error("Failed to create web server");
        return false;
    }

    // Forbind til WiFi
    #if ENABLE_WIFI_MANAGER
    // Hvis WiFiManager er sat, brug den (den håndterer forbindelse selv)
    if (wifiManager != nullptr) {
        wifiConnected = wifiManager->isConnected();
        apMode = wifiManager->isAPMode();
        ipAddress = wifiManager->getIPAddress();
        Logger::info("Using WiFiManager - AP Mode: " + String(apMode ? "YES" : "NO"));
    } else {
        Logger::warning("WiFiManager not set - using fallback");
        if (!connectWiFi()) {
            Logger::warning("WiFi connection failed - starting Access Point");
            setupAccessPoint();
        }
    }
    #else
    // Gammel adfærd uden WiFiManager
    if (!connectWiFi()) {
        Logger::warning("WiFi connection failed - starting Access Point");
        setupAccessPoint();
    }
    #endif

    // Opsæt mDNS
    #if ENABLE_MDNS
    setupMDNS();
    #endif

    // Opsæt OTA
    #if ENABLE_OTA
    setupOTA();
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

    // Håndter OTA opdateringer
    #if ENABLE_OTA
    handleOTA();
    #endif

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

    // Serve static files from LittleFS
    // Root - serve index.html
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/index.html")) {
            request->send(LittleFS, "/index.html", "text/html");
        } else {
            // Fallback if LittleFS not available
            String html = "<!DOCTYPE html><html><head><title>Robot Mower</title>";
            html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
            html += "</head><body>";
            html += "<h1>Robot Mower Control</h1>";
            html += "<p>Welcome to Robot Mower web interface!</p>";
            html += "<p><a href='/api/status'>View Status (JSON)</a></p>";
            html += "<p>Note: Upload data files to enable full web interface</p>";
            html += "</body></html>";
            request->send(200, "text/html", html);
        }
    });

    // Serve CSS file
    server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/style.css")) {
            request->send(LittleFS, "/style.css", "text/css");
        } else {
            request->send(404, "text/plain", "File not found");
        }
    });

    // Serve JavaScript file
    server->on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/app.js")) {
            request->send(LittleFS, "/app.js", "application/javascript");
        } else {
            request->send(404, "text/plain", "File not found");
        }
    });

    // OTA update endpoint (Web Upload)
    #if ENABLE_OTA
    server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html><html><head><title>OTA Update</title>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0;}";
        html += ".container{max-width:600px;margin:auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
        html += "h1{color:#333;}input[type=file]{margin:20px 0;}";
        html += "input[type=submit]{background:#4CAF50;color:white;padding:12px 30px;border:none;border-radius:5px;cursor:pointer;font-size:16px;}";
        html += "input[type=submit]:hover{background:#45a049;}</style>";
        html += "</head><body><div class='container'>";
        html += "<h1>🔄 Firmware Update</h1>";
        html += "<p>Upload ny firmware (.bin fil)</p>";
        html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
        html += "<input type='file' name='update' accept='.bin' required><br>";
        html += "<input type='submit' value='Upload Firmware'>";
        html += "</form></div></body></html>";
        request->send(200, "text/html", html);
    });

    server->on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool shouldReboot = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
            shouldReboot ?
            "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Update Success</title></head><body><h1>✅ Update Success!</h1><p>Robotten genstarter...</p></body></html>" :
            "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Update Failed</title></head><body><h1>❌ Update Failed!</h1><p>Prøv igen.</p></body></html>");
        response->addHeader("Connection", "close");
        request->send(response);

        if (shouldReboot) {
            Logger::info("OTA Update successful - rebooting...");
            delay(1000);
            ESP.restart();
        }
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Logger::info("OTA Update Start: " + filename);
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        }
        if (!Update.hasError()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
        }
        if (final) {
            if (Update.end(true)) {
                Logger::info("OTA Update Success: " + String(index + len) + " bytes");
            } else {
                Update.printError(Serial);
            }
        }
    });
    #endif

    // WiFi Manager routes
    #if ENABLE_WIFI_MANAGER
    // Captive portal page
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Hvis i AP mode, vis captive portal
        if (wifiManager != nullptr && wifiManager->isAPMode()) {
            request->send(200, "text/html", WiFiManager::getCaptivePortalHTML());
        } else {
            // Normal root handling
            if (LittleFS.exists("/index.html")) {
                request->send(LittleFS, "/index.html", "text/html");
            } else {
                String html = "<!DOCTYPE html><html><head><title>Robot Mower</title>";
                html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
                html += "</head><body>";
                html += "<h1>Robot Mower Control</h1>";
                html += "<p>Welcome! <a href='/api/status'>View Status</a></p>";
                html += "</body></html>";
                request->send(200, "text/html", html);
            }
        }
    });

    // WiFi scan endpoint
    server->on("/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        int n = WiFi.scanNetworks();
        String json = "[";
        for (int i = 0; i < n; i++) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
            json += "\"encryption\":\"" + String((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured") + "\"";
            json += "}";
        }
        json += "]";
        request->send(200, "application/json", json);
    });

    // WiFi save credentials
    server->on("/wifi/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (wifiManager == nullptr) {
            request->send(500, "application/json", "{\"error\":\"WiFiManager not available\"}");
            return;
        }

        String ssid = "";
        String password = "";

        if (request->hasParam("ssid", true)) {
            ssid = request->getParam("ssid", true)->value();
        }
        if (request->hasParam("password", true)) {
            password = request->getParam("password", true)->value();
        }

        if (ssid.length() > 0) {
            wifiManager->saveCredentials(ssid, password);
            request->send(200, "application/json", "{\"status\":\"saved\"}");

            // Stop captive portal og reconnect
            Logger::info("Credentials saved, reconnecting...");
            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "application/json", "{\"error\":\"Missing SSID\"}");
        }
    });

    // WiFi reset (force AP mode)
    server->on("/wifi/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (wifiManager != nullptr) {
            wifiManager->clearCredentials();
            request->send(200, "application/json", "{\"status\":\"cleared\"}");
            delay(1000);
            ESP.restart();
        } else {
            request->send(500, "application/json", "{\"error\":\"WiFiManager not available\"}");
        }
    });
    #endif

    // Auto-update routes
    #if ENABLE_AUTO_UPDATE
    // Check for updates
    server->on("/api/update/check", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (updateManager == nullptr) {
            request->send(500, "application/json", "{\"error\":\"UpdateManager not available\"}");
            return;
        }

        bool updateAvailable = updateManager->checkForUpdate();

        String json = "{";
        json += "\"updateAvailable\":" + String(updateAvailable ? "true" : "false") + ",";
        json += "\"currentVersion\":\"" + updateManager->getCurrentVersion() + "\",";
        json += "\"latestVersion\":\"" + updateManager->getLatestVersion() + "\",";
        json += "\"status\":\"" + String(updateManager->getStatus()) + "\"";
        json += "}";

        request->send(200, "application/json", json);
    });

    // Perform update
    server->on("/api/update/install", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (updateManager == nullptr) {
            request->send(500, "application/json", "{\"error\":\"UpdateManager not available\"}");
            return;
        }

        request->send(200, "application/json", "{\"status\":\"updating\"}");

        // Start update i baggrunden
        delay(500);
        updateManager->performUpdate();
        // Hvis vi kommer hertil, fejlede update
        // (normalt genstarter ESP efter succesfuld update)
    });

    // Get update status
    server->on("/api/update/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (updateManager == nullptr) {
            request->send(500, "application/json", "{\"error\":\"UpdateManager not available\"}");
            return;
        }

        String json = "{";
        json += "\"status\":" + String(updateManager->getStatus()) + ",";
        json += "\"progress\":" + String(updateManager->getProgress()) + ",";
        json += "\"error\":\"" + updateManager->getError() + "\"";
        json += "}";

        request->send(200, "application/json", json);
    });

    // Get version info
    server->on("/api/update/version", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (updateManager == nullptr) {
            request->send(500, "application/json", "{\"error\":\"UpdateManager not available\"}");
            return;
        }

        String json = "{";
        json += "\"current\":\"" + updateManager->getCurrentVersion() + "\",";
        json += "\"latest\":\"" + updateManager->getLatestVersion() + "\"";
        json += "}";

        request->send(200, "application/json", json);
    });
    #endif

    // Not found handler
    server->onNotFound([this](AsyncWebServerRequest *request) {
        handleNotFound(request);
    });

    Logger::info("Web routes configured (with WiFiManager and UpdateManager support)");
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


void WebServer::handleNotFound(AsyncWebServerRequest *request) {
    String message = "404 Not Found\n\n";
    message += "URI: " + request->url() + "\n";
    message += "Method: " + String((request->method() == HTTP_GET) ? "GET" : "POST") + "\n";

    request->send(404, "text/plain", message);
    Logger::warning("404: " + request->url());
}

void WebServer::setupOTA() {
    // Opsæt ArduinoOTA
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setPort(OTA_PORT);

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Logger::info("OTA: Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
        Logger::info("OTA: Update complete");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static unsigned int lastPercent = 0;
        unsigned int percent = (progress / (total / 100));
        if (percent != lastPercent && percent % 10 == 0) {
            Logger::info("OTA Progress: " + String(percent) + "%");
            lastPercent = percent;
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        String errorMsg = "OTA Error[" + String(error) + "]: ";
        if (error == OTA_AUTH_ERROR) errorMsg += "Auth Failed";
        else if (error == OTA_BEGIN_ERROR) errorMsg += "Begin Failed";
        else if (error == OTA_CONNECT_ERROR) errorMsg += "Connect Failed";
        else if (error == OTA_RECEIVE_ERROR) errorMsg += "Receive Failed";
        else if (error == OTA_END_ERROR) errorMsg += "End Failed";
        Logger::error(errorMsg);
    });

    ArduinoOTA.begin();

    Logger::info("ArduinoOTA initialized");
    Logger::info("OTA Hostname: " + String(OTA_HOSTNAME));
    Logger::info("OTA Port: " + String(OTA_PORT));
}

void WebServer::handleOTA() {
    ArduinoOTA.handle();
}

void WebServer::setWiFiManager(WiFiManager* wifiMgr) {
    wifiManager = wifiMgr;
    Logger::info("WiFiManager reference set");
}

void WebServer::setUpdateManager(UpdateManager* updateMgr) {
    updateManager = updateMgr;
    Logger::info("UpdateManager reference set");
}
