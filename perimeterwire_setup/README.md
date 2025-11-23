# Perimeter Wire System

## Overview

The perimeter wire system consists of two parts:
1. **Sender** (separate ESP32) - Generates the perimeter signal
2. **Receiver** (on robot mower) - Detects and decodes the signal

## Implementation Status

- [x] Sender project created (`perimeterwire_sender/`)
- [x] Signal generator (3.2 kHz carrier with pseudo-random coding)
- [x] Web API for sender control (start/stop/status)
- [x] Receiver module for robot mower
- [x] API client for robot mower to control sender
- [x] Integration in main robot code

## Hardware

### Sender (perimeterwire_sender/)
- ESP32 (DevKit or similar)
- Motor driver (MC33926 recommended, or L298N)
- DC/DC converter with adjustable output (6.5-12V)
- Perimeter wire (100m+)

### Receiver (on Robot Mower)
- Coil (100 mH or 150 mH) in upright position at front of robot
- LM386 operational amplifier module
  - **Important**: Bypass capacitor C3 on LM386 module for 0-3.3V output
- Connect LM386 output to GPIO0 (configurable in Config.h)

## Signal Specification

The perimeter signal is Ardumower compatible:
- Carrier frequency: 3.2 kHz
- Encoding: Two pulse widths (4808 Hz and 2404 Hz)
- 24-bit pseudo-random code sequence
- Maximum current: 1A (adjust via DC/DC potentiometer)

## API Endpoints

### Sender API (http://perimeter-sender.local)
- `GET /api/status` - Get sender status
- `POST /api/start` - Start signal output
- `POST /api/stop` - Stop signal output
- `POST /api/reset` - Reset after error

### Robot Mower API (http://robot-mower.local)
- `GET /api/perimeter/status` - Get receiver and sender status
- `POST /api/perimeter/start` - Start perimeter signal (via sender)
- `POST /api/perimeter/stop` - Stop perimeter signal
- `POST /api/perimeter/calibrate` - Calibrate receiver (place coil on wire)

## Configuration

### Sender Configuration
Edit `perimeterwire_sender/src/config/Config.h`:
- Pin definitions
- Signal parameters
- Current limits

Edit `perimeterwire_sender/src/config/Credentials.h`:
- WiFi credentials

### Robot Mower Configuration
Edit `src/config/Config.h`:
- `PERIMETER_SIGNAL_PIN` - GPIO for receiver input
- `PERIMETER_SENDER_IP` - IP address of sender ESP32
- `ENABLE_PERIMETER` - Enable/disable feature

## Building

### Sender
```bash
cd perimeterwire_sender
pio run
pio run -t upload
```

### Robot Mower
The perimeter receiver is integrated in the main robot mower code.
Build as usual with PlatformIO.

## Credits
- Ardumower project: https://wiki.ardumower.de/index.php?title=Perimeter_wire
