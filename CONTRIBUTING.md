# 🤝 Bidrag til Robot Plæneklipper

Tak fordi du vil bidrage til projektet! Vi værdsætter alle bidrag, fra bug fixes til nye features.

## 🚀 Hurtig Start

### 1. Fork og Clone

```bash
# Fork projektet på GitHub, derefter:
git clone https://github.com/dit-brugernavn/RobotMower.git
cd RobotMower
```

### 2. Opsæt Development Environment

**PlatformIO (Anbefalet):**
```bash
# Installer PlatformIO
pip install platformio

# Byg projektet
pio run

# Upload til ESP32
pio run --target upload
```

**Arduino IDE:**
- Se [README.md](README.md) for installation instruktioner
- Husk at installere alle libraries

### 3. Opret en Branch

```bash
git checkout -b feature/ny-feature
# eller
git checkout -b fix/bug-fix
```

## 📝 Development Workflow

### 1. Konfiguration

Før du bygger første gang:

```bash
# Kopiér credentials template
cp src/config/Credentials.h.example src/config/Credentials.h

# Opdater med dine WiFi credentials
nano src/config/Credentials.h
```

### 2. Byg og Test

```bash
# Byg projekt
pio run

# Upload til device
pio run --target upload

# Monitor serial output
pio device monitor
```

### 3. Code Style

Følg eksisterende code style:
- Brug 4 spaces for indentation
- Kommenter kompleks logik
- Brug beskrivende variable navne
- Følg C++ naming conventions:
  - `camelCase` for funktioner og variabler
  - `PascalCase` for klasser
  - `UPPER_CASE` for konstanter

### 4. Commit Messages

Brug beskrivende commit messages:

```bash
# God commit message
git commit -m "Tilføj IMU kalibrering på opstart"
git commit -m "Fix motor strøm beregning i Motors.cpp"

# Mindre god commit message
git commit -m "fix"
git commit -m "update code"
```

### 5. Testing

Test din ændring:
- ✅ Kompilerer uden errors/warnings
- ✅ Fungerer på hardware
- ✅ Eksisterende funktionalitet virker stadig
- ✅ Serial output viser ingen kritiske fejl

## 🔄 Pull Request Process

### 1. Push til din Fork

```bash
git push origin feature/ny-feature
```

### 2. Opret Pull Request

- Gå til GitHub og opret en Pull Request
- Beskriv dine ændringer
- Reference relevante issues (hvis nogen)
- Vent på automatisk build at køre

### 3. GitHub Actions Check

Når du opretter en PR, kører GitHub Actions automatisk:
- ✅ Build check (kompilerer koden)
- ✅ Genererer firmware .bin fil
- ✅ Uploader artifact

Hvis build fejler, fix problemerne og push igen.

### 4. Code Review

- Et projekt maintainer vil reviewe din PR
- Der kan blive bedt om ændringer
- Diskuter gerne implementationen

### 5. Merge

Når PR er godkendt:
- Maintainer merger til main branch
- GitHub Actions bygger automatisk ny version
- Din ændring er nu en del af projektet! 🎉

## 🐛 Rapportér Bugs

### Før du rapporterer

1. Tjek om buggen allerede er rapporteret
2. Test på seneste version
3. Isoler problemet (kan det reproduceres?)

### Bug Report Template

Opret et issue med følgende info:

```markdown
**Beskrivelse:**
Kort beskrivelse af problemet

**Steps to Reproduce:**
1. Gør dette
2. Gør det
3. Se fejlen

**Forventet adfærd:**
Hvad skulle ske?

**Faktisk adfærd:**
Hvad sker der i stedet?

**Environment:**
- ESP32 model: ESP32-WROOM-32U
- Firmware version: v1.0.0
- Arduino IDE / PlatformIO version:

**Serial Output:**
```
(paste serial output her)
```

**Ekstra info:**
Screenshots, logs, etc.
```

## 💡 Foreslå Features

Vil du foreslå en ny feature?

1. Opret et issue med label "enhancement"
2. Beskriv featuren og use case
3. Diskuter implementation med maintainers
4. Implementer (eller hjælp med at implementere!)

## 🏗️ Projekt Struktur

```
RobotMower/
├── RobotMower.ino          # Main fil
├── src/
│   ├── config/             # Konfiguration
│   ├── hardware/           # Hardware drivers
│   ├── navigation/         # Navigation logik
│   ├── system/             # System management
│   ├── web/                # Web server & API
│   └── utils/              # Utilities
├── data/                   # Web interface files
├── .github/workflows/      # GitHub Actions
└── platformio.ini          # PlatformIO config
```

## 📚 Resources

- [README.md](README.md) - Projekt overview
- [API.md](API.md) - API dokumentation
- [HARDWARE.md](HARDWARE.md) - Hardware specifikationer
- [PINOUT.md](PINOUT.md) - Pin forbindelser
- [SETUP.md](SETUP.md) - Detaljeret setup guide

## ❓ Spørgsmål?

Har du spørgsmål?
- Åbn et issue på GitHub
- Tag med "question" label

## 🙏 Tak!

Tak for dit bidrag til Robot Plæneklipper projektet! Hver PR, issue report og feature forslag hjælper med at gøre projektet bedre.

---

**Husk:** Dette er et open source projekt drevet af fællesskabet. Vær venlig, respektfuld og hjælpsom! 🌟
