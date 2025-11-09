# 📌 Pin Forbindelser Diagram

Komplet pin mapping for Robot Plæneklipper projektet - **ESP32-WROOM-32U 38-Pin Version**

## 🎯 Quick Reference Table

| Component | Function | GPIO# | ADC | Type | Notes |
|-----------|----------|-------|-----|------|-------|
| **Venstre Motor (BTS7960)** |
| Forward PWM | RPWM | GPIO 32 | ADC1_CH4 | PWM | Fremad hastighed |
| Reverse PWM | LPWM | GPIO 33 | ADC1_CH5 | PWM | Baglæns hastighed |
| Forward Enable | R_EN | GPIO 25 | ADC2_CH8 | Digital Out | Enable fremad |
| Reverse Enable | L_EN | GPIO 26 | ADC2_CH9 | Digital Out | Enable baglæns |
| Forward Current | R_IS | GPIO 34 | ADC1_CH6 | Analog In | Strømsensor fremad (input-only) |
| Reverse Current | L_IS | GPIO 35 | ADC1_CH7 | Analog In | Strømsensor baglæns (input-only) |
| **Højre Motor (BTS7960)** |
| Forward PWM | RPWM | GPIO 27 | ADC2_CH7 | PWM | Fremad hastighed |
| Reverse PWM | LPWM | GPIO 14 | ADC2_CH6 | PWM | Baglæns hastighed |
| Forward Enable | R_EN | GPIO 18 | - | Digital Out | Enable fremad (flyttet fra GPIO 12) |
| Reverse Enable | L_EN | GPIO 13 | ADC2_CH4 | Digital Out | Enable baglæns |
| Forward Current | R_IS | GPIO 36 (VP) | ADC1_CH0 | Analog In | Strømsensor fremad (input-only) |
| Reverse Current | L_IS | GPIO 39 (VN) | ADC1_CH3 | Analog In | Strømsensor baglæns (input-only) |
| **Venstre Sensor (HC-SR04)** |
| Trigger | TRIG | GPIO 15 | ADC2_CH3 | Digital Out | 10µs pulse |
| Echo | ECHO | GPIO 2 | ADC2_CH2 | Digital In | Distance measurement |
| **Midter Sensor (HC-SR04)** |
| Trigger | TRIG | GPIO 4 | ADC2_CH0 | Digital Out | 10µs pulse |
| Echo | ECHO | GPIO 16 | - | Digital In | Distance measurement |
| **Højre Sensor (HC-SR04)** |
| Trigger | TRIG | GPIO 17 | - | Digital Out | 10µs pulse |
| Echo | ECHO | GPIO 5 | - | Digital In | Distance measurement |
| **IMU (MPU-6050/9250)** |
| I2C Data | SDA | GPIO 21 | - | I2C | Standard I2C pin |
| I2C Clock | SCL | GPIO 22 | - | I2C | Standard I2C pin |
| **Klippermotor** |
| Relay Control | IN | GPIO 23 | - | Digital Out | HIGH = ON |
| **Batteri Monitor** |
| Voltage Sense | ADC | GPIO 19 | ADC2_CH8 | Analog In | Med voltage divider |
| **Status LED (optional)** |
| LED Output | LED | - | - | - | GPIO 18 bruges til motor enable |

**Note**: Display er IKKE i brug i denne ESP32-WROOM-32U version

---

## 📍 ESP32-WROOM-32U 38-Pin Layout

```
ESP32-WROOM-32U Development Board (38 pins)

Venstre Side (Top → Bund):          Højre Side (Top → Bund):
Pin 1  - GND                         Pin 1  - VIN (5V input)
Pin 2  - 3V3                         Pin 2  - GND
Pin 3  - EN (Reset)                  Pin 3  - GPIO23 ────► RELAY
Pin 4  - GPIO36 (VP) ─────► R_IS    Pin 4  - GPIO22 ────► SCL (IMU)
Pin 5  - GPIO39 (VN) ─────► L_IS    Pin 5  - GPIO1 (TX)
Pin 6  - GPIO34 ───────────► R_IS   Pin 6  - GPIO3 (RX)
Pin 7  - GPIO35 ───────────► L_IS   Pin 7  - GPIO21 ────► SDA (IMU)
Pin 8  - GPIO32 ───────────► RPWM   Pin 8  - GPIO19 ────► Battery ADC
Pin 9  - GPIO33 ───────────► LPWM   Pin 9  - GPIO18 ────► R_EN (Højre motor)
Pin 10 - GPIO25 ───────────► R_EN   Pin 10 - GPIO5 ─────► Echo Højre
Pin 11 - GPIO26 ───────────► L_EN   Pin 11 - GPIO17 ────► Trig Højre
Pin 12 - GPIO27 ───────────► RPWM   Pin 12 - GPIO16 ────► Echo Midter
Pin 13 - GPIO14 ───────────► LPWM   Pin 13 - GPIO4 ─────► Trig Midter
Pin 14 - GPIO12 (ikke brugt) ──────  Pin 14 - GPIO0 (Boot)
Pin 15 - GPIO13 ───────────► L_EN   Pin 15 - GPIO2 ─────► Echo Venstre
Pin 16 - GPIO15 ───────────► TRIG   Pin 16 - (Flash)
Pin 17 - GPIO10 (Flash)              Pin 17 - (Flash)
Pin 18 - GPIO9 (Flash)               Pin 18 - (Flash)
Pin 19 - GPIO8 (Flash)               Pin 19 - (Flash)

         ├─────── USB Port ──────┤
```

