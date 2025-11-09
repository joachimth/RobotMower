#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "../config/Config.h"
#include "Logger.h"

/**
 * UpdateManager klasse - Håndterer auto-update fra GitHub
 *
 * Features:
 * - Tjekker GitHub releases for ny firmware
 * - Sammenligner versioner
 * - Downloader og installerer firmware automatisk
 * - Progress callbacks
 */
class UpdateManager {
public:
    /**
     * Update status
     */
    enum UpdateStatus {
        UPDATE_IDLE,
        UPDATE_CHECKING,
        UPDATE_AVAILABLE,
        UPDATE_NO_UPDATE,
        UPDATE_DOWNLOADING,
        UPDATE_INSTALLING,
        UPDATE_SUCCESS,
        UPDATE_FAILED
    };

    /**
     * Constructor
     */
    UpdateManager();

    /**
     * Initialiserer Update Manager
     */
    bool begin();

    /**
     * Tjek for nye firmware updates
     * @return true hvis ny version tilgængelig
     */
    bool checkForUpdate();

    /**
     * Download og installer update
     * @return true hvis succesfuld
     */
    bool performUpdate();

    /**
     * Hent nuværende firmware version
     * @return Version string
     */
    String getCurrentVersion();

    /**
     * Hent seneste tilgængelige version
     * @return Version string
     */
    String getLatestVersion();

    /**
     * Hent update status
     * @return UpdateStatus
     */
    UpdateStatus getStatus();

    /**
     * Hent download progress (0-100%)
     * @return Progress i procent
     */
    int getProgress();

    /**
     * Hent fejlbesked hvis update fejlede
     * @return Fejlbesked
     */
    String getError();

    /**
     * Sammenlign versioner
     * @param v1 Version 1
     * @param v2 Version 2
     * @return >0 hvis v1 > v2, <0 hvis v1 < v2, 0 hvis ens
     */
    static int compareVersions(const String& v1, const String& v2);

private:
    /**
     * Hent seneste release info fra GitHub API
     * @return true hvis succesfuld
     */
    bool fetchLatestRelease();

    /**
     * Download firmware fra URL
     * @param url Download URL
     * @return true hvis succesfuld
     */
    bool downloadFirmware(const String& url);

    // State
    UpdateStatus status;
    String currentVersion;
    String latestVersion;
    String downloadUrl;
    String errorMessage;
    int downloadProgress;

    // GitHub API endpoints
    String getGitHubAPIUrl();
};

#endif // UPDATEMANAGER_H
