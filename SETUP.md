# 📝 Detaljeret Opsætningsguide

Denne guide gennemgår hele opsætningsprocessen fra bunden - fra software installation til første test kørsel.

## 📋 Indholdsfortegnelse

1. [Software Installation](#software-installation)
2. [Hardware Samling](#hardware-samling)
3. [Konfiguration](#konfiguration)
4. [Upload og Test](#upload-og-test)
5. [Kalibrering](#kalibrering)
6. [Første Test Kørsel](#første-test-kørsel)

---

## 1. Software Installation

### Trin 1.1: Arduino IDE

1. Download Arduino IDE 2.x fra [arduino.cc](https://www.arduino.cc/en/software)
2. Installer programmet
3. Åbn Arduino IDE

### Trin 1.2: ESP32 Board Support

1. Åbn **File → Preferences** (eller Arduino IDE → Settings på Mac)
2. I feltet "Additional Board Manager URLs", tilføj:

```
https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.7/package_heltec_esp32_index.json
```

3. Klik OK
4. Åbn **Tools → Board → Boards Manager**
5. Søg efter "Heltec ESP32"
6. Installer "Heltec ESP32 Dev-Boards"
7. Vent på installation færdig

### Trin 1.3: Installér Required Libraries

Åbn **Tools → Manage Libraries** og installer følgende:

#### Core Libraries
- **AsyncTCP** by dvarrel
- **ESPAsyncWebServer** by lacamera
- **ArduinoJson** by Benoit Blanchon (v6.x)

#### Sensor Libraries
- **MPU9250** by hideakitai (ELLER MPU6050 by Electronic Cats)
- **Adafruit MPU6050** (alternativ)

#### Display Library
- **U8g2** by oliver (til OLED display)

#### Heltec Library
- Allerede inkluderet i Heltec ESP32 board package

### Trin 1.4: Verificér Installation

1. Vælg **Tools → Board → Heltec ESP32 Arduino → WiFi Kit 32 (V3)**
2. Hvis boardet vises, er installationen succesfuld ✅

---

## 2. Hardware Samling

### Trin 2.1: Forbered Komponenter

Check at du har alle komponenter fra [HARDWARE.md](HARDWARE.md):

- [ ] Heltec WiFi Kit 32 V3
- [ ] L298N Motor Driver
- [ ] 2x DC Gear Motors
- [ ] 3x HC-SR04 Ultralyd Sensorer
- [ ] MPU-6050/9250 IMU
- [ ] Relay Modul
- [ ] 3S LiPo Batteri
- [ ] Forbindingsledninger
- [ ] Chassis/ramme

### Trin 2.2: Motor Driver Forbindelser

Følg [PINOUT.md](PINOUT.md) for detaljerede forbindelser.

#### L298N → ESP32 Forbindelser:
```
L298N                     ESP32-S3
─────────────────────────────────────
ENA (PWM venstre)    →    GPIO 5
IN1 (venstre retning)→    GPIO 19
IN2 (venstre retning)→    GPIO 18
IN3 (højre retning)  →    GPIO 16
IN4 (højre retning)  →    GPIO 15
ENB (PWM højre)      →    GPIO 17

+12V                 →    Batteri +
GND                  →    Batteri - & ESP32 GND
```

⚠️ **VIGTIGT**: Husk fælles GND mellem ESP32 og motor driver!

### Trin 2.3: Sensor Forbindelser

#### Venstre Sensor (HC-SR04):
```
Sensor          ESP32-S3
─────────────────────────
VCC        →    5V (eller 3.3V)
TRIG       →    GPIO 21
ECHO       →    GPIO 47
GND        →    GND
```

#### Midter Sensor:
```
VCC        →    5V
TRIG       →    GPIO 48
ECHO       →    GPIO 35
GND        →    GND
```

#### Højre Sensor:
```
VCC        →    5V
TRIG       →    GPIO 36
ECHO       →    GPIO 37
GND        →    GND
```

### Trin 2.4: IMU Forbindelser (MPU-6050/9250)

```
MPU         ESP32-S3
──────────────────────
VCC    →    3.3V
GND    →    GND
SCL    →    GPIO 42
SDA    →    GPIO 41
```

⚠️ **OBS**: Brug ALTID 3.3V til IMU, IKKE 5V!

### Trin 2.5: Klippermotor Relay

```
Relay       ESP32-S3
──────────────────────
VCC    →    5V
GND    →    GND
IN     →    GPIO 38
```

Relay kontakter:
```
NO (Normally Open)  → Klippermotor +
COM (Common)        → Batteri +
Klippermotor -      → Batteri -
```

### Trin 2.6: Batteri Monitoring

Voltage Divider til batteri monitoring:
```
Batteri + → [R1: 10kΩ] → [Målepunkt] → [R2: 2.2kΩ] → GND
                            ↓
                        ESP32 GPIO 1 (ADC)
```

⚠️ **VIGTIGT**: Verificér at spændingen ved målepunktet ALDRIG overstiger 3.3V!

### Trin 2.7: Power Distribution

```
3S LiPo Batteri (11.1V)
    ↓
    ├─→ L298N Motor Driver (12V input)
    ├─→ Klippermotor Relay (via relay)
    ├─→ Buck Converter (11.1V → 5V)
    │       ↓
    │       ├─→ ESP32 VIN (5V)
    │       ├─→ Sensorer (5V)
    │       └─→ Relay (5V)
    └─→ Voltage Divider til monitoring
```

### Trin 2.8: Første Power-On Test

1. Før tilslutning af batteri, **dobbelttjek alle forbindelser**
2. Fjern klippermotor under test!
3. Tilslut USB til ESP32
4. Verificér at ESP32 tænder (OLED lyser)
5. Upload en simpel "blink" sketch for at teste
6. Hvis OK, frakobl USB
7. Tilslut batteri (uden klippermotor)
8. Verificér at ESP32 tænder via batteri
9. **Hvis noget bliver varmt, frakobl STRAKS!**

---

## 3. Konfiguration

### Trin 3.1: Download Projekt

1. Download eller clone projektet
2. Åbn `RobotMower.ino` i Arduino IDE
3. Verificér at alle filer loader korrekt

### Trin 3.2: WiFi Credentials

1. Naviger til `config/` mappen
2. Kopiér `Credentials.h.example` → `Credentials.h`
3. Åbn `Credentials.h` og indtast dit WiFi:

```cpp
#define WIFI_SSID           "DitWiFiNavn"
#define WIFI_PASSWORD       "DitWiFiPassword"
```

### Trin 3.3: Pin Konfiguration

Åbn `config/Config.h` og verificér pin definitioner:

- Tjek at pins matcher dine fysiske forbindelser
- Juster hvis du har brugt andre pins

### Trin 3.4: Juster Konstanter

I `Config.h`, juster følgende efter dit setup:

```cpp
// Sensor thresholds
#define OBSTACLE_THRESHOLD          30     // cm (juster efter behov)

// Motor hastigheder (test med lave værdier først!)
#define MOTOR_CRUISE_SPEED          200    // Start med lav hastighed
#define MOTOR_TURN_SPEED            180

// Batteri (juster efter dit batteri)
#define BATTERY_MAX_VOLTAGE         12.6   // 3S LiPo fuld
#define BATTERY_LOW_VOLTAGE         10.5   // Lav advarsel
#define BATTERY_CRITICAL_VOLTAGE    10.0   // Stop

// Voltage divider (baseret på dine modstande)
#define BATTERY_R1                  10000.0  // 10kΩ
#define BATTERY_R2                  2200.0   // 2.2kΩ
```

### Trin 3.5: Enable/Disable Features

```cpp
// Feature flags - deaktivér hvis hardware ikke er tilsluttet
#define ENABLE_DISPLAY              true
#define ENABLE_IMU                  true
#define ENABLE_WEBSOCKET            true

// Debug modes
#define DEBUG_MODE                  true   // Altid ON under test!
#define DEBUG_SENSORS               true
#define DEBUG_MOTORS                true
```

---

## 4. Upload og Test

### Trin 4.1: Compile Test

1. Klik **Verify/Compile** knappen (✓)
2. Vent på compilation færdig
3. Tjek for fejl i output vinduet
4. Hvis fejl: verificér at alle libraries er installeret

### Trin 4.2: Board Konfiguration

```
Tools → Board: "WiFi Kit 32 (V3)"
Tools → Upload Speed: "921600"
Tools → CPU Frequency: "240MHz"
Tools → Flash Frequency: "80MHz"
Tools → Flash Mode: "QIO"
Tools → Flash Size: "8MB"
Tools → Partition Scheme: "Default 4MB..."
Tools → Core Debug Level: "None" (eller "Info" for debug)
```

### Trin 4.3: Upload

1. Forbind ESP32 via USB
2. Vælg korrekt **Port** under Tools → Port
3. Klik **Upload** knappen (→)
4. Vent på upload færdig (ca. 30-60 sekunder)

### Trin 4.4: Serial Monitor Test

1. Åbn **Tools → Serial Monitor**
2. Sæt baud rate til **115200**
3. Du skulle se boot beskeder:

```
============================================
   ROBOT PLÆNEKLIPPER - ESP32-S3
============================================
[INFO] System starting...
[INFO] Logger initialized
[INFO] StateManager initialized - State: IDLE
...
```

4. Verificér at alle moduler initialiserer OK
5. Noter IP adressen når WiFi forbinder

---

## 5. Kalibrering

### Trin 5.1: IMU Kalibrering

**VIGTIGT**: Robotten skal stå HELT stille under kalibrering!

**Via Serial Monitor:**
1. Placer robot på flad, stabil overflade
2. I Serial Monitor, vent på opstart
3. Robotten logger: "!!! IMPORTANT: Run calibration before use !!!"

**Via Web Interface:**
1. Åbn browser: `http://robot-mower.local` (eller IP adressen)
2. Klik "Kalibrér" knappen
3. Hold robot stille i 5 sekunder
4. Vent på "Calibration complete" besked

### Trin 5.2: Sensor Test

Test hver sensor individuelt:

1. Åbn Serial Monitor
2. Sensorer opdaterer automatisk hver sekund
3. Hold din hånd foran hver sensor:
   - Venstre sensor → se "L: XX cm"
   - Midter sensor → se "M: XX cm"
   - Højre sensor → se "R: XX cm"
4. Verificér at værdier ændrer sig korrekt

**Forventet range**: 2-400 cm

### Trin 5.3: Motor Test

⚠️ **SIKKERHED**: Løft robot op så hjul ikke rører jorden!

**Test via Web Interface:**
1. Åbn web interface
2. Klik "Start" (robotten går i MOWING state)
3. Verificér at begge motorer kører
4. Klik "Stop"
5. Motorer skal stoppe øjeblikkeligt

**Tjek i Serial Monitor:**
```
[Motors] Set speed - Left: 200, Right: 200
```

### Trin 5.4: Display Test

Hvis OLED display er tilsluttet:
1. Ved opstart vises splash screen "ROBOT MOWER v1.0 Ready"
2. Efter WiFi forbindelse vises WiFi info
3. Derefter vises status med:
   - State
   - Battery
   - Heading
   - Kompas

---

## 6. Første Test Kørsel

### Trin 6.1: Pre-Flight Check

Før første kørsel:

- [ ] Alle sensorer virker
- [ ] Motorer drejer korrekt retning
- [ ] Batteri fuldt ladet
- [ ] WiFi forbindelse stabil
- [ ] IMU kalibreret
- [ ] Web interface tilgængeligt
- [ ] **Klippermotor IKKE tilsluttet** (første test)
- [ ] Test område ryddet for forhindringer
- [ ] Safety lock deaktiveret i web interface

### Trin 6.2: Kontrolleret Test

**Indoor Test (uden klippermotor):**

1. Placer robot på gulv
2. Åbn web interface
3. Klik "Start"
4. Robotten skulle:
   - Køre langsomt fremad
   - Holde nogenlunde lige kurs
   - Stoppe ved forhindringer
   - Dreje væk fra forhindringer

5. Test obstacle avoidance:
   - Hold genstand foran robot
   - Robot skal stoppe og bakke
   - Robot skal dreje væk

6. Test stop kommando:
   - Klik "Stop" i web interface
   - Robot skal stoppe øjeblikkeligt

### Trin 6.3: Outdoor Test (små område)

**Første græs test:**

1. Vælg lille, afgrænset område (f.eks. 3x3 meter)
2. Fjern forhindringer
3. Placer robot i centrum
4. Klik "Start"
5. Observer i 5-10 minutter:
   - Kører den lige?
   - Undgår den forhindringer?
   - Følger den et mønster?

**Hvis problemer opstår:**
- Klik "Stop" øjeblikkeligt
- Tjek Serial Monitor for fejlbeskeder
- Juster hastigheder i Config.h hvis nødvendigt

### Trin 6.4: Med Klippermotor (FORSIGTIG!)

⚠️ **ADVARSEL**: Klipperblade er farlige!

Når drive motorer fungerer korrekt:

1. Sluk robot
2. Tilslut klippermotor til relay
3. Deaktivér safety lock i web interface
4. Tænd robot
5. Start i lille afgrænset område
6. Hold afstand!
7. Test kun i kort tid (<2 minutter første gang)
8. Observer for vibrationer eller problemer

---

## ✅ Troubleshooting

### Problem: WiFi forbinder ikke

**Løsning:**
1. Tjek SSID og password i `Credentials.h`
2. Tjek at WiFi router er tændt og i rækkevidde
3. Tjek Serial Monitor for specifikke fejl
4. Prøv at genstarte robot
5. Prøv 2.4GHz WiFi (ESP32 understøtter IKKE 5GHz)

### Problem: Sensorer læser altid 0

**Løsning:**
1. Tjek TRIG/ECHO forbindelser
2. Verificér sensor power (VCC → 5V)
3. Tjek GND forbindelse
4. Test med multimeter: VCC pin skulle måle ~5V
5. Prøv en anden sensor for at isolere problemet

### Problem: Motor drejer forkert retning

**Løsning:**
1. Byt IN1/IN2 forbindelser for den pågældende motor
2. ELLER byt motor ledninger (+/-)

### Problem: IMU drifter meget

**Løsning:**
1. Kør kalibrering igen (robot skal være HELT stille)
2. Tjek for magnetiske forstyrrelser
3. Monter IMU fast (vibrationer påvirker måling)
4. Juster komplementær filter alpha værdi i IMU.cpp

### Problem: Batteri læser forkert værdi

**Løsning:**
1. Verificér voltage divider modstande
2. Mål spænding ved ADC pin med multimeter (SKAL være <3.3V!)
3. Juster `BATTERY_R1` og `BATTERY_R2` i Config.h
4. Kalibrér ADC reference voltage hvis nødvendigt

---

## 🎉 Success!

Hvis du er nået hertil og alt fungerer - tillykke! 🎊

Din robot plæneklipper er nu klar til brug.

**Næste skridt:**
- Læs [API.md](API.md) for avanceret kontrol
- Eksperimenter med konstanter i `Config.h`
- Tilføj custom features
- Del din erfaring!

**Hav det sjovt med din autonome plæneklipper!** 🤖🌱
