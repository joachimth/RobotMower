#include "WebServer.h"
#include "../config/Credentials.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

PerimeterWebServer::PerimeterWebServer()
    : _server(WEB_SERVER_PORT)
    , _signalGen(nullptr)
    , _running(false)
    , _apMode(false)
{
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

bool PerimeterWebServer::begin(SignalGenerator* signalGen) {
    _signalGen = signalGen;

    Serial.println("[WebServer] Initializing...");

    // Initialiser LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[WebServer] WARNING: LittleFS mount failed");
    }

    // Forbind til WiFi eller start AP
    if (!connectWiFi()) {
        Serial.println("[WebServer] WiFi connection failed, starting AP mode...");
        startAP();
    }

    // Start mDNS
    if (MDNS.begin(MDNS_HOSTNAME)) {
        Serial.printf("[WebServer] mDNS started: http://%s.local\n", MDNS_HOSTNAME);
        MDNS.addService("http", "tcp", WEB_SERVER_PORT);
    }

    // Setup routes
    setupRoutes();

    // Start server
    _server.begin();
    _running = true;

    Serial.println("[WebServer] Server started");
    Serial.printf("[WebServer] Access at: http://%s\n", getIPAddress().c_str());

    return true;
}

void PerimeterWebServer::update() {
    // AsyncWebServer håndterer requests automatisk
}

String PerimeterWebServer::getIPAddress() const {
    if (_apMode) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

// ============================================================================
// PRIVATE METHODS - WIFI
// ============================================================================

bool PerimeterWebServer::connectWiFi() {
    #ifdef WIFI_SSID
    Serial.printf("[WebServer] Connecting to WiFi: %s\n", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
            Serial.println("[WebServer] WiFi connection timeout");
            return false;
        }
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.printf("[WebServer] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    _apMode = false;
    return true;
    #else
    Serial.println("[WebServer] No WiFi credentials configured");
    return false;
    #endif
}

void PerimeterWebServer::startAP() {
    Serial.printf("[WebServer] Starting AP: %s\n", AP_SSID);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.printf("[WebServer] AP started. IP: %s\n", WiFi.softAPIP().toString().c_str());
    _apMode = true;
}

// ============================================================================
// PRIVATE METHODS - ROUTES
// ============================================================================

void PerimeterWebServer::setupRoutes() {
    // Hovedside
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleRoot(request);
    });

    // API endpoints
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });

    _server.on("/api/start", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleStart(request);
    });

    _server.on("/api/stop", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleStop(request);
    });

    _server.on("/api/reset", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleReset(request);
    });

    // Også tillad GET for nem test
    _server.on("/api/start", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStart(request);
    });

    _server.on("/api/stop", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStop(request);
    });

    // Statiske filer fra LittleFS
    _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // 404 handler
    _server.onNotFound([this](AsyncWebServerRequest* request) {
        handleNotFound(request);
    });

    Serial.println("[WebServer] Routes configured");
}

