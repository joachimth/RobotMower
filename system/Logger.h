#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "../config/Config.h"

/**
 * LogLevel enum - Definerer log niveauer
 */
enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

/**
 * Logger klasse - Håndterer logging til Serial og web clients
 *
 * Denne klasse logger beskeder til Serial Monitor og gemmer dem
 * i en cirkulær buffer til web interface.
 */
class Logger {
public:
    /**
     * Initialiserer logger
     */
    static void begin();

    /**
     * Logger debug besked
     * @param message Besked at logge
     */
    static void debug(String message);

    /**
     * Logger info besked
     * @param message Besked at logge
     */
    static void info(String message);

    /**
     * Logger advarsel
     * @param message Besked at logge
     */
    static void warning(String message);

    /**
     * Logger fejl
     * @param message Besked at logge
     */
    static void error(String message);

    /**
     * Logger sensor data (formateret)
     * @param left Venstre sensor
     * @param middle Midter sensor
     * @param right Højre sensor
     */
    static void logSensorData(float left, float middle, float right);

    /**
     * Logger motor data (formateret)
     * @param left Venstre motor hastighed
     * @param right Højre motor hastighed
     */
    static void logMotorData(int left, int right);

    /**
     * Hent seneste log entries
     * @param count Antal entries at returnere
     * @return String med log entries (JSON format)
     */
    static String getRecentLogs(int count);

    /**
     * Hent alle log entries
     * @return String med alle log entries (JSON format)
     */
    static String getAllLogs();

    /**
     * Ryd log buffer
     */
    static void clearLogs();

    /**
     * Hent antal log entries i buffer
     * @return Antal entries
     */
    static int getLogCount();

private:
    /**
     * Intern log funktion
     * @param level Log niveau
     * @param message Besked
     */
    static void log(LogLevel level, String message);

    /**
     * Formatter log entry
     * @param level Log niveau
     * @param message Besked
     * @return Formateret log entry
     */
    static String formatLogEntry(LogLevel level, String message);

    /**
     * Konverterer log level til string
     * @param level Log niveau
     * @return String repræsentation
     */
    static String levelToString(LogLevel level);

    /**
     * Tilføjer log entry til cirkulær buffer
     * @param entry Log entry at tilføje
     */
    static void addToBuffer(String entry);

    // Cirkulær buffer til log entries
    static String logBuffer[LOG_BUFFER_SIZE];
    static int logIndex;
    static int logCount;

    // Initialization flag
    static bool initialized;
};

#endif // LOGGER_H
