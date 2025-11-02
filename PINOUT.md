# 📌 Pin Forbindelser Diagram

Komplet pin mapping for Robot Plæneklipper projektet.

## 🎯 Quick Reference Table

| Component | Pin | ESP32-S3 GPIO | Type | Notes |
|-----------|-----|---------------|------|-------|
| **Venstre Motor (BTS7960)** |
| Forward PWM | RPWM | GPIO 5 | PWM | Fremad hastighed |
| Reverse PWM | LPWM | GPIO 19 | PWM | Baglæns hastighed |
| Forward Enable | R_EN | GPIO 18 | Digital Out | Enable fremad |
| Reverse Enable | L_EN | GPIO 17 | Digital Out | Enable baglæns |
| Forward Current | R_IS | GPIO 2 | Analog In | Strømsensor fremad |
| Reverse Current | L_IS | GPIO 3 | Analog In | Strømsensor baglæns |
| **Højre Motor (BTS7960)** |
| Forward PWM | RPWM | GPIO 16 | PWM | Fremad hastighed |
| Reverse PWM | LPWM | GPIO 15 | PWM | Baglæns hastighed |
| Forward Enable | R_EN | GPIO 4 | Digital Out | Enable fremad |
| Reverse Enable | L_EN | GPIO 6 | Digital Out | Enable baglæns |
| Forward Current | R_IS | GPIO 7 | Analog In | Strømsensor fremad |
| Reverse Current | L_IS | GPIO 8 | Analog In | Strømsensor baglæns |
| **Venstre Sensor** |
| Trigger | TRIG | GPIO 21 | Digital Out | 10µs pulse |
| Echo | ECHO | GPIO 47 | Digital In | Distance measurement |
| **Midter Sensor** |
| Trigger | TRIG | GPIO 48 | Digital Out | 10µs pulse |
| Echo | ECHO | GPIO 35 | Digital In | Distance measurement |
| **Højre Sensor** |
| Trigger | TRIG | GPIO 36 | Digital Out | 10µs pulse |
| Echo | ECHO | GPIO 37 | Digital In | Distance measurement |
| **IMU (MPU-6050/9250)** |
| I2C Data | SDA | GPIO 41 | I2C | Delt med display |
| I2C Clock | SCL | GPIO 42 | I2C | Delt med display |
| **Klippermotor** |
| Relay Control | IN | GPIO 38 | Digital Out | HIGH = ON |
| **Batteri Monitor** |
| Voltage Sense | ADC | GPIO 1 | Analog In | Via voltage divider |
| **Display (OLED)** |
| I2C Data | SDA | GPIO 41 | I2C | Indbygget i Heltec |
| I2C Clock | SCL | GPIO 42 | I2C | Indbygget i Heltec |
| Power Control | Vext | GPIO 36 | Digital Out | LOW = ON (Heltec) |
| **Status LED** |
| Builtin LED | - | GPIO 35 | Digital Out | Indbygget i Heltec |

---

## 🔌 Detaljerede Forbindelser

### Motor Driver (BTS7960) Forbindelser

```
Venstre Motor Driver:
────────────────────────────────────────────────────────────────
ESP32-S3 Pin          BTS7960 Pin         Function
────────────────────────────────────────────────────────────────
GPIO 5           →    RPWM                PWM til fremad kørsel
GPIO 19          →    LPWM                PWM til baglæns kørsel
GPIO 18          →    R_EN                Enable fremad side
GPIO 17          →    L_EN                Enable baglæns side
GPIO 2           →    R_IS                Strømsensor fremad (analog)
GPIO 3           →    L_IS                Strømsensor baglæns (analog)

3.3V             →    VCC                 Logic power
GND              →    GND                 Ground

18V+ (5S LiPo)   →    B+                  Motor power +
18V- (5S LiPo)   →    B-                  Motor power -
                      M+, M-         →    Venstre motor +/-

Højre Motor Driver:
────────────────────────────────────────────────────────────────
ESP32-S3 Pin          BTS7960 Pin         Function
────────────────────────────────────────────────────────────────
GPIO 16          →    RPWM                PWM til fremad kørsel
GPIO 15          →    LPWM                PWM til baglæns kørsel
GPIO 4           →    R_EN                Enable fremad side
GPIO 6           →    L_EN                Enable baglæns side
GPIO 7           →    R_IS                Strømsensor fremad (analog)
GPIO 8           →    L_IS                Strømsensor baglæns (analog)

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

---

### Ultralyd Sensorer (HC-SR04)

```
Venstre Sensor:
────────────────────────
VCC              →    5V              (eller 3.3V)
TRIG             →    GPIO 21
ECHO             →    GPIO 47
GND              →    GND