void PerimeterWebServer::handleRoot(AsyncWebServerRequest* request) {
    // Hvis index.html findes i LittleFS, server den
    if (LittleFS.exists("/index.html")) {
        request->send(LittleFS, "/index.html", "text/html");
        return;
    }

    // Ellers server indbygget HTML
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Perimeter Wire Controller</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: #1a1a2e;
            color: #eee;
            min-height: 100vh;
            padding: 20px;
        }
        .container { max-width: 600px; margin: 0 auto; }
        h1 {
            text-align: center;
            margin-bottom: 30px;
            color: #00d4ff;
        }
        .status-card {
            background: #16213e;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .status-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid #0f3460;
        }
        .status-row:last-child { border-bottom: none; }
        .status-label { color: #888; }
        .status-value { font-weight: bold; }
        .status-running { color: #00ff88; }
        .status-off { color: #ff4444; }
        .status-error { color: #ff8800; }
        .controls {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-top: 20px;
        }
        button {
            padding: 15px 30px;
            font-size: 18px;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s;
        }
        .btn-start {
            background: #00d4ff;
            color: #000;
        }
        .btn-start:hover { background: #00a8cc; }
        .btn-stop {
            background: #ff4444;
            color: #fff;
        }
        .btn-stop:hover { background: #cc3333; }
        .btn-reset {
            grid-column: 1 / -1;
            background: #444;
            color: #fff;
        }
        .btn-reset:hover { background: #555; }
        .current-display {
            text-align: center;
            font-size: 48px;
            color: #00d4ff;
            margin: 20px 0;
        }
        .current-unit { font-size: 24px; color: #888; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Perimeter Wire Controller</h1>

        <div class="status-card">
            <div class="current-display">
                <span id="current">0</span>
                <span class="current-unit">mA</span>
            </div>
        </div>

        <div class="status-card">
            <div class="status-row">
                <span class="status-label">Status</span>
                <span class="status-value" id="state">OFF</span>
            </div>
            <div class="status-row">
                <span class="status-label">Runtime</span>
                <span class="status-value" id="runtime">0s</span>
            </div>
            <div class="status-row">
                <span class="status-label">Cycles</span>
                <span class="status-value" id="cycles">0</span>
            </div>
            <div class="status-row">
                <span class="status-label">IP Address</span>
                <span class="status-value" id="ip">-</span>
            </div>
        </div>

        <div class="controls">
            <button class="btn-start" onclick="startSignal()">START</button>
            <button class="btn-stop" onclick="stopSignal()">STOP</button>
            <button class="btn-reset" onclick="resetSignal()">RESET</button>
        </div>
    </div>

    <script>
        function updateStatus() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('state').textContent = data.state;
                    document.getElementById('state').className = 'status-value status-' + data.state.toLowerCase();
                    document.getElementById('current').textContent = data.current_mA.toFixed(0);
                    document.getElementById('runtime').textContent = formatTime(data.runtime_ms);
                    document.getElementById('cycles').textContent = data.cycles;
                    document.getElementById('ip').textContent = data.ip;
                })
                .catch(e => console.error('Status error:', e));
        }

        function formatTime(ms) {
            let s = Math.floor(ms / 1000);
            let m = Math.floor(s / 60);
            let h = Math.floor(m / 60);
            if (h > 0) return h + 'h ' + (m % 60) + 'm';
            if (m > 0) return m + 'm ' + (s % 60) + 's';
            return s + 's';
        }

        function startSignal() {
            fetch('/api/start', {method: 'POST'})
                .then(r => r.json())
                .then(data => { updateStatus(); })
                .catch(e => console.error('Start error:', e));
        }

        function stopSignal() {
            fetch('/api/stop', {method: 'POST'})
                .then(r => r.json())
                .then(data => { updateStatus(); })
                .catch(e => console.error('Stop error:', e));
        }

        function resetSignal() {
            fetch('/api/reset', {method: 'POST'})
                .then(r => r.json())
                .then(data => { updateStatus(); })
                .catch(e => console.error('Reset error:', e));
        }

        // Update status every second
        setInterval(updateStatus, 1000);
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", html);
    addCORSHeaders(response);
    request->send(response);
}

void PerimeterWebServer::handleStatus(AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", createStatusJSON());
    addCORSHeaders(response);
    request->send(response);
}

void PerimeterWebServer::handleStart(AsyncWebServerRequest* request) {
    if (_signalGen) {
        _signalGen->start();
    }

    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = "Signal started";
    doc["state"] = _signalGen ? _signalGen->getStateString() : "ERROR";

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse* resp = request->beginResponse(200, "application/json", response);
    addCORSHeaders(resp);
    request->send(resp);
}

void PerimeterWebServer::handleStop(AsyncWebServerRequest* request) {
    if (_signalGen) {
        _signalGen->stop();
    }

    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = "Signal stopped";
    doc["state"] = _signalGen ? _signalGen->getStateString() : "ERROR";

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse* resp = request->beginResponse(200, "application/json", response);
    addCORSHeaders(resp);
    request->send(resp);
}

void PerimeterWebServer::handleReset(AsyncWebServerRequest* request) {
    if (_signalGen) {
        _signalGen->reset();
    }

    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = "Signal generator reset";
    doc["state"] = _signalGen ? _signalGen->getStateString() : "ERROR";

    String response;
    serializeJson(doc, response);

    AsyncWebServerResponse* resp = request->beginResponse(200, "application/json", response);
    addCORSHeaders(resp);
    request->send(resp);
}

void PerimeterWebServer::handleNotFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not Found");
}

// ============================================================================
// PRIVATE METHODS - HELPERS
// ============================================================================

String PerimeterWebServer::createStatusJSON() {
    JsonDocument doc;

    doc["state"] = _signalGen ? _signalGen->getStateString() : "UNKNOWN";
    doc["running"] = _signalGen ? _signalGen->isRunning() : false;
    doc["current_mA"] = _signalGen ? _signalGen->getCurrentMA() : 0;
    doc["runtime_ms"] = _signalGen ? _signalGen->getRuntime() : 0;
    doc["cycles"] = _signalGen ? _signalGen->getCycleCount() : 0;
    doc["overcurrent"] = _signalGen ? _signalGen->isOvercurrent() : false;
    doc["ip"] = getIPAddress();
    doc["ap_mode"] = _apMode;
    doc["hostname"] = MDNS_HOSTNAME;

    String response;
    serializeJson(doc, response);
    return response;
}

void PerimeterWebServer::addCORSHeaders(AsyncWebServerResponse* response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}