**⚠️ VIGTIGT**:
- GPIO 6-11: Reserved til SPI Flash - UNDGÅ disse!
- GPIO 34, 35, 36 (VP), 39 (VN): Input-only pins - perfekte til strømsensorer (IS pins)
- GPIO 0: Boot pin - bruges ikke i dette projekt
- GPIO 1, 3: TX/RX - reserveret til Serial kommunikation
- Standard I2C: GPIO 21 (SDA), GPIO 22 (SCL)

---

## 🔌 Detaljerede Forbindelser

### Motor Driver (BTS7960) Forbindelser

```
Venstre Motor Driver (Pins grupperet for nem ledningsføring):
────────────────────────────────────────────────────────────────
ESP32 Pin             BTS7960 Pin         Function
────────────────────────────────────────────────────────────────
GPIO 32          →    RPWM                PWM til fremad kørsel
GPIO 33          →    LPWM                PWM til baglæns kørsel
GPIO 25          →    R_EN                Enable fremad side
GPIO 26          →    L_EN                Enable baglæns side
GPIO 34 (IN)     →    R_IS                Strømsensor fremad (analog)
GPIO 35 (IN)     →    L_IS                Strømsensor baglæns (analog)

3.3V             →    VCC                 Logic power
GND              →    GND                 Ground

18V+ (5S LiPo)   →    B+                  Motor power +
18V- (5S LiPo)   →    B-                  Motor power -
                      M+, M-         →    Venstre motor +/-

Højre Motor Driver (Pins grupperet for nem ledningsføring):
────────────────────────────────────────────────────────────────
ESP32 Pin             BTS7960 Pin         Function
────────────────────────────────────────────────────────────────
GPIO 27          →    RPWM                PWM til fremad kørsel
GPIO 14          →    LPWM                PWM til baglæns kørsel
GPIO 18          →    R_EN                Enable fremad side (flyttet fra GPIO 12)
GPIO 13          →    L_EN                Enable baglæns side
GPIO 36 (VP,IN)  →    R_IS                Strømsensor fremad (analog)
GPIO 39 (VN,IN)  →    L_IS                Strømsensor baglæns (analog)

3.3V             →    VCC                 Logic power
GND              →    GND                 Ground

18V+ (5S LiPo)   →    B+                  Motor power +
18V- (5S LiPo)   →    B-                  Motor power -
                      M+, M-         →    Højre motor +/-
```

**Motor Kontrol Logik:**
| RPWM | LPWM | R_EN | L_EN | Resultat |
|------|------|------|------|----------|
| 0 | 0 | HIGH | HIGH | Stop |
| 0-255 | 0 | HIGH | HIGH | Fremad (variabel hastighed) |
| 0 | 0-255 | HIGH | HIGH | Baglæns (variabel hastighed) |
| X | X | LOW | LOW | Disabled (emergency stop) |

**⚠️ VIGTIGT**:
- R_EN og L_EN skal være HIGH for at enable driveren
- ALDRIG sæt både RPWM og LPWM højt samtidigt!
- Current sense (IS) pins giver 10mV/A output
- BTS7960 kan håndtere op til 43A kontinuerligt
- GPIO 34, 35, 36, 39 er input-only - perfekte til strømsensorer

---

### Ultralyd Sensorer (HC-SR04)

```
Venstre Sensor:
────────────────────────────────────
VCC              →    5V
TRIG             →    GPIO 15
ECHO             →    GPIO 2
GND              →    GND

Midter Sensor:
────────────────────────────────────
VCC              →    5V
TRIG             →    GPIO 4
ECHO             →    GPIO 16
GND              →    GND

Højre Sensor:
────────────────────────────────────
VCC              →    5V
TRIG             →    GPIO 17
ECHO             →    GPIO 5
GND              →    GND
```

