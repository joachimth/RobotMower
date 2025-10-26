# 📌 Pin Forbindelser Diagram

Komplet pin mapping for Robot Plæneklipper projektet.

## 🎯 Quick Reference Table

| Component | Pin | ESP32-S3 GPIO | Type | Notes |
|-----------|-----|---------------|------|-------|
| **Venstre Motor** |
| PWM | ENA | GPIO 5 | PWM | Motor hastighed |
| Direction 1 | IN1 | GPIO 19 | Digital Out | Retning kontrol |
| Direction 2 | IN2 | GPIO 18 | Digital Out | Retning kontrol |
| **Højre Motor** |
| PWM | ENB | GPIO 17 | PWM | Motor hastighed |
| Direction 1 | IN3 | GPIO 16 | Digital Out | Retning kontrol |
| Direction 2 | IN4 | GPIO 15 | Digital Out | Retning kontrol |
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
| **Status LED** |
| Builtin LED | - | GPIO 35 | Digital Out | Indbygget i Heltec |

---

## 🔌 Detaljerede Forbindelser

### Motor Driver (L298N) Forbindelser

```
ESP32-S3 Pin          L298N Pin           Function
───────────────────────────────────────────────────────
GPIO 5           →    ENA                 PWM - Venstre hastighed
GPIO 19          →    IN1                 Venstre retning bit 1
GPIO 18          →    IN2                 Venstre retning bit 2

GPIO 17          →    ENB                 PWM - Højre hastighed
GPIO 16          →    IN3                 Højre retning bit 1
GPIO 15          →    IN4                 Højre retning bit 2

GND              →    GND                 Fælles ground

                      OUT1, OUT2     →    Venstre motor +/-
                      OUT3, OUT4     →    Højre motor +/-

Batteri +        →    +12V                Motor power
Batteri -        →    GND                 Ground
```

**Motor Retning Logik:**
| IN1 | IN2 | Retning |
|-----|-----|---------|
| LOW | LOW | Stop |
| HIGH | LOW | Fremad |
| LOW | HIGH | Baglæns |
| HIGH | HIGH | Brake |

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
RST         →   -                (ikke brugt)
```

**⚠️ Note**: Display deler I2C bus med IMU - begge devices skal have unikke adresser.

---

## ⚡ Power Distribution

```
                    3S LiPo Battery (11.1V - 12.6V)
                              │
                              ├─────────────────────────┐
                              │                         │
                        L298N Motor Driver        Buck Converter
                         (12V input)               (12V → 5V, 3A)
                              │                         │
                    ┌─────────┴──────────┐             │
                    │                    │             │
              Venstre Motor         Højre Motor        │
                (12V DC)              (12V DC)         │
                                                        │
                    ┌───────────────────────────────────┤
                    │                                   │
              Relay Module                        ESP32 VIN (5V)
              (5V input)                                │
                    │                                   │
              Klippermotor                        ┌─────┴─────┐
              (12V DC via relay)                  │           │
                                            Sensorer (5V)   IMU (3.3V)
                                            HC-SR04 x3      MPU-6050/9250
```

**⚠️ VIGTIGT**:
- Fælles GND mellem ALLE komponenter!
- Buck converter skal levere minimum 3A ved 5V
- ESP32 VIN pin kan tage 5V input (intern regulator til 3.3V)

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

| Total GPIOs Used | 18 |
|------------------|-----|
| Digital Out | 10 |
| Digital In | 3 |
| PWM Out | 2 |
| I2C | 2 (shared) |
| ADC | 1 |
| **Available** | **~12+** |

---

## 🔧 Custom Pin Configuration

For at ændre pins, rediger `config/Config.h`:

```cpp
// Motor Pins
#define MOTOR_LEFT_PWM      5      // PWM
#define MOTOR_LEFT_IN1      19     // Digital
#define MOTOR_LEFT_IN2      18     // Digital
// ... etc
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

