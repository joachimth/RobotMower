#include "WiFiManager.h"

WiFiManager::WiFiManager() {
    dnsServer = nullptr;
    apMode = false;
    hasCredentials = false;
    failedAttempts = 0;
    lastReconnectAttempt = 0;
}

bool WiFiManager::begin() {
    Logger::info("Starting WiFi Manager...");

    // Prøv at indlæse gemte credentials
    if (loadCredentials()) {
        Logger::info("Found stored credentials for: " + storedSSID);

        // Prøv at forbinde med gemte credentials
        if (connectWiFi(storedSSID, storedPassword)) {
            Logger::info("Connected to WiFi: " + storedSSID);
            Logger::info("IP Address: " + WiFi.localIP().toString());
            apMode = false;
            failedAttempts = 0;
            return true;
        } else {
            Logger::warning("Failed to connect with stored credentials");
            failedAttempts++;

            // Hvis for mange fejl, fallback til AP mode
            if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
                Logger::error("Too many failed attempts - starting AP mode");
                startCaptivePortal();
                return false;
            }
        }
    } else {
        Logger::info("No stored credentials - starting captive portal");
        startCaptivePortal();
        return false;
    }

    return false;
}

void WiFiManager::update() {
    // Hvis i AP mode, håndter DNS server
    if (apMode && dnsServer != nullptr) {
        dnsServer->processNextRequest();
        return;
    }

    // Tjek WiFi forbindelse periodisk
    unsigned long currentTime = millis();
    if (currentTime - lastReconnectAttempt > RECONNECT_INTERVAL) {
        lastReconnectAttempt = currentTime;

        if (WiFi.status() != WL_CONNECTED) {
            Logger::warning("WiFi disconnected - attempting reconnect...");
            failedAttempts++;

            if (hasCredentials) {
                if (connectWiFi(storedSSID, storedPassword)) {
                    Logger::info("Reconnected to WiFi");
                    failedAttempts = 0;
                } else if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
                    Logger::error("Too many reconnect failures - starting AP mode");
                    startCaptivePortal();
                }
            }
        } else {
            // Forbindelse OK - reset counter
            if (failedAttempts > 0) {
                Logger::info("WiFi connection restored");
                failedAttempts = 0;
            }
        }
    }
}

bool WiFiManager::saveCredentials(const String& ssid, const String& password) {
    Logger::info("Saving WiFi credentials to NVS...");

    preferences.begin("wifi", false); // Read/write mode

    bool success = true;
    success &= preferences.putString("ssid", ssid);
    success &= preferences.putString("password", password);

    preferences.end();

    if (success) {
        Logger::info("Credentials saved successfully");
        storedSSID = ssid;
        storedPassword = password;
        hasCredentials = true;
        return true;
    } else {
        Logger::error("Failed to save credentials");
        return false;
    }
}

void WiFiManager::clearCredentials() {
    Logger::info("Clearing stored WiFi credentials");

    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();

    storedSSID = "";
    storedPassword = "";
    hasCredentials = false;
}

bool WiFiManager::hasStoredCredentials() {
    return hasCredentials;
}

void WiFiManager::startCaptivePortal() {
    Logger::info("Starting Captive Portal...");

    // Stop eksisterende WiFi forbindelse
    WiFi.disconnect();
    delay(100);

    // Start Access Point
    WiFi.mode(WIFI_AP);
    String apSSID = String(CAPTIVE_PORTAL_SSID);
    bool success = WiFi.softAP(apSSID.c_str(), CAPTIVE_PORTAL_PASSWORD);

    if (success) {
        apMode = true;
        IPAddress IP = WiFi.softAPIP();
        Logger::info("AP started: " + apSSID);
        Logger::info("AP IP: " + IP.toString());
        Logger::info("Connect to '" + apSSID + "' and open http://" + IP.toString());

        // Start DNS server for captive portal
        setupCaptiveDNS();
    } else {
        Logger::error("Failed to start Access Point");
        apMode = false;
    }
}

void WiFiManager::stopCaptivePortal() {
    Logger::info("Stopping Captive Portal");

    if (dnsServer != nullptr) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
    }

    WiFi.softAPdisconnect(true);
    apMode = false;
}

bool WiFiManager::isAPMode() {
    return apMode;
}

bool WiFiManager::isConnected() {
    return !apMode && (WiFi.status() == WL_CONNECTED);
}

String WiFiManager::getIPAddress() {
    if (apMode) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}

String WiFiManager::getSSID() {
    if (apMode) {
        return String(CAPTIVE_PORTAL_SSID);
    } else {
        return WiFi.SSID();
    }
}

void WiFiManager::forceAPMode() {
    Logger::info("Forcing AP mode");
    failedAttempts = MAX_FAILED_ATTEMPTS;
    startCaptivePortal();
}