**⚠️ VIGTIGT**:
- HC-SR04 sensorer kræver typisk 5V på VCC
- ECHO pin output er 5V - ESP32 GPIO er 5V tolerant
- TRIG pin kræver 10µs puls for at starte måling
- Sensorer er grupperet på tilstødende pins for nem ledningsføring

---

### IMU (MPU-6050 / MPU-9250)

```
MPU Pin         ESP32 Pin            Function
─────────────────────────────────────────────────────
VCC         →   3.3V                 Power (IKKE 5V!)
GND         →   GND                  Ground
SCL         →   GPIO 22              I2C Clock (standard)
SDA         →   GPIO 21              I2C Data (standard)
AD0         →   GND                  I2C Address (0x68)
INT         →   (not connected)      Interrupt (optional)
```

**⚠️ ADVARSEL**: MPU-6050/9250 er **IKKE** 5V tolerant! Brug kun 3.3V!

**I2C Address:**
- AD0 = GND → Address 0x68
- AD0 = VCC → Address 0x69

---

### Relay Modul (Klippermotor)

```
Relay Module:
────────────────────────────────────
VCC              →    5V
GND              →    GND
IN               →    GPIO 23

Relay Kontakter:
────────────────────────────────────
COM              →    Batteri + (18V fra 5S LiPo)
NO (Normally Open) → Klippermotor +
Klippermotor -   →    Batteri - (GND)
```

**Relay Logic:**
- GPIO 23 = LOW → Relay OFF → Motor OFF
- GPIO 23 = HIGH → Relay ON → Motor ON

---

### Batteri Monitoring (Voltage Divider)

**⚠️ VIGTIGT**: ESP32-WROOM-32U kræver ekstern voltage divider for batteri monitoring!

```
Batteri + (12V fra 3S LiPo)
     │
     ├─── [R1: 10kΩ] ───┬─── GPIO 19 (ADC2_CH8)
     │                  │
                   [R2: 2.2kΩ]
                        │
                       GND
```

**Beregning:**
```
Formel:
VBAT = (R1 + R2) / R2 × VADC
VBAT = (10000 + 2200) / 2200 × VADC
VBAT = 5.545 × VADC

Eksempler:
Med 12.6V batteri → VADC = 12.6 / 5.545 = 2.27V (under 3.3V max ✓)
Med 11.1V batteri → VADC = 11.1 / 5.545 = 2.00V
Med 10.0V batteri → VADC = 10.0 / 5.545 = 1.80V

I koden:
float adcVoltage = analogRead(BATTERY_PIN) * (3.3 / 4095.0);
float batteryVoltage = adcVoltage * 5.545;  // eller 5.55 for afrunding
```

**⚠️ KRITISK**:
- BRUG voltage divider for at beskytte ESP32 ADC pin!
- Max ADC input: 3.3V
- Max batteri spænding: ~18V med denne divider giver 3.25V på ADC
- GPIO 19 er ADC2_CH8 - undgå at bruge samtidig med WiFi hvis muligt

---

### Status LED (Optional)

```
LED forbindelse:
────────────────────────────────────
GPIO 18      →   [220Ω] → LED+ (anode)
LED- (kathode) → GND
```

**Note**:
- Ekstern LED er optional
- Brug 220Ω-330Ω modstand i serie
- GPIO 18 = HIGH → LED ON
- GPIO 18 = LOW → LED OFF

---

## ⚡ Power Distribution

```
                    5S LiPo Battery (18.5V - 21V)    3S LiPo Battery (11.1V - 12.6V)
                    [Motor Power]                    [Control Power]
                              │                              │
                              ├──────────────┐              │
                              │              │              │
                    BTS7960 Venstre   BTS7960 Højre   Buck Converter
                    Motor Driver      Motor Driver    (12V → 5V, 3A)
                    (18V input)       (18V input)           │
                              │              │              │
                    ┌─────────┴──────┐       │              │
                    │                │       │              │
              Venstre Motor    Højre Motor   │              │
                (18V DC)         (18V DC)    │              │
                                             │              │
              Current Sense (R_IS, L_IS)     │              │
              → GPIO 34,35,36,39 (ADC)       │              │
                                             │              │
                    ┌────────────────────────┼──────────────┤
                    │                        │              │
              Relay Module             Klippermotor   ESP32 VIN (5V)
              (5V input)              (18V via relay)       │
                    │                        │         ┌────┴─────┐
                    │                        │         │          │
              Relay Kontakt ─────────────────┘   Sensorer    IMU (3.3V)
              (18V switching)                   HC-SR04 x3   MPU-6050/9250
                                                   (5V)
```

