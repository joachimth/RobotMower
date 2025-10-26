# 🔧 Hardware Specifikationer

Komplet hardware liste for Robot Plæneklipper projektet.

## 📦 Hovedkomponenter

### 1. Microcontroller
- **Heltec WiFi Kit 32 V3**
  - ESP32-S3 chip
  - WiFi + Bluetooth
  - Indbygget OLED display (128x64)
  - 8MB Flash
  - USB-C forbindelse
  - [Køb her](https://heltec.org/project/wifi-kit-32-v3/)
  - Pris: ~$15-20

### 2. Motor Driver
- **L298N Dual H-Bridge Motor Driver**
  - 2 kanaler
  - Max 2A per kanal
  - PWM hastigheds kontrol
  - Indbygget 5V regulator
  - Pris: ~$3-5

### 3. Drive Motors
- **2x DC Gear Motors**
  - 12V, 100-200 RPM
  - Med encoder (optional men anbefalet)
  - Minimum 30-50mm hjul diameter
  - Pris: ~$10-15 per stk

### 4. Ultralyd Sensorer
- **3x HC-SR04 Ultrasonic Sensor**
  - Range: 2-400cm
  - 5V operation
  - 15° detection vinkel
  - Pris: ~$2 per stk

### 5. IMU (Gyroscope + Accelerometer)
- **MPU-6050** (budget option)
  - 6-axis (3 gyro + 3 accel)
  - I2C interface
  - Pris: ~$3-5

ELLER

- **MPU-9250** (anbefalet)
  - 9-axis (3 gyro + 3 accel + 3 mag)
  - Magnetometer for bedre heading
  - Pris: ~$8-12

### 6. Relay Modul
- **1-Channel 5V Relay**
  - For klippermotor kontrol
  - Max 10A / 250VAC
  - Optocoupler isolation
  - Pris: ~$2-3

### 7. Klippermotor
- **DC Motor** 12V eller
- **Brushless Motor** med ESC
  - Afhængig af dit design
  - Med klipperblade
  - Pris: Varierer

### 8. Batteri
- **3S LiPo Battery**
  - 11.1V nominal (12.6V full)
  - Minimum 3000mAh
  - Anbefalet: 5000-8000mAh
  - XT60 eller T-plug connector
  - Pris: ~$30-60

### 9. Batteri Charger
- **LiPo Balance Charger**
  - For 3S LiPo
  - Auto-cutoff ved fuld
  - Pris: ~$15-30

### 10. Voltage Regulator
- **Buck Converter** (DC-DC Step Down)
  - Input: 12V
  - Output: 5V, 3A minimum
  - For ESP32 og sensorer
  - Pris: ~$3-5

## 🔌 Elektronik Komponenter

### Modstande
- 10kΩ - 2x (voltage divider)
- 2.2kΩ - 1x (voltage divider)

### Kondensatorer (anbefalet for støj reduktion)
- 100µF electrolytic - 2x
- 0.1µF ceramic - 5x

### Dioder (flyback protection)
- 1N4007 - 2x

### Forbindingsledninger
- Dupont jumper wires (hun-hun, han-han, han-hun)
- Minimum 40 stk mixed
- Pris: ~$5-10

### Prototype Board / Perfboard
- For permanent montering
- 70x90mm eller større
- Pris: ~$3-5

### Wire
- 22 AWG for signal
- 18 AWG for power
- Forskellige farver
- Pris: ~$10-15

## 🏗️ Mekaniske Komponenter

### Chassis
- Plast, aluminium eller 3D printed
- Minimum størrelse: 300x300mm
- Skal kunne bære ~2-3 kg

### Hjul
- 2x drive hjul (65-100mm diameter)
- 1x støtte hjul (caster wheel)
- God grip til græs
- Pris: ~$10-20

### Motor Beslag
- L-brackets for motor montering
- M3 eller M4 bolte og møtrikker
- Pris: ~$5

### Enclosures / Kasser
- Til elektronik beskyttelse
- Vandtæt anbefalet
- Pris: ~$10-20

## 🛠️ Værktøj Påkrævet

- Loddekolbe og tin
- Wire strippers
- Skruetrækkere (Phillips og flathead)
- Multimeter
- Breadboard (til prototyping)
- Afbitningstang
- Hot glue gun (optional)

## 💰 Samlet Pris Estimat

| Kategori | Pris (USD) |
|----------|------------|
| Microcontroller | $15-20 |
| Motor Driver | $5 |
| Motors (2x) | $20-30 |
| Sensorer (3x) | $6 |
| IMU | $5-12 |
| Relay | $3 |
| Batteri | $30-60 |
| Charger | $20 |
| Regulator | $5 |
| Elektronik dele | $20 |
| Mekanik | $30-50 |
| **TOTAL** | **~$160-230** |

*Priser er ca. og varierer efter leverandør og region*

## 🛒 Hvor at Købe

### Online Butikker
- **AliExpress** - Billigst, langsom shipping
- **Amazon** - Hurtig shipping, højere priser
- **eBay** - God til brugte dele
- **Adafruit** - Kvalitets komponenter
- **SparkFun** - Kvalitets komponenter
- **Digikey** - Professionelle komponenter

### Lokalt (Danmark)
- **Elfa Distrelec**
- **Conrad Electronic**
- **Farnell**

## 🔍 Valgfrie Opgraderinger

### GPS Modul
- **NEO-6M** eller **NEO-8M**
- For præcis position tracking
- Pris: ~$10-20

### Perimeter Wire Sensor
- Induktions spole
- For boundary detection
- Pris: ~$5-15

### Kamera
- **ESP32-CAM** modul
- For græs detection / obstacles
- Pris: ~$8-12

### Rain Sensor
- Enkel resistiv sensor
- Auto-stop ved regn
- Pris: ~$2-5

### Charging Station
- Auto-dock system
- Kræver IR beacon eller lignende
- DIY projekt

### Bumper Switches
- Mekanisk backup til sensorer
- Microswitch x 3-4
- Pris: ~$5

## ⚡ Power Budget

Estimeret strømforbrug:

| Komponent | Strøm |
|-----------|-------|
| ESP32-S3 | ~200mA (peak 500mA) |
| Motors (2x) | ~500-2000mA (load dependent) |
| Klippermotor | ~1000-3000mA |
| Sensorer (3x) | ~15mA x 3 = 45mA |
| IMU | ~5mA |
| Display | ~20mA |
| **Total** | **~2-6A** |

Med 5000mAh batteri:
- Idle: ~4-5 timer
- Aktiv klipning: ~45-90 minutter

## 🔧 Vedligeholdelse Dele

Dele der slides/skal udskiftes:
- Klipperblade (hver 20-50 timer)
- Hjul (hver 100+ timer)
- Batteri (efter 300-500 cycles)

---

**Note**: Dette er en reference liste. Du kan substituere komponenter med lignende specifikationer.

