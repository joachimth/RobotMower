# 🌐 Web API Dokumentation

Robot Plæneklipper REST API og WebSocket interface.

## Base URL

```
http://robot-mower.local
```

Eller brug IP adresse (se Serial Monitor eller OLED display)

---

## REST API Endpoints

### GET /api/status

Returnerer nuværende robot status.

**Response:**
```json
{
  "state": "MOWING",
  "timeInState": 45230,
  "battery": {
    "voltage": 12.1,
    "percentage": 85,
    "isLow": false,
    "isCritical": false
  },
  "heading": 45.2,
  "pitch": 2.1,
  "roll": -1.3,
  "sensors": {
    "left": 150.5,
    "middle": 200.0,
    "right": 180.2
  },
  "motors": {
    "left": 200,
    "right": 200,
    "isMoving": true
  },
  "uptime": 3600000,
  "freeHeap": 234567
}
```

**States:**
- `IDLE` - Venter på kommando
- `CALIBRATING` - Kalibrerer sensorer
- `MOWING` - Klipper aktivt
- `TURNING` - Drejer
- `AVOIDING` - Undgår forhindring
- `RETURNING` - Vender hjem
- `CHARGING` - Lader
- `ERROR` - Fejltilstand

---

### POST /api/start

Starter klipning.

**Response:**
```json
{
  "status": "started"
}
```

---

### POST /api/stop

Stopper klipning og går til IDLE.

**Response:**
```json
{
  "status": "stopped"
}
```

---

### POST /api/pause

Pauser klipning (kan genoptages).

**Response:**
```json
{
  "status": "paused"
}
```

---

### POST /api/calibrate

Starter IMU kalibrering.

**Note:** Robot skal være helt stille!

**Response:**
```json
{
  "status": "calibrating"
}
```

---

### GET /api/logs

Henter seneste log entries.

**Parameters:**
- `count` (optional) - Antal log entries (default: 50)

**Example:**
```
GET /api/logs?count=100
```

**Response:**
```json
[
  {
    "level": "INFO",
    "message": "System starting...",
    "timestamp": 1234
  },
  {
    "level": "DEBUG",
    "message": "Sensors updated",
    "timestamp": 1335
  }
]
```

**Log Levels:**
- `DEBUG` - Debug information
- `INFO` - Informational messages
- `WARN` - Warnings
- `ERROR` - Errors

---

### GET /api/settings

Henter nuværende indstillinger.

**Response:**
```json
{
  "obstacleThreshold": 30,
  "motorCruiseSpeed": 200,
  "rowWidth": 30,
  "turnAngle": 90
}
```

---

### POST /api/settings

Opdaterer indstillinger.

**Request Body:**
```json
{
  "obstacleThreshold": 35,
  "motorCruiseSpeed": 180,
  "rowWidth": 25
}
```

**Response:**
```json
{
  "status": "updated"
}
```

---

## WebSocket API

### Connection

```javascript
const ws = new WebSocket('ws://robot-mower.local/ws');
```

### Message Types

#### 1. Status Update

Sendes periodisk med robot status.

```json
{
  "type": "status",
  "data": {
    "state": "MOWING",
    "battery": { ... },
    "heading": 45.2,
    "sensors": { ... }
  }
}
```

#### 2. Sensor Data

Real-time sensor updates.

```json
{
  "type": "sensor",
  "data": {
    "left": 150.0,
    "middle": 200.0,
    "right": 180.0
  }
}
```

#### 3. Log Message

Real-time log messages.

```json
{
  "type": "log",
  "level": "INFO",
  "message": "Started mowing",
  "timestamp": 123456
}
```

### Client → Server Commands

Send kommandoer til robot:

```json
{
  "command": "start"
}
```

**Supported Commands:**
- `start` - Start mowing
- `stop` - Stop mowing
- `pause` - Pause mowing
- `calibrate` - Start calibration

---

## Error Handling

### HTTP Error Codes

- `200 OK` - Request succesfuld
- `400 Bad Request` - Ugyldig request
- `404 Not Found` - Endpoint ikke fundet
- `500 Internal Server Error` - Server fejl

### Error Response Format

```json
{
  "error": "Description of error"
}
```

---

## Rate Limiting

Ingen rate limiting implementeret, men det anbefales at:
- Ikke sende flere end 10 requests/sekund til REST API
- WebSocket updates sendes max hver 200ms

---

## CORS

CORS er ikke aktiveret per default. Hvis du vil tilgå API fra anden origin:

Tilføj i `WebServer.cpp`:
```cpp
server->on(...).setFilter(ON_STA_FILTER)
     .setAuthentication("user", "password");
```

---

## Authentication

Ingen authentication implementeret per default.

For at tilføje basic auth, se ESPAsyncWebServer dokumentation.

---

## JavaScript Example

```javascript
// Fetch status
async function getStatus() {
  const response = await fetch('/api/status');
  const data = await response.json();
  console.log('Robot state:', data.state);
  console.log('Battery:', data.battery.percentage + '%');
}

// Start mowing
async function startMowing() {
  const response = await fetch('/api/start', {
    method: 'POST'
  });
  const data = await response.json();
  console.log('Started:', data.status);
}

// WebSocket connection
const ws = new WebSocket('ws://' + location.host + '/ws');

ws.onmessage = function(event) {
  const message = JSON.parse(event.data);

  switch(message.type) {
    case 'status':
      updateDashboard(message.data);
      break;
    case 'sensor':
      updateSensors(message.data);
      break;
    case 'log':
      addLog(message.level, message.message);
      break;
  }
};

// Send command via WebSocket
function sendCommand(cmd) {
  ws.send(JSON.stringify({ command: cmd }));
}
```

---

## Python Example

```python
import requests
import json

BASE_URL = "http://robot-mower.local"

# Get status
response = requests.get(f"{BASE_URL}/api/status")
status = response.json()
print(f"State: {status['state']}")
print(f"Battery: {status['battery']['percentage']}%")

# Start mowing
response = requests.post(f"{BASE_URL}/api/start")
print(response.json())

# Get logs
response = requests.get(f"{BASE_URL}/api/logs?count=10")
logs = response.json()
for log in logs:
    print(f"[{log['level']}] {log['message']}")
```

---

## curl Examples

```bash
# Get status
curl http://robot-mower.local/api/status

# Start mowing
curl -X POST http://robot-mower.local/api/start

# Stop mowing
curl -X POST http://robot-mower.local/api/stop

# Get logs
curl "http://robot-mower.local/api/logs?count=20"

# Update settings
curl -X POST http://robot-mower.local/api/settings \
  -H "Content-Type: application/json" \
  -d '{"obstacleThreshold": 35}'
```

---

## Notes

- Alle timestamps er i millisekunder siden boot (millis())
- Alle distance værdier er i centimeter
- Alle vinkler er i grader (0-360)
- Alle hastigheder er PWM værdier (0-255)
- WebSocket broadcast interval: 200ms
- Status update interval: 1000ms

