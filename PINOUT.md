# 📌 Pin Forbindelser Diagram

Komplet pin mapping for Robot Plæneklipper projektet.

## 🎯 Quick Reference Table

| Component | Function | Header | Pin# | GPIO# | Type | Notes |
|-----------|----------|--------|------|-------|------|-------|
| **Venstre Motor (BTS7960)** |
| Forward PWM | RPWM | J2 | 14 | GPIO 5 | PWM | Fremad hastighed |
| Reverse PWM | LPWM | J2 | 11 | GPIO 19 | PWM | Baglæns hastighed |
| Forward Enable | R_EN | J3 | 4 | GPIO 26 | Digital Out | Enable fremad |
| Reverse Enable | L_EN | J3 | 7 | GPIO 33 | Digital Out | Enable baglæns |
| Forward Current | R_IS | J2 | 17 | GPIO 2 | Analog In | Strømsensor fremad |
| Reverse Current | L_IS | J2 | 16 | GPIO 3 | Analog In | Strømsensor baglæns |
| **Højre Motor (BTS7960)** |
| Forward PWM | RPWM | J3 | 8 | GPIO 34 | PWM | Fremad hastighed |
| Reverse PWM | LPWM | J2 | 10 | GPIO 20 | PWM | Baglæns hastighed |
| Forward Enable | R_EN | J2 | 15 | GPIO 4 | Digital Out | Enable fremad |
| Reverse Enable | L_EN | J2 | 13 | GPIO 6 | Digital Out | Enable baglæns |
| Forward Current | R_IS | J2 | 12 | GPIO 7 | Analog In | Strømsensor fremad |
| Reverse Current | L_IS | J3 | 11 | GPIO 37 | Analog In | Strømsensor baglæns |
| **Venstre Sensor** |
| Trigger | TRIG | J2 | 9 | GPIO 21 | Digital Out | 10µs pulse |
| Echo | ECHO | J3 | 6 | GPIO 47 | Digital In | Distance measurement |
| **Midter Sensor** |
| Trigger | TRIG | J3 | 5 | GPIO 48 | Digital Out | 10µs pulse |
| Echo | ECHO | J3 | 9 | GPIO 35 | Digital In | Distance measurement |
| **Højre Sensor** |
| Trigger | TRIG | J3 | 13 | GPIO 39 | Digital Out | 10µs pulse |
| Echo | ECHO | J3 | 14 | GPIO 40 | Digital In | Distance measurement |
| **IMU (MPU-6050/9250)** |
| I2C Data | SDA | J3 | 15 | GPIO 41 | I2C | Delt med display |
| I2C Clock | SCL | J3 | 16 | GPIO 42 | I2C | Delt med display |
| **Klippermotor** |
| Relay Control | IN | J3 | 12 | GPIO 38 | Digital Out | HIGH = ON |
| **Batteri Monitor** |
| Voltage Sense | ADC | J2 | 18 | GPIO 1 | Analog In | Indbygget voltage divider |
| **Display (OLED)** |
| I2C Data | SDA | J3 | 15 | GPIO 41 | I2C | Indbygget i Heltec |
| I2C Clock | SCL | J3 | 16 | GPIO 42 | I2C | Indbygget i Heltec |
| Power Control | Vext | J3 | 10 | GPIO 36 | Digital Out | LOW = ON (Heltec) |
| **Status LED** |
| Builtin LED | - | J3 | 9 | GPIO 35 | Digital Out | Indbygget i Heltec |

**Note**: Header (J2/J3) og Pin# refererer til fysiske locations på Heltec WiFi Kit 32 V3 board jf. databladet

---

## 📍 Heltec WiFi Kit 32 V3 - Pin Layout

**Jf. Heltec Official Datasheet Rev 1.1**

