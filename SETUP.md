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

⚠️ **VIGTIGT**: Til ESP32-S3 skal du bruge ESP32Async versionerne (aktivt vedligeholdt)!

- **AsyncTCP** by **ESP32Async** (mathieucarbou)
  - GitHub: https://github.com/ESP32Async/AsyncTCP
  - Dette er den opdaterede fork til ESP32-S3
  - ⚠️ IKKE "me-no-dev" versionen (ikke længere vedligeholdt)
  - ⚠️ IKKE "dvarrel" versionen!
  - **SKAL installeres manuelt** (ikke i Library Manager)

- **ESPAsyncWebServer** by **ESP32Async** (mathieucarbou)
  - GitHub: https://github.com/ESP32Async/ESPAsyncWebServer
  - Dette er den opdaterede fork til ESP32-S3
  - ⚠️ IKKE "me-no-dev" versionen (ikke længere vedligeholdt)
  - ⚠️ IKKE "lacamera" versionen!
  - ⚠️ IKKE "ESP_Async_WebServer" (med underscores)!
  - **SKAL installeres manuelt** (ikke i Library Manager)

- **ArduinoJson** by **Benoit Blanchon** (version 6.x eller nyere)
  - Denne findes i Library Manager
  - Version 7.x virker også

#### Sensor Libraries
Vælg ÉN af følgende IMU libraries:
- **MPU9250** by **hideakitai** (Hideaki Tai) - Anbefalet til 9-axis
  ELLER
- **MPU6050** by **Electronic Cats** - God til 6-axis
  ELLER
- **Adafruit MPU6050** by **Adafruit** - Alternativ til 6-axis

#### Display Library
- **U8g2** by **olikraus** (Oliver Kraus)
  - Komplet navn: "U8g2 by oliver"

#### Heltec Library
- Allerede inkluderet i Heltec ESP32 board package fra trin 1.2

### ⚠️ VIGTIGT - Verificér Udviklere

I Arduino Library Manager, tjek ALTID "by [udvikler navn]" feltet før installation!
Forkerte libraries vil give compile errors.

#### Sådan Finder Du Det Rigtige Library

1. Åbn **Tools → Manage Libraries**
2. Søg efter library navnet (f.eks. "AsyncTCP")
3. Der vil ofte være FLERE resultater
4. Kig på "by [navn]" under hver library
5. Vælg det med den KORREKTE udvikler (se listen ovenfor)
6. Klik **Install**

**Eksempel - AsyncTCP**:
```
❌ AsyncTCP by dvarrel           <- FORKERT!
❌ AsyncTCP by me-no-dev         <- FORÆLDET (ikke til ESP32-S3)!
✅ AsyncTCP by ESP32Async        <- KORREKT! (Manuel installation)
```

**Eksempel - ESPAsyncWebServer**:
```
❌ ESPAsyncWebServer by lacamera       <- FORKERT!
❌ ESP_Async_WebServer (med _)         <- FORKERT!
❌ ESPAsyncWebServer by me-no-dev      <- FORÆLDET (ikke til ESP32-S3)!
✅ ESPAsyncWebServer by ESP32Async     <- KORREKT! (Manuel installation)
```

### Trin 1.3.1: Manuel Installation af AsyncTCP og ESPAsyncWebServer

**DISSE LIBRARIES SKAL INSTALLERES MANUELT** da de ikke findes i Arduino Library Manager.

#### Metode 1: Download ZIP (Nemmest)

**1. Installer AsyncTCP:**
1. Gå til https://github.com/ESP32Async/AsyncTCP
2. Klik "Code" → "Download ZIP"
3. I Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
4. Vælg den downloadede ZIP fil
5. Vent til "Library added to your libraries" vises

**2. Installer ESPAsyncWebServer:**
1. Gå til https://github.com/ESP32Async/ESPAsyncWebServer
2. Klik "Code" → "Download ZIP"
3. I Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
4. Vælg den downloadede ZIP fil
5. Vent til "Library added to your libraries" vises

**3. Genstart Arduino IDE** - VIGTIGT!

#### Metode 2: Git Clone (Avanceret)

**Windows:**
```bash
cd C:\Users\[dit navn]\Documents\Arduino\libraries\
git clone https://github.com/ESP32Async/AsyncTCP.git
git clone https://github.com/ESP32Async/ESPAsyncWebServer.git
```

**Mac/Linux:**
```bash
cd ~/Documents/Arduino/libraries/
git clone https://github.com/ESP32Async/AsyncTCP.git
git clone https://github.com/ESP32Async/ESPAsyncWebServer.git
```

**Genstart Arduino IDE** efter installation!

### Trin 1.4: Verificér Installation

1. Vælg **Tools → Board → Heltec ESP32 Arduino → WiFi Kit 32 (V3)**
2. Hvis boardet vises, er installationen succesfuld ✅

---

## 2. Hardware Samling

### Trin 2.1: Forbered Komponenter

Check at du har alle komponenter fra [HARDWARE.md](HARDWARE.md):