**⚠️ KRITISK VIGTIGT**:
- **ADSKILT STRØMFORSYNING**: Motorer (18V) og kontrol (12V→5V) er SEPARATE!
- **FÆLLES GND**: Alle GND skal forbindes sammen (både 18V og 12V system)
- BTS7960 drivere får 18V power fra 5S LiPo
- BTS7960 logic (VCC) forbindes til 3.3V fra ESP32
- Buck converter skal levere minimum 3A ved 5V
- ESP32 VIN pin kan tage 5V input (intern regulator til 3.3V)
- Klippermotor bruger også 18V power via relay

---

## 🔒 GPIO Restrictions (ESP32-WROOM-32)

### ⛔ Undgå Disse Pins!
- **GPIO 6-11**: Forbundet til SPI Flash - UNDGÅ!
- **GPIO 1, 3**: UART TX/RX - reserveret til Serial kommunikation

### ⚠️ Strapping Pins (pas på!)
- **GPIO 0**: Boot mode (hold HIGH eller floating ved normal drift)
- **GPIO 2**: Boot mode (må ikke have pullup ved boot hvis flash er 3.3V)
- **GPIO 12**: Flash voltage (IKKE BRUGT i dette projekt - undgås pga. strapping konflikter)
- **GPIO 15**: Boot mode (hold HIGH ved boot)

### 📥 Input-Only Pins
- **GPIO 34-39**: Input only (ingen internal pullup/pulldown)
- Perfekte til ADC sensorer som strømmåling (IS pins)
- Kan IKKE bruges som outputs

### ✅ Safe GPIO Pins til General Use
- GPIO 2, 4, 5, 12-19, 21-23, 25-27, 32-33
- GPIO 34-39 (kun input)

### 🔌 ADC Channels
**ADC1** (kan bruges med WiFi):
- GPIO 32-39 (8 channels)

**ADC2** (undgå hvis WiFi bruges):
- GPIO 0, 2, 4, 12-15, 25-27 (10 channels)
- **OBS**: ADC2 kan ikke bruges når WiFi er aktivt!

---

## 📊 Pin Usage Summary

| Total GPIOs Used | 22 |
|------------------|-----|
| Digital Out | 9 (Motor EN pins, Relay, LED) |
| Digital In | 3 (Sensor ECHO pins) |
| PWM Out | 4 (Motor PWM) |
| Digital Out (TRIG) | 3 (Sensor TRIG) |
| I2C | 2 (SDA, SCL) |
| ADC (Current Sense) | 4 (Motor IS pins) |
| ADC (Battery) | 1 |
| **Available** | **~16** (excluding flash/boot pins) |

---

## 🎨 Pin Gruppering (For Nem Ledningsføring)

Pins er designet til at gruppere hver komponent på tilstødende fysiske pins:

**Venstre Motor Gruppe** (GPIO 25, 26, 32-35):
- Alle 6 pins er placeret nær hinanden på boardet
- Kort ledningsføring fra ESP32 til BTS7960

**Højre Motor Gruppe** (GPIO 13-14, 18, 27, 36, 39):
- Så tæt grupperet som muligt
- GPIO 18 bruges i stedet for GPIO 12 (undgår strapping pin konflikt)
- GPIO 36 og 39 er på modsatte side, men stadig ADC1 channels

**Sensor Gruppe** (GPIO 2, 4, 5, 15-17):
- Alle sensor pins grupperet sammen
- Nem routing til de 3 ultralyd sensorer

**I2C Gruppe** (GPIO 21-22):
- Standard I2C pins
- Kun IMU på bussen (display ikke i brug)

---

## 🔧 Custom Pin Configuration

For at ændre pins, rediger `src/config/Config.h`:

