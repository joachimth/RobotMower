#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config/Config.h"

/**
 * Display klasse - Håndterer OLED display (128x64)
 *
 * Denne klasse viser robot status på det indbyggede OLED display
 * på Heltec WiFi Kit 32 V3.
 */
class Display {
public:
    /**
     * Constructor
     */
    Display();

    /**
     * Initialiserer display
     * @return true hvis succesfuld, false ved fejl
     */
    bool begin();

    /**
     * Opdaterer display (non-blocking)
     * Kalder denne regelmæssigt i loop()
     */
    void update();

    /**
     * Rydder display
     */
    void clear();

    /**
     * Viser splash screen ved opstart
     */
    void showSplash();

    /**
     * Viser hovedstatus screen
     * @param state Robot tilstand (tekst)
     * @param battery Batteri spænding
     * @param heading Retning i grader
     */
    void showStatus(String state, float battery, float heading);

    /**
     * Viser fejlbesked
     * @param message Fejlbesked at vise
     */
    void showError(String message);

    /**
     * Viser sensor data
     * @param left Venstre sensor afstand
     * @param middle Midter sensor afstand
     * @param right Højre sensor afstand
     */
    void showSensorData(float left, float middle, float right);

    /**
     * Viser kalibrerings progress
     * @param progress Progress i procent (0-100)
     */
    void showCalibration(int progress);

    /**
     * Viser WiFi forbindelses info
     * @param ssid WiFi SSID
     * @param ip IP adresse
     */
    void showWiFiInfo(String ssid, String ip);

    /**
     * Tvinger display opdatering (ignorerer interval)
     */
    void forceUpdate();

private:
    /**
     * Tegner batteri ikon
     * @param x X position
     * @param y Y position
     * @param voltage Batteri spænding
     */
    void drawBatteryIcon(int x, int y, float voltage);

    /**
     * Tegner kompas rose
     * @param x Center X position
     * @param y Center Y position
     * @param heading Retning i grader
     * @param radius Radius af kompas
     */
    void drawCompassRose(int x, int y, float heading, int radius);

    /**
     * Tegner progress bar
     * @param x X position
     * @param y Y position
     * @param width Bredde
     * @param height Højde
     * @param progress Progress i procent (0-100)
     */
    void drawProgressBar(int x, int y, int width, int height, int progress);

    /**
     * Beregner batteri procent
     * @param voltage Spænding
     * @return Procent (0-100)
     */
    int getBatteryPercentage(float voltage);

    // U8g2 display objekt (Heltec indbygget OLED)
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2;

    // Timing
    unsigned long lastUpdate;
    bool needsUpdate;

    // Display tilstand
    bool initialized;
};

#endif // DISPLAY_H