Midter Sensor:
────────────────────────
VCC              →    5V
TRIG             →    GPIO 48
ECHO             →    GPIO 35
GND              →    GND

Højre Sensor:
────────────────────────
VCC              →    5V
TRIG             →    GPIO 36
ECHO             →    GPIO 37
GND              →    GND
```

**⚠️ VIGTIGT**:
- Nogle HC-SR04 sensorer kræver 5V på VCC
- ECHO pin output er 5V - brug voltage divider eller 3.3V tolerant pins
- På ESP32-S3 er de fleste pins 5V tolerant

---

### IMU (MPU-6050 / MPU-9250)

```
MPU Pin         ESP32-S3 Pin       Function
───────────────────────────────────────────────
VCC         →   3.3V               Power (IKKE 5V!)
GND         →   GND                Ground
SCL         →   GPIO 42            I2C Clock
SDA         →   GPIO 41            I2C Data
AD0         →   GND                I2C Address (0x68)
INT         →   (not connected)    Interrupt (optional)
```

**⚠️ ADVARSEL**: MPU-6050/9250 er **IKKE** 5V tolerant! Brug kun 3.3V!

**I2C Address:**
- AD0 = GND → Address 0x68
- AD0 = VCC → Address 0x69

---

### Relay Modul (Klippermotor)

```
Relay Module:
────────────────────────────────
VCC              →    5V
GND              →    GND
IN               →    GPIO 38

Relay Kontakter:
────────────────────────────────
COM              →    Batteri + (12V)
NO (Normally Open) → Klippermotor +
Klippermotor -   →    Batteri - (GND)
```

**Relay Logic:**
- GPIO 38 = LOW → Relay OFF → Motor OFF
- GPIO 38 = HIGH → Relay ON → Motor ON

---

### Batteri Monitoring (Voltage Divider)

```
Batteri + (12.6V max)
     │
     ├─── [R1: 10kΩ] ───┬─── GPIO 1 (ADC)
     │                  │
                   [R2: 2.2kΩ]
                        │
                       GND
```

**Beregning:**
```
Vout = Vin × R2 / (R1 + R2)

Med 12.6V input:
Vout = 12.6 × 2200 / (10000 + 2200)
Vout = 12.6 × 0.1803 = 2.27V ✓ (under 3.3V max)

Med 11.1V input (nominal):
Vout = 11.1 × 0.1803 = 2.00V

Med 9.0V input (minimum):
Vout = 9.0 × 0.1803 = 1.62V
```

**⚠️ KRITISK**: Verificér at Vout ALDRIG overstiger 3.3V!

---

### OLED Display (Indbygget)

Display er indbygget i Heltec WiFi Kit 32 V3:

```
Display         ESP32-S3 (intern)
────────────────────────────────
SDA         →   GPIO 41          (hardware defineret)
SCL         →   GPIO 42          (hardware defineret)
Vext        →   GPIO 36          (power control - LOW = ON)
RST         →   -                (ikke brugt)
```

**⚠️ VIGTIGT**:
- På Heltec V3 skal Vext pin sættes LOW for at aktivere display power
- Display deler I2C bus med IMU - begge devices skal have unikke adresser
- Uden Vext aktivering vil displayet ikke få strøm!

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
#define MOTOR_LEFT_RPWM     5      // PWM til fremad
#define MOTOR_LEFT_LPWM     19     // PWM til baglæns
#define MOTOR_LEFT_R_EN     18     // Enable fremad
#define MOTOR_LEFT_L_EN     17     // Enable baglæns
#define MOTOR_LEFT_R_IS     2      // Strømsensor fremad (ADC)
#define MOTOR_LEFT_L_IS     3      // Strømsensor baglæns (ADC)

// Højre motor driver
#define MOTOR_RIGHT_RPWM    16     // PWM til fremad
#define MOTOR_RIGHT_LPWM    15     // PWM til baglæns
#define MOTOR_RIGHT_R_EN    4      // Enable fremad
#define MOTOR_RIGHT_L_EN    6      // Enable baglæns
#define MOTOR_RIGHT_R_IS    7      // Strømsensor fremad (ADC)
#define MOTOR_RIGHT_L_IS    8      // Strømsensor baglæns (ADC)

// Display
#define DISPLAY_VEXT        36     // Vext On (LOW = power on)
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
   ┌────▼────┐          ┌──────▼──────┐        ┌────▼────┐
   │ L298N   │          │  HC-SR04 x3 │        │ MPU6050 │
   │ Motor   │          │   Sensors   │        │   IMU   │
   │ Driver  │          └─────────────┘        └─────────┘
   └────┬────┘
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

