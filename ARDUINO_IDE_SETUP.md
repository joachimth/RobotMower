# Arduino IDE Setup Guide

## Problem: Sketch Too Big

Hvis du får fejlen "Sketch too big" i Arduino IDE, skal du ændre partition scheme til at give mere plads til programmet.

## Løsning: Ændr Partition Scheme

### I Arduino IDE:

1. **Åbn Arduino IDE**
2. **Vælg dit board**: Tools → Board → ESP32 Arduino → ESP32 Dev Module
3. **Ændr Partition Scheme**:
   - Tools → Partition Scheme → **"Huge APP (3MB No OTA/1MB SPIFFS)"**

   Eller alternativt:
   - Tools → Partition Scheme → **"No OTA (2MB APP/2MB SPIFFS)"**

4. **Upload igen**

### Forklaring af Partition Schemes:

- **Default (1.3MB)**: Standard, men for lidt til dette projekt
- **No OTA (2MB APP/2MB SPIFFS)**: ✅ Anbefalet - god balance
- **Huge APP (3MB)**: ✅ Maksimal plads til app, minimal til filer
- **Minimal SPIFFS (1.9MB APP)**: ✅ Også OK hvis du ikke bruger mange web filer

## Alternative Optimeringer

Hvis du stadig har problemer, kan du reducere features i `src/config/Config.h`:

```cpp
// Deaktiver ikke-kritiske features:
#define ENABLE_AUTO_UPDATE          false  // Sparer ~50KB
#define ENABLE_WIFI_MANAGER         false  // Sparer ~30KB (mister captive portal)
#define ENABLE_WEBSOCKET            false  // Sparer ~40KB (mister real-time updates)
```

## Compiler Optimering

For at optimere yderligere i Arduino IDE:

1. Tools → Core Debug Level → **"None"**
2. Sørg for at `DEBUG_MODE` er `false` i Config.h for production builds

## Verification

Efter ændring af partition scheme skulle du se:

```
Sketch uses ~1200000 bytes (60-90%) of program storage space.
Maximum is 2097152 bytes (for No OTA scheme)
```

Dette er acceptabelt! ✅