```
Heltec WiFi Kit 32 V3 har to separate headers:

╔══════════════════════════════════════════════════════════════════╗
║                    HELTEC WiFi Kit 32 V3                         ║
║                     ESP32-S3 + OLED Display                      ║
╚══════════════════════════════════════════════════════════════════╝

Header J2 (Venstre side):              Header J3 (Højre side):
Pin 1  - GND                           Pin 1  - GND
Pin 2  - 5V                            Pin 2  - 3V3
Pin 3  - Ve (3.3V output)              Pin 3  - 3V3
Pin 4  - Ve (3.3V output)              Pin 4  - GPIO 26
Pin 5  - RX (GPIO 44) [USB]            Pin 5  - GPIO 48 (TRIG Midter)
Pin 6  - TX (GPIO 43) [USB]            Pin 6  - GPIO 47 (ECHO Venstre)
Pin 7  - RST (CHIP_PU)                 Pin 7  - GPIO 33
Pin 8  - 0 (GPIO 0) [Boot]             Pin 8  - GPIO 34 (R Motor RPWM)
Pin 9  - GPIO 21 (OLED RST/TRIG V)     Pin 9  - GPIO 35 (LED/ECHO Midter)
Pin 10 - GPIO 20 (R Motor LPWM)        Pin 10 - GPIO 36 (Vext Ctrl)
Pin 11 - GPIO 19 (L Motor LPWM)        Pin 11 - GPIO 37 (R Motor L_IS)
Pin 12 - GPIO 7  (R Motor R_IS)        Pin 12 - GPIO 38 (Relay)
Pin 13 - GPIO 6  (R Motor L_EN)        Pin 13 - GPIO 39 (R Sensor TRIG)
Pin 14 - GPIO 5  (L Motor RPWM)        Pin 14 - GPIO 40 (R Sensor ECHO)
Pin 15 - GPIO 4  (R Motor R_EN)        Pin 15 - GPIO 41 (SDA)
Pin 16 - GPIO 3  (L Motor L_IS)        Pin 16 - GPIO 42 (SCL)
Pin 17 - GPIO 2  (L Motor R_IS)        Pin 17 - GPIO 45
Pin 18 - GPIO 1  (Battery ADC)         Pin 18 - GPIO 46
```

**⚠️ VIGTIGT**:
- Header J2 og J3 er fysiske connectorer på boardet
- Pin numre er positioner på hver header (1-18)
- GPIO numre er de logiske pin numre brugt i koden
- Nogle GPIO er reserved: GPIO 43/44 (USB), GPIO 0 (Boot)
- OLED display bruger GPIO 41 (SDA), 42 (SCL) - indbygget
- Vext control (GPIO 36) styrer power til eksterne sensorer - LOW = ON
- Indbygget LED er på GPIO 35
- GPIO 1 (Pin J2-18) læser batterispænding via indbygget voltage divider

---

## 🔌 Detaljerede Forbindelser

### Motor Driver (BTS7960) Forbindelser

```
Venstre Motor Driver:
────────────────────────────────────────────────────────────────
ESP32-S3 Pin          BTS7960 Pin         Function
────────────────────────────────────────────────────────────────
GPIO 5  (J2-14)  →    RPWM                PWM til fremad kørsel
GPIO 19 (J2-11)  →    LPWM                PWM til baglæns kørsel
GPIO 26 (J3-4)   →    R_EN                Enable fremad side
GPIO 33 (J3-7)   →    L_EN                Enable baglæns side
GPIO 2  (J2-17)  →    R_IS                Strømsensor fremad (analog)
GPIO 3  (J2-16)  →    L_IS                Strømsensor baglæns (analog)

3.3V (J3-2/3)    →    VCC                 Logic power
GND (J2-1/J3-1)  →    GND                 Ground

18V+ (5S LiPo)   →    B+                  Motor power +
18V- (5S LiPo)   →    B-                  Motor power -
                      M+, M-         →    Venstre motor +/-

Højre Motor Driver:
────────────────────────────────────────────────────────────────
ESP32-S3 Pin          BTS7960 Pin         Function
────────────────────────────────────────────────────────────────
GPIO 34 (J3-8)   →    RPWM                PWM til fremad kørsel
GPIO 20 (J2-10)  →    LPWM                PWM til baglæns kørsel
GPIO 4  (J2-15)  →    R_EN                Enable fremad side
GPIO 6  (J2-13)  →    L_EN                Enable baglæns side
GPIO 7  (J2-12)  →    R_IS                Strømsensor fremad (analog)
GPIO 37 (J3-11)  →    L_IS                Strømsensor baglæns (analog)

3.3V (J3-2/3)    →    VCC                 Logic power
GND (J2-1/J3-1)  →    GND                 Ground

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

---

### Ultralyd Sensorer (HC-SR04)

```
Venstre Sensor:
────────────────────────────────────
VCC              →    5V (J2-2)
TRIG             →    GPIO 21 (J2-9)
ECHO             →    GPIO 47 (J3-6)
GND              →    GND (J2-1/J3-1)

