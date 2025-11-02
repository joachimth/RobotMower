#include "Display.h"

Display::Display() {
    u8g2 = nullptr;
    lastUpdate = 0;
    needsUpdate = true;
    initialized = false;
}

bool Display::begin() {
    #if !ENABLE_DISPLAY
    Serial.println("[Display] Display disabled in config");
    return false;
    #endif

    // Aktiver Vext (strømforsyning til OLED display)
    // På Heltec WiFi Kit 32 V3, skal Vext være LOW for at aktivere
    pinMode(DISPLAY_VEXT, OUTPUT);
    digitalWrite(DISPLAY_VEXT, LOW); // LOW = ON for Vext
    delay(100); // Giv display tid til at starte

    // Initialiser U8g2 display (Heltec OLED)
    u8g2 = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE,
                                                    /* clock=*/ DISPLAY_SCL,
                                                    /* data=*/ DISPLAY_SDA);

    if (u8g2 == nullptr) {
        Serial.println("[Display] Failed to allocate display object");
        return false;
    }

    u8g2->begin();
    u8g2->enableUTF8Print();

    initialized = true;

    Serial.println("[Display] Vext On - Display power enabled");
    Serial.println("[Display] Initialized successfully");

    // Vis splash screen
    showSplash();

    return true;
}

void Display::update() {
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
    if (!initialized) return;
    u8g2->clearBuffer();
    u8g2->sendBuffer();
}

void Display::showSplash() {
    if (!initialized) return;

    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_ncenB14_tr);
    u8g2->drawStr(15, 30, "ROBOT");
    u8g2->drawStr(8, 50, "MOWER");

    u8g2->setFont(u8g2_font_6x10_tr);
    u8g2->drawStr(25, 62, "v1.0 Ready");

    u8g2->sendBuffer();
    needsUpdate = true;
}

void Display::showStatus(String state, float battery, float heading) {
    if (!initialized) return;

    u8g2->clearBuffer();

    // Titel
    u8g2->setFont(u8g2_font_ncenB08_tr);
    u8g2->drawStr(30, 10, "Robot Mower");

    // Linje separator
    u8g2->drawLine(0, 12, 128, 12);

    // State
    u8g2->setFont(u8g2_font_6x10_tr);
    u8g2->drawStr(0, 24, "State:");
    u8g2->drawStr(40, 24, state.c_str());

    // Batteri
    u8g2->drawStr(0, 36, "Battery:");
    char batteryStr[10];
    sprintf(batteryStr, "%.1fV", battery);
    u8g2->drawStr(50, 36, batteryStr);
    drawBatteryIcon(100, 28, battery);

    // Heading
    u8g2->drawStr(0, 48, "Heading:");
    char headingStr[10];
    sprintf(headingStr, "%.0f", heading);
    u8g2->drawStr(50, 48, headingStr);
    u8g2->drawStr(80, 48, "deg");

    // Kompas (lille)
    drawCompassRose(110, 50, heading, 10);

    u8g2->sendBuffer();
    needsUpdate = true;
}

void Display::showError(String message) {
    if (!initialized) return;

    u8g2->clearBuffer();

    // Error titel
    u8g2->setFont(u8g2_font_ncenB10_tr);
    u8g2->drawStr(35, 15, "ERROR!");

    // Error ikon (!!)
    u8g2->setFont(u8g2_font_ncenB14_tr);
    u8g2->drawStr(56, 35, "!!");

    // Error besked
    u8g2->setFont(u8g2_font_6x10_tr);

    // Word wrap hvis beskeden er lang
    if (message.length() > 20) {
        u8g2->drawStr(5, 50, message.substring(0, 20).c_str());
        u8g2->drawStr(5, 62, message.substring(20).c_str());
    } else {
        u8g2->drawStr(5, 55, message.c_str());
    }

    u8g2->sendBuffer();
    needsUpdate = true;
}

