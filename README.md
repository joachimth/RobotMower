# 🤖 Robot Plæneklipper - ESP32-S3

Autonome plæneklipper bygget på ESP32-S3 med systematisk klipningsmønster, ultralyd baseret forhindring undgåelse og web kontrol interface.

![Project Status](https://img.shields.io/badge/status-active-success)
![Version](https://img.shields.io/badge/version-1.0-blue)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-orange)

## ✨ Features

- **Systematisk Klipning**: Parallelt række-mønster for effektiv plæneklipning
- **Forhindring Undgåelse**: 3x ultralyd sensorer til obstacle detection
- **IMU Navigation**: MPU-6050/9250 til præcis retningsbestemmelse
- **Web Interface**: Real-time kontrol og monitoring via WiFi
- **WebSocket Telemetri**: Live sensor data og status updates
- **OLED Display**: Indbygget status display
- **Batteri Overvågning**: Automatisk low-battery håndtering
- **State Machine**: Robust state management system
- **Logging System**: Comprehensive debug logging

## 📦 Hardware Krav

### Hovedkomponenter
- **Heltec WiFi Kit 32 V3** (ESP32-S3) - Hovedcontroller
- **L298N Motor Driver** - DC motor kontrol
- **2x DC Gear Motors** - Drive motors
- **3x HC-SR04 Ultralyd Sensorer** - Forhindring detection
- **MPU-6050 eller MPU-9250** - IMU (accelerometer + gyroscope)
- **Relay Modul** - Klippermotor kontrol
- **3S LiPo Batteri** (11.1V) - Power supply
- **Voltage Divider** - Batteri monitoring

### Valgfrie Komponenter
- Perimeter wire sensor
- GPS modul
- Kamera modul
- Ladestation beacon

Se [HARDWARE.md](HARDWARE.md) for detaljeret hardware liste og specifikationer.

## 🔌 Pin Forbindelser

Se [PINOUT.md](PINOUT.md) for komplet pin diagram og forbindelser.

### Hurtig Reference

| Komponent | Pins |
|-----------|------|
| Venstre Motor | PWM: 5, IN1: 19, IN2: 18 |
| Højre Motor | PWM: 17, IN1: 16, IN2: 15 |
| Venstre Sensor | TRIG: 21, ECHO: 47 |
| Midter Sensor | TRIG: 48, ECHO: 35 |
| Højre Sensor | TRIG: 36, ECHO: 37 |
| IMU | SDA: 41, SCL: 42 |
| Klippermotor Relay | Pin: 38 |
| Batteri Monitor | ADC: 1 |

## 🚀 Installation

### 1. Arduino IDE Setup

1. Installer **Arduino IDE 2.x** fra [arduino.cc](https://www.arduino.cc/en/software)
2. Tilføj ESP32 board support:
   - Åbn Preferences → Additional Board Manager URLs
   - Tilføj Heltec ESP32 URL
3. Installer "Heltec ESP32 Dev-Boards" via Board Manager

### 2. Library Installation

Installer følgende libraries via Arduino Library Manager:

```
- Heltec ESP32 Dev-Boards by Heltec Automation
- AsyncTCP by ESP32Async (mathieucarbou) - Manuel installation påkrævet
- ESPAsyncWebServer by ESP32Async (mathieucarbou) - Manuel installation påkrævet
- ArduinoJson by Benoit Blanchon (version 6.x eller nyere)
- MPU9250 by hideakitai (Hideaki Tai) ELLER
- MPU6050 by Electronic Cats eller Adafruit MPU6050 by Adafruit
- U8g2 by olikraus (Oliver Kraus)
```

**VIGTIGT - ESP32-S3 Kompatibilitet**:
- AsyncTCP og ESPAsyncWebServer skal installeres **manuelt** fra GitHub
- Brug **ESP32Async** versionerne (ikke me-no-dev versioner)
- ESP32Async versionerne er opdateret til ESP32-S3 og aktivt vedligeholdt

**Manuel Installation (Påkrævet)**:
```
AsyncTCP:          https://github.com/ESP32Async/AsyncTCP
ESPAsyncWebServer: https://github.com/ESP32Async/ESPAsyncWebServer
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
2. Vælg board: **WiFi Kit 32 (V3)**
3. Vælg korrekt COM port
4. Tryk Upload
5. Åbn Serial Monitor (115200 baud) for at se status

Se [SETUP.md](SETUP.md) for detaljeret opsætningsguide.

## 🌐 Web Interface

Efter opstart er web interfacet tilgængeligt på:

- **mDNS**: `http://robot-mower.local`
- **IP**: Se Serial Monitor eller OLED display for IP adresse

### API Endpoints

Se [API.md](API.md) for komplet API dokumentation.

Hurtig reference:
- `GET /api/status` - Hent robot status
- `POST /api/start` - Start klipning
- `POST /api/stop` - Stop klipning
- `POST /api/pause` - Pause klipning
- `POST /api/calibrate` - Kalibrér sensorer
- `GET /api/logs` - Hent debug logs

### WebSocket

Real-time data på `ws://robot-mower.local/ws`

## 📁 Projekt Struktur

```
RobotMower/
├── RobotMower.ino              # Hovedfil (setup + loop)
└── src/                        # Kildekode (Arduino IDE kompatibel struktur)
    ├── config/
    │   ├── Config.h            # System konstanter
    │   └── Credentials.h.example # WiFi credentials template
    ├── hardware/
    │   ├── Motors.*            # Motor kontrol
    │   ├── Sensors.*           # Ultralyd sensorer
    │   ├── IMU.*               # Gyroscope/accelerometer
    │   ├── Display.*           # OLED display
    │   ├── CuttingMechanism.*  # Klippermotor kontrol
    │   └── Battery.*           # Batteri monitoring
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
2. **WiFi Forbindelse**: Vent på WiFi forbindelse (se OLED/Serial)
3. **Kalibrering**: Tryk "Calibrate" i web interface
4. **Placering**: Sæt robotten på græsplænen

### Start Klipning

1. Åbn web interface: `http://robot-mower.local`
2. Tryk "Start" knappen
3. Robotten starter systematisk klipning

### Overvågning

- **OLED Display**: Viser real-time status lokalt
- **Web Interface**: Full dashboard med sensorer, batteri, logs
- **Serial Monitor**: Detaljeret debug output

### Stop/Pause

- Tryk "Stop" for permanent stop
- Tryk "Pause" for midlertidig pause
- Emergency stop via OLED knap (hvis implementeret)

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

- Tjek motor driver forbindelser
- Verificér power supply
- Tjek at emergency stop ikke er aktiveret
- Se motor debug output i Serial Monitor

### Sensorer Læser Forkert

- Tjek sensor forbindelser (TRIG/ECHO pins)
- Rengør sensor overflader
- Kør sensor kalibrering
- Juster `OBSTACLE_THRESHOLD` i Config.h

### IMU Drifter

- Kør IMU kalibrering (robot skal være helt stille)
- Tjek for magnetiske forstyrrelser
- Juster komplementær filter værdier

## 🔐 Sikkerhed

### Indbyggede Sikkerhedsfunktioner

- **Emergency Stop**: Øjeblikkelig stop af alle motorer
- **Battery Protection**: Auto-stop ved kritisk batteri
- **Tilt Detection**: Stop hvis robot vælter
- **Obstacle Detection**: Undgår kollisioner
- **Safety Lock**: Klippermotor kan ikke starte utilsigtet
- **Watchdog Timer**: Auto-reset ved system hang

### Vigtige Sikkerhedsregler

⚠️ **ADVARSEL**: Klippemekanismen er skarp og farlig!

- Aktiver ALTID safety lock før vedligeholdelse
- Test ALTID uden klippermotor først
- Hold kæledyr og børn væk under drift
- Inspicer område for objekter før start
- Brug beskyttelsesbriller ved vedligeholdelse

## 📊 Performance

- **Klipningshastighed**: ~20 cm/s
- **Køretid**: ~45-60 min (afhængig af batteri)
- **Række bredde**: 30 cm (justerbar)
- **Obstacle reaction**: <100ms
- **Heading præcision**: ±5°

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
```

## 🤝 Bidrag

Bidrag er velkomne! Fork projektet og submit pull requests.

## 📄 Licens

Dette projekt er open source og frit tilgængeligt til ikke-kommerciel brug.

## 👥 Credits

- **ESP32 Arduino Core**: Espressif Systems
- **Heltec ESP32 Dev-Boards**: Heltec Automation
- **AsyncTCP**: ESP32Async (mathieucarbou) - Fork af me-no-dev's original
- **ESPAsyncWebServer**: ESP32Async (mathieucarbou) - Fork af me-no-dev's original
- **ArduinoJson**: Benoit Blanchon
- **U8g2**: olikraus (Oliver Kraus)
- **MPU9250 Library**: hideakitai (Hideaki Tai)
- **MPU6050 Library**: Electronic Cats / Adafruit

**Tak til originale udviklere**:
- me-no-dev (Hristo Gochkov) for oprindelige AsyncTCP og ESPAsyncWebServer

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

---

**Bygget med ❤️ til autonome plæneklippere**