- [ ] Heltec WiFi Kit 32 V3
- [ ] 2x Double BTS7960 43A H-Bridge Motor Driver
- [ ] 2x DC Gear Motors (18V)
- [ ] 3x HC-SR04 Ultralyd Sensorer
- [ ] MPU-6050/9250 IMU
- [ ] Relay Modul
- [ ] 5S LiPo Batteri (18V for motorer)
- [ ] 3S LiPo Batteri (12V for kontrol)
- [ ] Buck Converter (12V → 5V)
- [ ] Forbindingsledninger
- [ ] Chassis/ramme

### Trin 2.2: Motor Driver Forbindelser (BTS7960)

Følg [PINOUT.md](PINOUT.md) for detaljerede forbindelser.

#### Venstre Motor (BTS7960) → ESP32 Forbindelser:
```
BTS7960                   ESP32-S3
─────────────────────────────────────
RPWM (fremad PWM)    →    GPIO 5
LPWM (baglæns PWM)   →    GPIO 19
R_EN (fremad enable) →    GPIO 18
L_EN (baglæns enable)→    GPIO 17
R_IS (strømsensor)   →    GPIO 2  (ADC)
L_IS (strømsensor)   →    GPIO 3  (ADC)

VCC (logic)          →    3.3V
GND                  →    GND

B+ (motor power)     →    18V+ (5S LiPo)
B- (motor power)     →    18V- (5S LiPo)
M+, M-               →    Venstre motor
```

#### Højre Motor (BTS7960) → ESP32 Forbindelser:
```
BTS7960                   ESP32-S3
─────────────────────────────────────
RPWM (fremad PWM)    →    GPIO 16
LPWM (baglæns PWM)   →    GPIO 15
R_EN (fremad enable) →    GPIO 4
L_EN (baglæns enable)→    GPIO 6
R_IS (strømsensor)   →    GPIO 7  (ADC)
L_IS (strømsensor)   →    GPIO 8  (ADC)

VCC (logic)          →    3.3V
GND                  →    GND

B+ (motor power)     →    18V+ (5S LiPo)
B- (motor power)     →    18V- (5S LiPo)
M+, M-               →    Højre motor
```

⚠️ **VIGTIGT**:
- Husk fælles GND mellem ESP32, begge BTS7960 drivers og batteri!
- BTS7960 logic (VCC) forbindes til 3.3V fra ESP32
- BTS7960 motor power (B+/B-) forbindes til 18V fra 5S LiPo
- R_EN og L_EN skal være HIGH for at enable driveren
- ALDRIG sæt både RPWM og LPWM højt samtidigt!
- Current sense pins (R_IS, L_IS) giver 10mV/A output

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
5S LiPo Batteri (18.5V)           3S LiPo Batteri (11.1V)
[Motor Power]                     [Control Power]
    ↓                                  ↓
    ├─────────────┐                    │
    │             │                    │
BTS7960       BTS7960            Buck Converter
Venstre       Højre              (12V → 5V, 3A)
Driver        Driver                   ↓
    │             │              ┌─────┼─────┐
Venstre      Højre              │     │     │
Motor        Motor          ESP32   Sensorer Relay
(18V)        (18V)          VIN(5V) (5V)    (5V)
    │             │                    │
    │             │              Voltage Divider
    │             │              → GPIO 1 (ADC)
    │             │
Klippermotor (18V via relay)
```

**⚠️ KRITISK VIGTIGT**:
- **ADSKILT STRØMFORSYNING**: Motorer (18V fra 5S) og kontrol (12V fra 3S) er SEPARATE!
- **FÆLLES GND**: Alle GND skal forbindes sammen (både 18V og 12V system)
- BTS7960 drivere får 18V power fra 5S LiPo (B+/B-)
- BTS7960 logic (VCC) forbindes til 3.3V fra ESP32
- Buck converter skal levere minimum 3A ved 5V
- ESP32 VIN pin kan tage 5V input (intern regulator til 3.3V)
- Klippermotor bruger også 18V power via relay

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

### Problem: Compile error - 'HTTP_GET' was not declared

**Fejlbesked:**
```
error: 'HTTP_GET' was not declared in this scope
error: 'HTTP_ANY' was not declared in this scope
```

**Årsag:**
Du har installeret forkert eller forældet ESPAsyncWebServer library!

**Løsning:**
1. **Fjern alle gamle versioner:**
   - Gå til `C:\Users\[dit navn]\Documents\Arduino\libraries\`
   - Slet mapper: `ESPAsyncWebServer`, `ESP_Async_WebServer`, `AsyncTCP`

2. **Installer ESP32Async versioner (manuelt):**
   - Download AsyncTCP: https://github.com/ESP32Async/AsyncTCP
   - Download ESPAsyncWebServer: https://github.com/ESP32Async/ESPAsyncWebServer
   - Klik "Code" → "Download ZIP" for begge
   - I Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
   - Tilføj begge ZIP filer

3. **Genstart Arduino IDE**

4. **Prøv at compile igen** ✅

### Problem: Compile error - 'createStatusJSON()' is private

**Løsning:**
Denne fejl er rettet i den seneste version af koden.
Pull den seneste version fra git eller opdater `web/WebAPI.h`.

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
