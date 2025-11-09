# 🤖 Robot Plæneklipper - ESP32-WROOM-32U

Autonome plæneklipper bygget på ESP32-WROOM-32U med systematisk klipningsmønster, ultralyd baseret forhindring undgåelse og web kontrol interface.

![Project Status](https://img.shields.io/badge/status-active-success)
![Version](https://img.shields.io/badge/version-1.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP32--WROOM--32U-orange)

## ✨ Features

- **Systematisk Klipning**: Parallelt række-mønster for effektiv plæneklipning
- **Forhindring Undgåelse**: 3x ultralyd sensorer til obstacle detection
- **IMU Navigation**: MPU-6050/9250 til præcis retningsbestemmelse
- **Web Interface**: Komplet responsive web interface med manuel kontrol
- **WebSocket Telemetri**: Live sensor data og status updates
- **Strømovervågning**: Real-time strømmåling fra BTS7960 current sense
- **Manuel Kontrol**: Direkte motor kontrol via web interface
- **Batteri Overvågning**: Automatisk low-battery håndtering
- **State Machine**: Robust state management system
- **Logging System**: Comprehensive debug logging
- **Optimeret Pin Layout**: Komponenter grupperet på tilstødende pins

## 📦 Hardware Krav

### Hovedkomponenter
- **ESP32-WROOM-32U Development Board (38-Pin)** - Hovedcontroller
- **2x Double BTS7960 43A H-Bridge Motor Driver** - High-power DC motor kontrol
- **2x DC Gear Motors** - Drive motors (18V)
- **3x HC-SR04 Ultralyd Sensorer** - Forhindring detection
- **MPU-6050 eller MPU-9250** - IMU (accelerometer + gyroscope)
- **Relay Modul** - Klippermotor kontrol
- **5S LiPo Batteri** (18.5V) - Motor power supply
- **3S LiPo Batteri** (11.1V) - Control electronics power
- **Voltage Divider** (10kΩ + 2.2kΩ) - Batteri monitoring

### Valgfrie Komponenter
- 0.96" I2C OLED Display (SSD1306) - Ekstern status display
- Perimeter wire sensor
- GPS modul
- Kamera modul
- Ladestation beacon

Se [HARDWARE.md](HARDWARE.md) for detaljeret hardware liste og specifikationer.

## 🔌 Pin Forbindelser

Se [PINOUT.md](PINOUT.md) for komplet pin diagram og forbindelser.

### Hurtig Reference

**Venstre Motor (BTS7960):**
- RPWM: GPIO32, LPWM: GPIO33, R_EN: GPIO25, L_EN: GPIO26
- R_IS: GPIO34, L_IS: GPIO35

**Højre Motor (BTS7960):**
- RPWM: GPIO27, LPWM: GPIO14, R_EN: GPIO12, L_EN: GPIO13
- R_IS: GPIO36, L_IS: GPIO39

**Ultralyd Sensorer:**
- Venstre: TRIG GPIO15, ECHO GPIO2
- Midter: TRIG GPIO4, ECHO GPIO16
- Højre: TRIG GPIO17, ECHO GPIO5

**IMU (I2C):**
- SDA: GPIO21, SCL: GPIO22

**Andet:**
- Relay: GPIO23
- Battery ADC: GPIO19 (med voltage divider)
- LED (optional): GPIO18

## 🚀 Installation

### 1. Arduino IDE Setup

1. Installer **Arduino IDE 2.x** fra [arduino.cc](https://www.arduino.cc/en/software)
2. Tilføj ESP32 board support:
   - Åbn Preferences → Additional Board Manager URLs
   - Tilføj: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Installer "esp32 by Espressif Systems" via Board Manager

### 2. Library Installation

Installer følgende libraries via Arduino Library Manager:

```
- AsyncTCP by mathieucarbou - For ESP32 async networking
- ESPAsyncWebServer by mathieucarbou - Async web server
- ArduinoJson by Benoit Blanchon (version 7.x eller nyere)
- MPU9250 by hideakitai (Hideaki Tai) ELLER
- MPU6050 by Electronic Cats eller Adafruit MPU6050 by Adafruit
```

**Valgfrit (hvis eksternt display ønskes):**
```
- U8g2 by olikraus (Oliver Kraus) - For I2C OLED display
```

**VIGTIGT - ESP32 Kompatibilitet**:
- Brug **mathieucarbou** versionerne af AsyncTCP og ESPAsyncWebServer
- Disse versioner er opdateret og aktivt vedligeholdt
- Kan installeres direkte fra Library Manager

**Manuel Installation (Alternativ)**:
```
AsyncTCP:          https://github.com/mathieucarbou/AsyncTCP
ESPAsyncWebServer: https://github.com/mathieucarbou/ESPAsyncWebServer
```

Download ZIP fra GitHub → Arduino IDE → Sketch → Include Library → Add .ZIP Library

### 3. Konfiguration

1. Kopiér `src/config/Credentials.h.example` til `src/config/Credentials.h`
2. Indtast dit WiFi SSID og password:

```cpp
#define WIFI_SSID           "DitWiFiNavn"
#define WIFI_PASSWORD       "DitWiFiPassword"
```

3. Juster konstanter i `src/config/Config.h` efter behov

### 4. Upload

1. Forbind ESP32 via USB
2. Vælg board: **ESP32 Dev Module** eller **NodeMCU-32S**
3. Vælg korrekt COM port
4. Upload Speed: 115200 eller 921600
5. Tryk Upload
6. Åbn Serial Monitor (115200 baud) for at se status

Se [SETUP.md](SETUP.md) for detaljeret opsætningsguide.

## 🌐 Web Interface

Efter opstart er web interfacet tilgængeligt på:

- **mDNS**: `http://robot-mower.local`
- **IP**: Se Serial Monitor for IP adresse

### API Endpoints

Se [API.md](API.md) for komplet API dokumentation.

**Automatisk Kontrol:**
- `GET /api/status` - Hent robot status
- `POST /api/start` - Start klipning
- `POST /api/stop` - Stop klipning
- `POST /api/pause` - Pause klipning
- `POST /api/calibrate` - Kalibrér sensorer
- `GET /api/logs` - Hent debug logs

**Manuel Kontrol:**
- `POST /api/manual/forward` - Kør fremad
- `POST /api/manual/backward` - Kør baglæns
- `POST /api/manual/left` - Drej venstre
- `POST /api/manual/right` - Drej højre
- `POST /api/manual/stop` - Stop motorer
- `POST /api/manual/speed` - Sæt motorhastighed

**Klippemotor:**
- `POST /api/cutting/start` - Start klipning
- `POST /api/cutting/stop` - Stop klipning

**Strømovervågning:**
- `GET /api/current` - Hent strømdata

### WebSocket

Real-time data på `ws://robot-mower.local/ws`

## 📁 Projekt Struktur

```
RobotMower/
├── RobotMower.ino              # Hovedfil (setup + loop)
└── src/                        # Kildekode (Arduino IDE kompatibel struktur)
    ├── config/
    │   ├── Config.h            # System konstanter og pin-definitioner
    │   └── Credentials.h.example # WiFi credentials template
    ├── hardware/
    │   ├── Motors.*            # Motor kontrol (BTS7960)
    │   ├── Sensors.*           # Ultralyd sensorer (HC-SR04)
    │   ├── IMU.*               # Gyroscope/accelerometer (MPU-6050/9250)
    │   ├── Display.*           # Display support (deaktiveret som standard)
    │   ├── CuttingMechanism.*  # Klippermotor kontrol (relay)
    │   └── Battery.*           # Batteri monitoring (voltage divider)
    ├── navigation/
    │   ├── PathPlanner.*       # Rute planlægning
    │   ├── ObstacleAvoidance.* # Forhindring detection
    │   └── Movement.*          # Bevægelses kontrol
    ├── system/
    │   ├── StateManager.*      # State machine
    │   └── Logger.*            # Logging system
    ├── web/
    │   ├── WebServer.*         # HTTP server
    │   ├── WebAPI.*            # REST API
    │   ├── WebSocket.*         # WebSocket handler
    │   └── data/
    │       ├── index.html      # Web interface
    │       ├── style.css       # Styling
    │       └── app.js          # Frontend JavaScript
    └── utils/
        ├── Timer.*             # Non-blocking timers
        └── Math.*              # Math utilities
```

**Bemærk**: Alle moduler er placeret i `src/` mappen for Arduino IDE kompatibilitet.

## 🎯 Brug

### Initial Opsætning

1. **Power ON**: Tænd robotten
2. **WiFi Forbindelse**: Vent på WiFi forbindelse (se Serial Monitor)
3. **Kalibrering**: Tryk "Calibrate" i web interface
4. **Placering**: Sæt robotten på græsplænen

### Start Klipning

1. Åbn web interface: `http://robot-mower.local`
2. Tryk "Start" knappen
3. Robotten starter systematisk klipning

### Overvågning

- **Web Interface**: Full dashboard med sensorer, batteri, logs
- **Serial Monitor**: Detaljeret debug output
- **Ekstern Display (optional)**: I2C OLED kan tilsluttes

### Stop/Pause

- Tryk "Stop" for permanent stop
- Tryk "Pause" for midlertidig pause
- Emergency stop via web interface

## 🔧 Troubleshooting

### Arduino IDE Linker Errors

Hvis du får "undefined reference" fejl under kompilering:

- **Årsag**: Arduino IDE kræver at alle .cpp/.h filer i subdirectories er placeret i en `src/` mappe
- **Løsning**: Projektet er allerede struktureret korrekt med `src/` mappen
- **Verificér**: Alle moduler skal være i `src/` mappen (config, hardware, navigation, system, web, utils)
- **Alternativ**: Overvej at bruge PlatformIO i stedet for Arduino IDE for bedre projekt struktur support

### WiFi Forbinder Ikke

- Tjek SSID og password i `Credentials.h`
- Prøv at genstarte robotten
- Tjek signal styrke
- Se Serial Monitor for fejlbeskeder

### Motorer Kører Ikke

- Tjek motor driver forbindelser (BTS7960)
- Verificér power supply (18V til motorer, 5V til ESP32)
- Tjek at emergency stop ikke er aktiveret
- Verificér enable pins (R_EN, L_EN) er HIGH
- Se motor debug output i Serial Monitor

### Sensorer Læser Forkert

- Tjek sensor forbindelser (TRIG/ECHO pins)
- Rengør sensor overflader
- Kør sensor kalibrering
- Juster `OBSTACLE_THRESHOLD` i Config.h
- Verificér 5V power til HC-SR04 sensorer

### IMU Drifter

- Kør IMU kalibrering (robot skal være helt stille)
- Tjek I2C forbindelser (GPIO21 SDA, GPIO22 SCL)
- Tjek for magnetiske forstyrrelser
- Verificér 3.3V power til IMU (IKKE 5V!)
- Scan I2C bus: address skal være 0x68 eller 0x69

### Batteri Læser Forkert

- Tjek voltage divider (10kΩ + 2.2kΩ)
- Verificér forbindelse til GPIO19
- Mål divider output med multimeter (skal være <3.3V)
- Juster `BATTERY_R1` og `BATTERY_R2` i Config.h hvis nødvendigt

## 🔐 Sikkerhed

### Indbyggede Sikkerhedsfunktioner

- **Emergency Stop**: Øjeblikkelig stop af alle motorer
- **Battery Protection**: Auto-stop ved kritisk batteri
- **Tilt Detection**: Stop hvis robot vælter (med IMU)
- **Obstacle Detection**: Undgår kollisioner
- **Safety Lock**: Klippermotor kan ikke starte utilsigtet
- **Watchdog Timer**: Auto-reset ved system hang
- **Current Monitoring**: Overvågning af motor strøm

### Vigtige Sikkerhedsregler

⚠️ **ADVARSEL**: Klippemekanismen er skarp og farlig!

- Aktiver ALTID safety lock før vedligeholdelse
- Test ALTID uden klippermotor først
- Hold kæledyr og børn væk under drift
- Inspicer område for objekter før start
- Brug beskyttelsesbriller ved vedligeholdelse
- Tjek voltage divider før tilslutning af batteri

## 📊 Performance

- **Klipningshastighed**: ~20 cm/s
- **Køretid**: ~45-60 min (afhængig af batteri)
- **Række bredde**: 30 cm (justerbar)
- **Obstacle reaction**: <100ms
- **Heading præcision**: ±5°
- **Motor strøm**: Op til 43A per motor (BTS7960)

## 🛠️ Udvikling

### Tilføje Nye Features

Projektet er modulært opbygget for let udvidelse:

1. **Ny Hardware**: Tilføj ny klasse i `hardware/`
2. **Ny Navigation**: Udvid `navigation/` moduler
3. **Ny Web Feature**: Tilføj endpoint i `WebAPI.cpp`

### Debug Mode

Aktiver debug output i `Config.h`:

```cpp
#define DEBUG_MODE      true
#define DEBUG_SENSORS   true
#define DEBUG_MOTORS    true
#define DEBUG_NAVIGATION true
```

### Tilføj Eksternt Display

For at tilføje I2C OLED display:

1. Installer U8g2 library
2. Sæt `ENABLE_DISPLAY = true` i Config.h
3. Tilslut display til I2C (GPIO21 SDA, GPIO22 SCL)
4. Display deler I2C bus med IMU

## 🤝 Bidrag

Bidrag er velkomne! Fork projektet og submit pull requests.

## 📄 Licens

Dette projekt er open source og frit tilgængeligt til ikke-kommerciel brug.

## 👥 Credits

- **ESP32 Arduino Core**: Espressif Systems
- **AsyncTCP**: mathieucarbou
- **ESPAsyncWebServer**: mathieucarbou
- **ArduinoJson**: Benoit Blanchon
- **U8g2**: olikraus (Oliver Kraus)
- **MPU9250 Library**: hideakitai (Hideaki Tai)
- **MPU6050 Library**: Electronic Cats / Adafruit

## 📞 Support

- **Issues**: Åbn et issue på GitHub
- **Diskussion**: Se discussions på GitHub
- **Wiki**: Check projektet wiki for guides

## 🗺️ Roadmap

### Planlagte Features

- [ ] GPS navigation
- [ ] Perimeter wire support
- [ ] Auto-return til ladestation
- [ ] Regnfølsomhed
- [ ] Kamera baseret græs detection
- [ ] Mobile app
- [ ] Multi-zone support
- [ ] Tidsplanlægning
- [ ] Ekstern I2C OLED display support

## 🆚 Hardware Versioner

Dette projekt understøtter ESP32-WROOM-32U 38-pin development board.

**Fordele ved ESP32-WROOM-32U:**
- Billigere (~$5-10)
- Veldokumenteret og populær platform
- Standard 38-pin breadboard-friendly layout
- Tilstødende pins for komponenter
- Bred library support

Se PINOUT.md for komplet pin mapping og sammenligning.

---

**Bygget med ❤️ til autonome plæneklippere**