Midter Sensor:
────────────────────────────────────
VCC              →    5V (J2-2)
TRIG             →    GPIO 48 (J3-5)
ECHO             →    GPIO 35 (J3-9)
GND              →    GND (J2-1/J3-1)

Højre Sensor:
────────────────────────────────────
VCC              →    5V (J2-2)
TRIG             →    GPIO 39 (J3-13)
ECHO             →    GPIO 40 (J3-14)
GND              →    GND (J2-1/J3-1)
```

**⚠️ VIGTIGT**:
- Nogle HC-SR04 sensorer kræver 5V på VCC
- ECHO pin output er 5V - brug voltage divider eller 3.3V tolerant pins
- På ESP32-S3 er de fleste pins 5V tolerant

---

### IMU (MPU-6050 / MPU-9250)

```
MPU Pin         ESP32-S3 Pin          Function
─────────────────────────────────────────────────────
VCC         →   3.3V (J3-2/3)        Power (IKKE 5V!)
GND         →   GND (J2-1/J3-1)      Ground
SCL         →   GPIO 42 (J3-16)      I2C Clock
SDA         →   GPIO 41 (J3-15)      I2C Data
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
VCC              →    5V (J2-2)
GND              →    GND (J2-1/J3-1)
IN               →    GPIO 38 (J3-12)

