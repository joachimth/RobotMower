#include "PerimeterClient.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

PerimeterClient::PerimeterClient()
    : _senderIP(PERIMETER_SENDER_IP)
    , _senderPort(PERIMETER_SENDER_PORT)
    , _initialized(false)
    , _connected(false)
    , _senderRunning(false)
    , _senderState("UNKNOWN")
    , _senderCurrent(0)
    , _senderRuntime(0)
    , _lastError("")
{
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

bool PerimeterClient::begin(const char* ip, int port) {
    _senderIP = ip;
    _senderPort = port;
    _initialized = true;

    Serial.println("[PerimeterClient] Initialized");
    Serial.printf("[PerimeterClient] Sender: http://%s:%d\n", _senderIP.c_str(), _senderPort);

    // Prøv at hente status for at verificere forbindelse
    if (updateStatus()) {
        Serial.println("[PerimeterClient] Connection verified");
        return true;
    } else {
        Serial.println("[PerimeterClient] WARNING: Could not connect to sender");
        Serial.println("[PerimeterClient] Check that sender is running and IP is correct");
        return true;  // Returner true alligevel - vi prøver igen senere
    }
}

bool PerimeterClient::startSignal() {
    if (!_initialized) {
        _lastError = "Not initialized";
        return false;
    }

    Serial.println("[PerimeterClient] Starting perimeter signal...");

    String response;
    int httpCode = httpPost("/api/start", response);

    if (httpCode == 200) {
        Serial.println("[PerimeterClient] Signal started");
        _connected = true;
        updateStatus();
        return true;
    } else {
        _lastError = "HTTP error: " + String(httpCode);
        Serial.printf("[PerimeterClient] Failed to start signal: %s\n", _lastError.c_str());
        return false;
    }
}

bool PerimeterClient::stopSignal() {
    if (!_initialized) {
        _lastError = "Not initialized";
        return false;
    }

    Serial.println("[PerimeterClient] Stopping perimeter signal...");

    String response;
    int httpCode = httpPost("/api/stop", response);

    if (httpCode == 200) {
        Serial.println("[PerimeterClient] Signal stopped");
        _connected = true;
        updateStatus();
        return true;
    } else {
        _lastError = "HTTP error: " + String(httpCode);
        Serial.printf("[PerimeterClient] Failed to stop signal: %s\n", _lastError.c_str());
        return false;
    }
}

bool PerimeterClient::resetSender() {
    if (!_initialized) {
        _lastError = "Not initialized";
        return false;
    }

    Serial.println("[PerimeterClient] Resetting perimeter sender...");

    String response;
    int httpCode = httpPost("/api/reset", response);

    if (httpCode == 200) {
        Serial.println("[PerimeterClient] Sender reset");
        _connected = true;
        updateStatus();
        return true;
    } else {
        _lastError = "HTTP error: " + String(httpCode);
        Serial.printf("[PerimeterClient] Failed to reset sender: %s\n", _lastError.c_str());
        return false;
    }
}

bool PerimeterClient::updateStatus() {
    if (!_initialized) {
        return false;
    }

    String response;
    int httpCode = httpGet("/api/status", response);

    if (httpCode == 200) {
        _connected = true;
        return parseStatusResponse(response);
    } else {
        _connected = false;
        _senderState = "DISCONNECTED";
        _lastError = "HTTP error: " + String(httpCode);
        return false;
    }
}

void PerimeterClient::setSenderIP(const char* ip) {
    _senderIP = ip;
    Serial.printf("[PerimeterClient] Sender IP changed to: %s\n", _senderIP.c_str());
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

int PerimeterClient::httpGet(const char* endpoint, String& response) {
    HTTPClient http;
    String url = buildURL(endpoint);

    http.begin(url);
    http.setTimeout(PERIMETER_API_TIMEOUT);

    int httpCode = http.GET();

    if (httpCode > 0) {
        response = http.getString();
    } else {
        response = "";
        _lastError = http.errorToString(httpCode);
    }

    http.end();
    return httpCode;
}

int PerimeterClient::httpPost(const char* endpoint, String& response) {
    HTTPClient http;
    String url = buildURL(endpoint);

    http.begin(url);
    http.setTimeout(PERIMETER_API_TIMEOUT);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST("");  // Tom body

    if (httpCode > 0) {
        response = http.getString();
    } else {
        response = "";
        _lastError = http.errorToString(httpCode);
    }

    http.end();
    return httpCode;
}

bool PerimeterClient::parseStatusResponse(const String& json) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        _lastError = "JSON parse error: " + String(error.c_str());
        Serial.printf("[PerimeterClient] %s\n", _lastError.c_str());
        return false;
    }

    // Parser felter
    _senderState = doc["state"].as<String>();
    _senderRunning = doc["running"].as<bool>();
    _senderCurrent = doc["current_mA"].as<float>();
    _senderRuntime = doc["runtime_ms"].as<unsigned long>();

    return true;
}

String PerimeterClient::buildURL(const char* endpoint) {
    return "http://" + _senderIP + ":" + String(_senderPort) + endpoint;
}