```cpp
// Motor Pins (BTS7960)
// Venstre motor driver
#define MOTOR_LEFT_RPWM     32     // PWM til fremad
#define MOTOR_LEFT_LPWM     33     // PWM til baglæns
#define MOTOR_LEFT_R_EN     25     // Enable fremad
#define MOTOR_LEFT_L_EN     26     // Enable baglæns
#define MOTOR_LEFT_R_IS     34     // Strømsensor fremad (ADC)
#define MOTOR_LEFT_L_IS     35     // Strømsensor baglæns (ADC)

// Højre motor driver
#define MOTOR_RIGHT_RPWM    27     // PWM til fremad
#define MOTOR_RIGHT_LPWM    14     // PWM til baglæns
#define MOTOR_RIGHT_R_EN    18     // Enable fremad (flyttet fra GPIO 12)
#define MOTOR_RIGHT_L_EN    13     // Enable baglæns
#define MOTOR_RIGHT_R_IS    36     // Strømsensor fremad (ADC)
#define MOTOR_RIGHT_L_IS    39     // Strømsensor baglæns (ADC)

// Sensorer
#define SENSOR_LEFT_TRIG    15
#define SENSOR_LEFT_ECHO    2
#define SENSOR_MIDDLE_TRIG  4
#define SENSOR_MIDDLE_ECHO  16
#define SENSOR_RIGHT_TRIG   17
#define SENSOR_RIGHT_ECHO   5

// IMU (Standard I2C)
#define IMU_SDA             21
#define IMU_SCL             22

// Relay og Batteri
#define CUTTING_RELAY       23
#define BATTERY_PIN         19
```

Efter ændring, opdater dine fysiske forbindelser tilsvarende!

---

## 🧪 Testing Pin Forbindelser

### Test Procedure

1. **Visual Inspection**: Tjek alle forbindelser visuelt
2. **Continuity Test**: Brug multimeter til at tjekke forbindelser
3. **Voltage Test**: Mål spændinger med multimeter:
   - 5V rail = 4.8-5.2V
   - 3.3V rail = 3.2-3.4V
   - Batteri = 11.1-12.6V (3S LiPo)
   - Motor batteri = 18.5-21V (5S LiPo)
4. **Individual Component Test**: Test hver komponent separat først

### Common Issues

**Motor ikke kører:**
- Tjek PWM forbindelse (GPIO 32, 33, 27, 14)
- Verificér R_EN/L_EN forbindelser
- Check motor driver power (18V)
- Verificér logic power (3.3V til VCC)

**Sensor læser 0:**
- Tjek TRIG forbindelse
- Tjek ECHO forbindelse
- Verificér sensor power (5V)

**IMU virker ikke:**
- Tjek I2C forbindelser (GPIO 21 SDA, GPIO 22 SCL)
- Verificér 3.3V power (IKKE 5V!)
- Scan I2C bus for device address (0x68 eller 0x69)
- Brug `i2cdetect` kommando

**Batteri læser forkert:**
- Verificér voltage divider (10kΩ og 2.2kΩ)
- Tjek ADC forbindelse (GPIO 19)
- Test divider output med multimeter
- Skal være under 3.3V ved max batteri spænding

---

## 📸 Wiring Diagram

```
                    ┌─────────────────────────┐
                    │  ESP32-WROOM-32U        │
                    │   38-Pin Dev Board      │
                    └──────────┬──────────────┘
                               │
        ┌──────────────────────┼──────────────────────┐
        │                      │                      │
   ┌────▼─────┐        ┌───────▼────────┐      ┌────▼────┐
   │ BTS7960  │        │  HC-SR04 x3    │      │ MPU6050 │
   │ Motor    │        │   Sensors      │      │   IMU   │
   │ Drivers  │        └────────────────┘      └─────────┘
   │  (x2)    │
   └────┬─────┘
        │
   ┌────▼────┐
   │ Motors  │
   │  (2x)   │
   └─────────┘
```

---

## 🆚 Forskelle fra Heltec WiFi Kit 32 V3

| Feature | Heltec WiFi Kit 32 V3 | ESP32-WROOM-32U |
|---------|----------------------|-----------------|
| **Chip** | ESP32-S3 | ESP32 (original) |
| **Display** | Indbygget OLED 128x64 | Ingen display |
| **I2C** | GPIO 41, 42 | GPIO 21, 22 (standard) |
| **Flash Pins** | GPIO 6-11 (S3) | GPIO 6-11 (undgå) |
| **Input-Only** | GPIO 35-39 | GPIO 34-39 |
| **ADC** | ADC1 kun | ADC1 + ADC2 |
| **Display Power** | GPIO 36 (Vext) | N/A |
| **Battery ADC** | GPIO 1 (indbygget divider) | GPIO 19 (ekstern divider) |

**Fordele ved ESP32-WROOM-32U:**
- Billigere
- Mere standard pin layout
- Veldokumenteret
- Bred support

**Ulemper:**
- Ingen indbygget display
- Kræver ekstern voltage divider til batteri
- ADC2 kan ikke bruges med WiFi

---

**For fysisk montering, se også:**
- HARDWARE.md for komponent detaljer
- SETUP.md for trin-for-trin guide
- README.md for projekt oversigt