void Display::showSensorData(float left, float middle, float right) {
    if (!initialized) return;

    u8g2->clearBuffer();

    // Titel
    u8g2->setFont(u8g2_font_ncenB08_tr);
    u8g2->drawStr(30, 10, "Sensors (cm)");

    // Linje
    u8g2->drawLine(0, 12, 128, 12);

    u8g2->setFont(u8g2_font_6x10_tr);

    // Venstre sensor
    u8g2->drawStr(0, 28, "Left:");
    char leftStr[10];
    sprintf(leftStr, "%.0f", left);
    u8g2->drawStr(45, 28, leftStr);

    // Midter sensor
    u8g2->drawStr(0, 42, "Middle:");
    char middleStr[10];
    sprintf(middleStr, "%.0f", middle);
    u8g2->drawStr(45, 42, middleStr);

    // Højre sensor
    u8g2->drawStr(0, 56, "Right:");
    char rightStr[10];
    sprintf(rightStr, "%.0f", right);
    u8g2->drawStr(45, 56, rightStr);

    // Visuel repræsentation (bars)
    int leftBar = map(left, 0, 200, 0, 35);
    int middleBar = map(middle, 0, 200, 0, 35);
    int rightBar = map(right, 0, 200, 0, 35);

    u8g2->drawBox(85, 27 - leftBar/2, 10, leftBar);
    u8g2->drawBox(100, 41 - middleBar/2, 10, middleBar);
    u8g2->drawBox(115, 55 - rightBar/2, 10, rightBar);

    u8g2->sendBuffer();
    needsUpdate = true;
}

void Display::showCalibration(int progress) {
    if (!initialized) return;

    u8g2->clearBuffer();

    u8g2->setFont(u8g2_font_ncenB10_tr);
    u8g2->drawStr(20, 20, "Calibrating");

    u8g2->setFont(u8g2_font_6x10_tr);
    u8g2->drawStr(15, 35, "Keep robot still!");

    // Progress bar
    drawProgressBar(14, 45, 100, 10, progress);

    // Percentage
    char progressStr[10];
    sprintf(progressStr, "%d%%", progress);
    u8g2->setFont(u8g2_font_6x10_tr);
    u8g2->drawStr(55, 62, progressStr);

    u8g2->sendBuffer();
    needsUpdate = true;
}

void Display::showWiFiInfo(String ssid, String ip) {
    if (!initialized) return;

    u8g2->clearBuffer();

    u8g2->setFont(u8g2_font_ncenB08_tr);
    u8g2->drawStr(30, 12, "WiFi Connected");

    u8g2->drawLine(0, 14, 128, 14);

    u8g2->setFont(u8g2_font_6x10_tr);
    u8g2->drawStr(0, 28, "SSID:");
    u8g2->drawStr(0, 40, ssid.c_str());

    u8g2->drawStr(0, 54, "IP:");
    u8g2->drawStr(0, 64, ip.c_str());

    u8g2->sendBuffer();
    needsUpdate = true;
}

void Display::forceUpdate() {
    lastUpdate = 0;
    needsUpdate = true;
    update();
}

void Display::drawBatteryIcon(int x, int y, float voltage) {
    // Beregn fill level
    int percentage = getBatteryPercentage(voltage);
    int fillHeight = map(percentage, 0, 100, 0, 10);

    // Batteri ramme
    u8g2->drawFrame(x, y, 14, 12);

    // Batteri top
    u8g2->drawBox(x + 4, y - 2, 6, 2);

    // Fill level
    if (fillHeight > 0) {
        u8g2->drawBox(x + 2, y + 12 - fillHeight - 2, 10, fillHeight);
    }
}

void Display::drawCompassRose(int x, int y, float heading, int radius) {
    // Beregn nord pil position baseret på heading
    float radians = (heading - 90) * PI / 180.0;
    int x2 = x + radius * cos(radians);
    int y2 = y + radius * sin(radians);

    // Cirkel
    u8g2->drawCircle(x, y, radius);

    // Nord pil
    u8g2->drawLine(x, y, x2, y2);

    // Pil hoved
    u8g2->drawPixel(x2, y2);
}

void Display::drawProgressBar(int x, int y, int width, int height, int progress) {
    // Ramme
    u8g2->drawFrame(x, y, width, height);

    // Fill
    int fillWidth = map(progress, 0, 100, 0, width - 4);
    if (fillWidth > 0) {
        u8g2->drawBox(x + 2, y + 2, fillWidth, height - 4);
    }
}

int Display::getBatteryPercentage(float voltage) {
    if (voltage >= BATTERY_MAX_VOLTAGE) return 100;
    if (voltage <= BATTERY_MIN_VOLTAGE) return 0;

    return map(voltage * 10, BATTERY_MIN_VOLTAGE * 10, BATTERY_MAX_VOLTAGE * 10, 0, 100);
}