String WiFiManager::getCaptivePortalHTML() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang='da'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Robot Mower WiFi Setup</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 500px;
            width: 100%;
            padding: 40px;
            animation: slideIn 0.5s ease-out;
        }
        @keyframes slideIn {
            from { transform: translateY(-30px); opacity: 0; }
            to { transform: translateY(0); opacity: 1; }
        }
        .logo {
            text-align: center;
            margin-bottom: 30px;
        }
        .logo-icon {
            font-size: 60px;
            margin-bottom: 10px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 10px;
            font-size: 24px;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .form-group {
            margin-bottom: 25px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            color: #555;
            font-weight: 500;
            font-size: 14px;
        }
        input[type="text"],
        input[type="password"],
        select {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            font-size: 16px;
            transition: all 0.3s ease;
            background: #f8f9fa;
        }
        input:focus, select:focus {
            outline: none;
            border-color: #667eea;
            background: white;
        }
        .btn {
            width: 100%;
            padding: 15px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
            margin-top: 10px;
        }
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px rgba(102, 126, 234, 0.4);
        }
        .btn:active {
            transform: translateY(0);
        }
        .scan-btn {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            margin-bottom: 20px;
        }
        .info-box {
            background: #e8f5e9;
            border-left: 4px solid #4caf50;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
            font-size: 13px;
            color: #2e7d32;
        }
        .password-toggle {
            position: relative;
        }
        .toggle-icon {
            position: absolute;
            right: 15px;
            top: 50%;
            transform: translateY(-50%);
            cursor: pointer;
            user-select: none;
            font-size: 20px;
        }
        .loading {
            display: none;
            text-align: center;
            margin-top: 20px;
        }
        .spinner {
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <div class='container'>
        <div class='logo'>
            <div class='logo-icon'>🤖</div>
            <h1>Robot Mower</h1>
            <p class='subtitle'>WiFi Konfiguration</p>
        </div>

        <div class='info-box'>
            ℹ️ Første gang opsætning - indtast dine WiFi oplysninger for at forbinde robotten til dit netværk.
        </div>

        <form id='wifiForm' action='/wifi/save' method='POST'>
            <div class='form-group'>
                <label for='ssid'>WiFi Netværk (SSID)</label>
                <select id='ssid' name='ssid' required>
                    <option value=''>Vælg netværk eller indtast manuelt</option>
                </select>
            </div>

            <div class='form-group password-toggle'>
                <label for='password'>WiFi Adgangskode</label>
                <input type='password' id='password' name='password' required
                       placeholder='Din WiFi adgangskode'>
                <span class='toggle-icon' onclick='togglePassword()'>👁️</span>
            </div>

            <button type='button' class='btn scan-btn' onclick='scanNetworks()'>
                🔍 Scan efter netværk
            </button>

            <button type='submit' class='btn'>
                💾 Gem og forbind
            </button>
        </form>

        <div class='loading' id='loading'>
            <div class='spinner'></div>
            <p style='margin-top: 15px; color: #666;'>Forbinder til WiFi...</p>
        </div>
    </div>

    <script>
        function togglePassword() {
            const pwd = document.getElementById('password');
            pwd.type = pwd.type === 'password' ? 'text' : 'password';
        }

        async function scanNetworks() {
            const btn = event.target;
            btn.disabled = true;
            btn.textContent = '🔄 Scanner...';

            try {
                const response = await fetch('/wifi/scan');
                const networks = await response.json();

                const select = document.getElementById('ssid');
                select.innerHTML = '<option value="">Vælg netværk</option>';

                networks.forEach(net => {
                    const option = document.createElement('option');
                    option.value = net.ssid;
                    option.textContent = `${net.ssid} (${net.rssi} dBm) ${net.encryption}`;
                    select.appendChild(option);
                });

                btn.textContent = '✅ Scan komplet';
                setTimeout(() => {
                    btn.textContent = '🔍 Scan efter netværk';
                    btn.disabled = false;
                }, 2000);
            } catch (error) {
                alert('Kunne ikke scanne netværk: ' + error);
                btn.textContent = '🔍 Scan efter netværk';
                btn.disabled = false;
            }
        }

        document.getElementById('wifiForm').addEventListener('submit', async function(e) {
            e.preventDefault();

            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;

            if (!ssid || !password) {
                alert('Indtast venligst både SSID og password');
                return;
            }

            document.querySelector('.container').style.display = 'none';
            document.getElementById('loading').style.display = 'block';

            try {
                const formData = new FormData();
                formData.append('ssid', ssid);
                formData.append('password', password);

                const response = await fetch('/wifi/save', {
                    method: 'POST',
                    body: formData
                });

                if (response.ok) {
                    setTimeout(() => {
                        document.getElementById('loading').innerHTML = `
                            <div style="text-align: center;">
                                <h2 style="color: #4caf50; margin-bottom: 20px;">✅ Forbundet!</h2>
                                <p>Robotten forbinder nu til dit WiFi netværk.</p>
                                <p style="margin-top: 20px;">Find robottens nye IP adresse i din routers admin panel,<br>
                                eller åbn <a href="http://robot-mower.local">robot-mower.local</a></p>
                            </div>
                        `;
                    }, 5000);
                } else {
                    throw new Error('Fejl ved lagring af credentials');
                }
            } catch (error) {
                alert('Fejl: ' + error.message);
                location.reload();
            }
        });

        // Scan networks on page load
        window.addEventListener('load', () => {
            setTimeout(scanNetworks, 1000);
        });
    </script>
</body>
</html>
)rawliteral";

    return html;
}

bool WiFiManager::connectWiFi(const String& ssid, const String& password) {
    Logger::info("Attempting to connect to: " + ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttempt = millis();
    const unsigned long timeout = 15000; // 15 sekunder

    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Logger::info("WiFi connected successfully");
        return true;
    } else {
        Logger::error("WiFi connection failed");
        return false;
    }
}

bool WiFiManager::loadCredentials() {
    preferences.begin("wifi", true); // Read-only mode

    storedSSID = preferences.getString("ssid", "");
    storedPassword = preferences.getString("password", "");

    preferences.end();

    hasCredentials = (storedSSID.length() > 0);
    return hasCredentials;
}

void WiFiManager::setupCaptiveDNS() {
    if (dnsServer != nullptr) {
        delete dnsServer;
    }

    dnsServer = new DNSServer();

    // Redirect all DNS requests to AP IP
    dnsServer->start(53, "*", WiFi.softAPIP());

    Logger::info("Captive DNS started");
}