Relay Kontakter:
────────────────────────────────────
COM              →    Batteri + (12V)
NO (Normally Open) → Klippermotor +
Klippermotor -   →    Batteri - (GND)
```

**Relay Logic:**
- GPIO 38 = LOW → Relay OFF → Motor OFF
- GPIO 38 = HIGH → Relay ON → Motor ON

---

### Batteri Monitoring (Indbygget Voltage Divider)

**⚠️ VIGTIGT**: Heltec WiFi Kit 32 V3 har en **indbygget voltage divider** på GPIO 1!

```
Batteri + (Via Heltec's indbygget circuit)
     │
     ├─── [R1: 390Ω] ───┬─── GPIO 1 (J2-18, ADC1_CH0)
     │                  │
                   [R2: 100Ω]
                        │
                       GND
```

**Beregning (jf. Heltec datasheet):**
```
Formel fra databladet:
VBAT = (100 + 390) / 100 × VADC_IN
VBAT = 490 / 100 × VADC
VBAT = 4.9 × VADC

Eksempler:
Med 12.6V batteri → VADC = 12.6 / 4.9 = 2.57V (under 3.3V max ✓)
Med 11.1V batteri → VADC = 11.1 / 4.9 = 2.27V
Med 9.0V batteri  → VADC = 9.0 / 4.9 = 1.84V

I koden:
float adcVoltage = analogRead(BATTERY_ADC) * (3.3 / 4095.0);
float batteryVoltage = adcVoltage * 4.9;
```

**⚠️ KRITISK**:
- BRUG IKKE eksterne voltage dividere på GPIO 1!
- Heltec har allerede indbygget en divider
- Max batteri spænding: ~16V (giver 3.27V på ADC)
- GPIO 1 er specifikt designet til batteri monitoring

---

### OLED Display (Indbygget)

Display er indbygget i Heltec WiFi Kit 32 V3:

```
Display         ESP32-S3 Pin         Function
───────────────────────────────────────────────────────
SDA         →   GPIO 41 (J3-15)     I2C Data (hardware)
SCL         →   GPIO 42 (J3-16)     I2C Clock (hardware)
Vext        →   GPIO 36 (J3-10)     Power control - LOW = ON
RST         →   GPIO 21 (J2-9)      OLED Reset (optional)
```

**⚠️ VIGTIGT**:
- På Heltec V3 skal Vext pin (GPIO 36) sættes LOW for at aktivere display power
- Display deler I2C bus med IMU - begge devices skal have unikke adresser
- Uden Vext aktivering vil displayet ikke få strøm!
- GPIO 21 kan bruges som OLED reset hvis nødvendigt (men typisk ikke påkrævet)

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
              → GPIO 2,3,7,8 (ADC)           │              │
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

## 🔒 GPIO Restrictions (ESP32-S3)

### Strapping Pins (pas på!)
- GPIO 0 - Boot mode (hold LOW ved boot = download mode)
- GPIO 45 - VDD_SPI voltage select
- GPIO 46 - Boot mode

### Input Only Pins
- GPIO 35-39 - Input only (ingen internal pullup/pulldown)

### Reserved/Special Pins
- GPIO 19-20 - USB (brug ikke hvis USB skal virke)
- GPIO 43-44 - UART (Serial Monitor)

### Safe GPIO Pins til General Use
- GPIO 1-18
- GPIO 21
- GPIO 35-39 (input only)
- GPIO 47-48

---

## 📊 Pin Usage Summary

| Total GPIOs Used | 26 |
|------------------|-----|
| Digital Out | 9 |
| Digital In | 3 |
| PWM Out | 4 |
| I2C | 2 (shared) |
| ADC (Current Sense) | 4 |
| ADC (Battery) | 1 |
| Power Control | 1 |
| Relay | 1 |
| **Available** | **~6+** |

---

## 🔧 Custom Pin Configuration

For at ændre pins, rediger `config/Config.h`:

```cpp
// Motor Pins (BTS7960)
// Venstre motor driver
#define MOTOR_LEFT_RPWM     5      // J2-14 - PWM til fremad
#define MOTOR_LEFT_LPWM     19     // J2-11 - PWM til baglæns
#define MOTOR_LEFT_R_EN     26     // J3-4  - Enable fremad
#define MOTOR_LEFT_L_EN     33     // J3-7  - Enable baglæns
#define MOTOR_LEFT_R_IS     2      // J2-17 - Strømsensor fremad (ADC)
#define MOTOR_LEFT_L_IS     3      // J2-16 - Strømsensor baglæns (ADC)

// Højre motor driver
#define MOTOR_RIGHT_RPWM    34     // J3-8  - PWM til fremad
#define MOTOR_RIGHT_LPWM    20     // J2-10 - PWM til baglæns
#define MOTOR_RIGHT_R_EN    4      // J2-15 - Enable fremad
#define MOTOR_RIGHT_L_EN    6      // J2-13 - Enable baglæns
#define MOTOR_RIGHT_R_IS    7      // J2-12 - Strømsensor fremad (ADC)
#define MOTOR_RIGHT_L_IS    37     // J3-11 - Strømsensor baglæns (ADC)

// Sensorer
#define SENSOR_LEFT_TRIG    21     // J2-9
#define SENSOR_LEFT_ECHO    47     // J3-6
#define SENSOR_MID_TRIG     48     // J3-5
#define SENSOR_MID_ECHO     35     // J3-9
#define SENSOR_RIGHT_TRIG   39     // J3-13
#define SENSOR_RIGHT_ECHO   40     // J3-14

// Display og IMU (Indbygget i Heltec V3)
#define DISPLAY_SDA         41     // J3-15 (delt med IMU)
#define DISPLAY_SCL         42     // J3-16 (delt med IMU)
#define DISPLAY_VEXT        36     // J3-10 - Vext On (LOW = power on)

// Relay og Batteri
#define RELAY_PIN           38     // J3-12 - Klippermotor
#define BATTERY_ADC         1      // J2-18 - Battery voltage sense (indbygget divider)
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
   - Batteri = 11.1-12.6V
4. **Individual Component Test**: Test hver komponent separat først

### Common Issues

**Motor ikke kører:**
- Tjek PWM forbindelse
- Verificér IN1/IN2 forbindelser
- Check motor driver power

**Sensor læser 0:**
- Tjek TRIG forbindelse
- Tjek ECHO forbindelse
- Verificér sensor power (5V)

**IMU virker ikke:**
- Tjek I2C forbindelser (SDA/SCL)
- Verificér 3.3V power (IKKE 5V!)
- Scan I2C bus for device address

---

## 📸 Wiring Diagram

```
                    ┌─────────────────────┐
                    │  ESP32-S3 (Heltec)  │
                    │   WiFi Kit 32 V3    │
                    └──────────┬──────────┘
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

**For fysisk montering, se også:**
- HARDWARE.md for komponent detaljer
- SETUP.md for trin-for-trin guide
- README.md for projekt oversigt

