#include "Logger.h"

// Static member initialization
String Logger::logBuffer[LOG_BUFFER_SIZE];
int Logger::logIndex = 0;
int Logger::logCount = 0;
bool Logger::initialized = false;

void Logger::begin() {
    // Ryd buffer
    for (int i = 0; i < LOG_BUFFER_SIZE; i++) {
        logBuffer[i] = "";
    }

    logIndex = 0;
    logCount = 0;
    initialized = true;

    info("Logger initialized");
}

void Logger::debug(String message) {
    #if DEBUG_MODE
    log(LOG_DEBUG, message);
    #endif
}

void Logger::info(String message) {
    log(LOG_INFO, message);
}

void Logger::warning(String message) {
    log(LOG_WARNING, message);
}

void Logger::error(String message) {
    log(LOG_ERROR, message);
}

void Logger::logSensorData(float left, float middle, float right) {
    char buffer[100];
    sprintf(buffer, "Sensors - L: %.1fcm, M: %.1fcm, R: %.1fcm", left, middle, right);
    debug(String(buffer));
}

void Logger::logMotorData(int left, int right) {
    char buffer[80];
    sprintf(buffer, "Motors - Left: %d, Right: %d", left, right);
    debug(String(buffer));
}

String Logger::getRecentLogs(int count) {
    if (!initialized) {
        return "[]";
    }

    if (count > logCount) {
        count = logCount;
    }
    if (count > LOG_BUFFER_SIZE) {
        count = LOG_BUFFER_SIZE;
    }

    String json = "[";

    // Start fra seneste og gå baglæns
    for (int i = 0; i < count; i++) {
        int index = (logIndex - 1 - i + LOG_BUFFER_SIZE) % LOG_BUFFER_SIZE;

        if (logBuffer[index] != "") {
            if (i > 0) {
                json += ",";
            }
            json += logBuffer[index];
        }
    }

    json += "]";
    return json;
}

String Logger::getAllLogs() {
    return getRecentLogs(logCount);
}

void Logger::clearLogs() {
    for (int i = 0; i < LOG_BUFFER_SIZE; i++) {
        logBuffer[i] = "";
    }
    logIndex = 0;
    logCount = 0;

    info("Logs cleared");
}

int Logger::getLogCount() {
    return logCount;
}

void Logger::log(LogLevel level, String message) {
    // Formatter log entry
    String entry = formatLogEntry(level, message);

    // Output til Serial hvis aktiveret
    #if LOG_TO_SERIAL
    Serial.println(entry);
    #endif

    // Tilføj til buffer hvis initialiseret
    if (initialized) {
        // Konverter til JSON format for buffer
        String jsonEntry = "{";
        jsonEntry += "\"level\":\"" + levelToString(level) + "\",";
        jsonEntry += "\"message\":\"" + message + "\",";
        jsonEntry += "\"timestamp\":" + String(millis());
        jsonEntry += "}";

        addToBuffer(jsonEntry);
    }
}

String Logger::formatLogEntry(LogLevel level, String message) {
    // Format: [timestamp] [LEVEL] message
    String formatted = "[";
    formatted += String(millis());
    formatted += "] [";
    formatted += levelToString(level);
    formatted += "] ";
    formatted += message;

    return formatted;
}

String Logger::levelToString(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR:   return "ERROR";
        default:          return "UNKNOWN";
    }
}

void Logger::addToBuffer(String entry) {
    // Tilføj til cirkulær buffer
    logBuffer[logIndex] = entry;

    // Opdater index
    logIndex = (logIndex + 1) % LOG_BUFFER_SIZE;

    // Opdater count (max LOG_BUFFER_SIZE)
    if (logCount < LOG_BUFFER_SIZE) {
        logCount++;
    }
}
