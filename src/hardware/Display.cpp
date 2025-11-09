#include "Display.h"

Display::Display() {
    #if ENABLE_DISPLAY
    u8g2 = nullptr;
    #endif
    lastUpdate = 0;
    needsUpdate = true;
    initialized = false;
}

bool Display::begin() {
    #if !ENABLE_DISPLAY
    Serial.println("[Display] Display disabled in config (ESP32-WROOM-32U has no built-in display)");
    initialized = false;
    return false;
    #else
    // Display functionality only available if ENABLE_DISPLAY is true
    // For ESP32-WROOM-32U, display is disabled by default
    // If you want to add an external I2C OLED display:
    // 1. Set ENABLE_DISPLAY = true in Config.h
    // 2. Define DISPLAY_SDA and DISPLAY_SCL pins
    // 3. Connect display to I2C bus (shares with IMU)

    Serial.println("[Display] Display support not configured for ESP32-WROOM-32U");
    initialized = false;
    return false;
    #endif
}

void Display::update() {
    #if !ENABLE_DISPLAY
    return;
    #endif

    if (!initialized || !needsUpdate) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastUpdate < DISPLAY_UPDATE_INTERVAL) {
        return; // Ikke tid endnu
    }

    lastUpdate = currentTime;
    needsUpdate = false;
}

void Display::clear() {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::showSplash() {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::showStatus(String state, float battery, float heading) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::showError(String message) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::showSensorData(float left, float middle, float right) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::showCalibration(int progress) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::showWiFiInfo(String ssid, String ip) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::forceUpdate() {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::drawBatteryIcon(int x, int y, float voltage) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::drawCompassRose(int x, int y, float heading, int radius) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

void Display::drawProgressBar(int x, int y, int width, int height, int progress) {
    #if !ENABLE_DISPLAY
    return;
    #endif
}

int Display::getBatteryPercentage(float voltage) {
    #if !ENABLE_DISPLAY
    return 0;
    #endif

    if (voltage >= BATTERY_MAX_VOLTAGE) return 100;
    if (voltage <= BATTERY_MIN_VOLTAGE) return 0;

    return map(voltage * 10, BATTERY_MIN_VOLTAGE * 10, BATTERY_MAX_VOLTAGE * 10, 0, 100);
}
