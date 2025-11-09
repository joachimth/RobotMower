#include "UpdateManager.h"

UpdateManager::UpdateManager() {
    status = UPDATE_IDLE;
    currentVersion = String(CURRENT_FIRMWARE_VERSION);
    latestVersion = "";
    downloadUrl = "";
    errorMessage = "";
    downloadProgress = 0;
}

bool UpdateManager::begin() {
    Logger::info("Update Manager initialized");
    Logger::info("Current firmware version: " + currentVersion);
    return true;
}

bool UpdateManager::checkForUpdate() {
    Logger::info("Checking for firmware updates...");
    status = UPDATE_CHECKING;

    if (!fetchLatestRelease()) {
        status = UPDATE_FAILED;
        return false;
    }

    // Sammenlign versioner
    int comparison = compareVersions(latestVersion, currentVersion);

    if (comparison > 0) {
        Logger::info("New version available: " + latestVersion);
        status = UPDATE_AVAILABLE;
        return true;
    } else {
        Logger::info("No update available (current: " + currentVersion + ", latest: " + latestVersion + ")");
        status = UPDATE_NO_UPDATE;
        return false;
    }
}

bool UpdateManager::performUpdate() {
    if (status != UPDATE_AVAILABLE || downloadUrl.length() == 0) {
        errorMessage = "No update available";
        return false;
    }

    Logger::info("Starting firmware update...");
    Logger::info("Downloading from: " + downloadUrl);

    status = UPDATE_DOWNLOADING;

    if (!downloadFirmware(downloadUrl)) {
        status = UPDATE_FAILED;
        return false;
    }

    status = UPDATE_SUCCESS;
    Logger::info("Firmware update completed successfully!");
    Logger::info("Rebooting in 3 seconds...");

    delay(3000);
    ESP.restart();

    return true;
}

String UpdateManager::getCurrentVersion() {
    return currentVersion;
}

String UpdateManager::getLatestVersion() {
    return latestVersion;
}

UpdateManager::UpdateStatus UpdateManager::getStatus() {
    return status;
}

int UpdateManager::getProgress() {
    return downloadProgress;
}

String UpdateManager::getError() {
    return errorMessage;
}

int UpdateManager::compareVersions(const String& v1, const String& v2) {
    // Parse version strings (format: "1.2.3" eller "v1.2.3")
    String ver1 = v1;
    String ver2 = v2;

    // Fjern 'v' prefix hvis tilstede
    if (ver1.startsWith("v")) ver1 = ver1.substring(1);
    if (ver2.startsWith("v")) ver2 = ver2.substring(1);

    int major1 = 0, minor1 = 0, patch1 = 0;
    int major2 = 0, minor2 = 0, patch2 = 0;

    // Parse v1
    sscanf(ver1.c_str(), "%d.%d.%d", &major1, &minor1, &patch1);

    // Parse v2
    sscanf(ver2.c_str(), "%d.%d.%d", &major2, &minor2, &patch2);

    // Sammenlign
    if (major1 != major2) return major1 - major2;
    if (minor1 != minor2) return minor1 - minor2;
    return patch1 - patch2;
}

bool UpdateManager::fetchLatestRelease() {
    HTTPClient http;
    String apiUrl = getGitHubAPIUrl();

    Logger::info("Fetching: " + apiUrl);

    http.begin(apiUrl);
    http.addHeader("User-Agent", "RobotMower-ESP32");
    http.addHeader("Accept", "application/vnd.github.v3+json");

    int httpCode = http.GET();

    if (httpCode != 200) {
        errorMessage = "GitHub API error: " + String(httpCode);
        Logger::error(errorMessage);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    // Parse JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        errorMessage = "JSON parse error: " + String(error.c_str());
        Logger::error(errorMessage);
        return false;
    }

    // Hent version tag
    latestVersion = doc["tag_name"].as<String>();

    // Find .bin asset
    JsonArray assets = doc["assets"].as<JsonArray>();
    bool foundBin = false;

    for (JsonObject asset : assets) {
        String name = asset["name"].as<String>();
        if (name.endsWith(".bin")) {
            downloadUrl = asset["browser_download_url"].as<String>();
            foundBin = true;
            Logger::info("Found firmware: " + name);
            break;
        }
    }

    if (!foundBin) {
        errorMessage = "No .bin file found in release";
        Logger::error(errorMessage);
        return false;
    }

    return true;
}

bool UpdateManager::downloadFirmware(const String& url) {
    HTTPClient http;

    http.begin(url);
    http.addHeader("User-Agent", "RobotMower-ESP32");

    int httpCode = http.GET();

    if (httpCode != 200) {
        errorMessage = "Download failed: HTTP " + String(httpCode);
        Logger::error(errorMessage);
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    Logger::info("Firmware size: " + String(contentLength) + " bytes");

    // Start OTA update
    if (!Update.begin(contentLength)) {
        errorMessage = "Not enough space for update";
        Logger::error(errorMessage);
        http.end();
        return false;
    }

    status = UPDATE_INSTALLING;

    // Download og skriv til flash
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[128];

    while (http.connected() && (written < contentLength)) {
        size_t available = stream->available();
        if (available) {
            int bytesToRead = ((available > sizeof(buffer)) ? sizeof(buffer) : available);
            int bytesRead = stream->readBytes(buffer, bytesToRead);

            if (Update.write(buffer, bytesRead) != bytesRead) {
                errorMessage = "Write error during update";
                Logger::error(errorMessage);
                Update.abort();
                http.end();
                return false;
            }

            written += bytesRead;

            // Opdater progress
            downloadProgress = (written * 100) / contentLength;

            // Log progress hver 10%
            static int lastProgress = 0;
            if (downloadProgress >= lastProgress + 10) {
                Logger::info("Update progress: " + String(downloadProgress) + "%");
                lastProgress = downloadProgress;
            }
        }
        delay(1);
    }

    http.end();

    // Afslut update
    if (Update.end()) {
        if (Update.isFinished()) {
            Logger::info("Update successfully completed!");
            return true;
        } else {
            errorMessage = "Update not finished";
            Logger::error(errorMessage);
            return false;
        }
    } else {
        errorMessage = "Update error: " + String(Update.getError());
        Logger::error(errorMessage);
        return false;
    }
}

String UpdateManager::getGitHubAPIUrl() {
    // GitHub API endpoint for latest release
    return "https://api.github.com/repos/" +
           String(GITHUB_REPO_OWNER) + "/" +
           String(GITHUB_REPO_NAME) + "/releases/latest";
}
