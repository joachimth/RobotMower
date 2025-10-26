// Robot Mower Web Interface JavaScript

// Global variables
let ws = null;
let wsConnected = false;
let autoScroll = true;
let updateInterval = null;

// Canvas contexts
let sensorCanvas = null;
let sensorCtx = null;
let compassCanvas = null;
let compassCtx = null;

// Initialize on page load
document.addEventListener('DOMContentLoaded', function() {
    console.log('Robot Mower Interface Loading...');

    initializeCanvas();
    initializeEventListeners();
    initializeWebSocket();
    startStatusUpdates();
});

// Initialize canvas elements
function initializeCanvas() {
    sensorCanvas = document.getElementById('sensorCanvas');
    sensorCtx = sensorCanvas.getContext('2d');

    compassCanvas = document.getElementById('compassCanvas');
    compassCtx = compassCanvas.getContext('2d');

    // Draw initial states
    drawSensors(0, 0, 0);
    drawCompass(0);
}

// Initialize event listeners
function initializeEventListeners() {
    // Control buttons
    document.getElementById('btnStart').addEventListener('click', handleStart);
    document.getElementById('btnStop').addEventListener('click', handleStop);
    document.getElementById('btnPause').addEventListener('click', handlePause);
    document.getElementById('btnCalibrate').addEventListener('click', handleCalibrate);

    // Logs controls
    document.getElementById('btnClearLogs').addEventListener('click', clearLogs);
    document.getElementById('chkAutoScroll').addEventListener('change', function(e) {
        autoScroll = e.target.checked;
    });

    // Settings
    document.getElementById('btnSaveSettings').addEventListener('click', handleSaveSettings);
    document.getElementById('motorSpeed').addEventListener('input', function(e) {
        document.getElementById('motorSpeedValue').textContent = e.target.value;
    });
}

// Initialize WebSocket connection
function initializeWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;

    console.log('Connecting to WebSocket:', wsUrl);

    ws = new WebSocket(wsUrl);

    ws.onopen = function() {
        console.log('WebSocket connected');
        wsConnected = true;
        updateConnectionStatus(true);
        addLog('info', 'WebSocket forbundet');
    };

    ws.onclose = function() {
        console.log('WebSocket disconnected');
        wsConnected = false;
        updateConnectionStatus(false);
        addLog('warn', 'WebSocket forbindelse lukket');

        // Attempt reconnect after 5 seconds
        setTimeout(initializeWebSocket, 5000);
    };

    ws.onerror = function(error) {
        console.error('WebSocket error:', error);
        addLog('error', 'WebSocket fejl');
    };

    ws.onmessage = function(event) {
        handleWebSocketMessage(event.data);
    };
}

// Handle WebSocket messages
function handleWebSocketMessage(data) {
    try {
        const message = JSON.parse(data);

        switch(message.type) {
            case 'status':
                updateDashboard(message.data);
                break;
            case 'sensor':
                updateSensors(message.data);
                break;
            case 'log':
                addLog(message.level || 'info', message.message);
                break;
            default:
                console.log('Unknown message type:', message.type);
        }
    } catch(e) {
        console.error('Error parsing WebSocket message:', e);
    }
}

// Update connection status indicator
function updateConnectionStatus(connected) {
    const statusDot = document.getElementById('statusDot');
    const connectionText = document.getElementById('connectionText');

    if (connected) {
        statusDot.classList.add('connected');
        connectionText.textContent = 'Forbundet';
    } else {
        statusDot.classList.remove('connected');
        connectionText.textContent = 'Ikke forbundet';
    }
}

// Start periodic status updates via HTTP
function startStatusUpdates() {
    updateInterval = setInterval(fetchStatus, 1000); // Every 1 second
}

// Fetch status from API
async function fetchStatus() {
    try {
        const response = await fetch('/api/status');
        if (response.ok) {
            const data = await response.json();
            updateDashboard(data);
        }
    } catch(e) {
        console.error('Error fetching status:', e);
    }
}

// Update dashboard with status data
function updateDashboard(data) {
    // State
    if (data.state) {
        document.getElementById('robotState').textContent = data.state;
    }

    // Battery
    if (data.battery) {
        const voltage = data.battery.voltage.toFixed(1);
        const percentage = data.battery.percentage;

        document.getElementById('batteryVoltage').textContent = `${voltage}V`;
        document.getElementById('batteryPercent').textContent = `${percentage}%`;

        // Update battery bar
        const batteryLevel = document.getElementById('batteryLevel');
        batteryLevel.style.width = `${percentage}%`;

        // Color based on level
        if (data.battery.isCritical) {
            batteryLevel.style.background = '#f44336';
        } else if (data.battery.isLow) {
            batteryLevel.style.background = '#ff9800';
        } else {
            batteryLevel.style.background = 'linear-gradient(90deg, #f44336, #ff9800, #4CAF50)';
        }
    }

    // Heading
    if (data.heading !== undefined) {
        const heading = Math.round(data.heading);
        document.getElementById('heading').textContent = `${heading}°`;
        document.getElementById('compassHeading').textContent = heading;
        drawCompass(heading);
    }

    // Sensors
    if (data.sensors) {
        updateSensors(data.sensors);
    }

    // Uptime
    if (data.uptime) {
        const uptimeSeconds = Math.floor(data.uptime / 1000);
        const hours = Math.floor(uptimeSeconds / 3600);
        const minutes = Math.floor((uptimeSeconds % 3600) / 60);
        const seconds = uptimeSeconds % 60;

        document.getElementById('uptime').textContent =
            `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
    }

    // Free heap
    if (data.freeHeap) {
        document.getElementById('freeHeap').textContent = Math.floor(data.freeHeap / 1024);
    }
}

// Update sensor visualization
function updateSensors(sensors) {
    const left = sensors.left || 0;
    const middle = sensors.middle || 0;
    const right = sensors.right || 0;

    document.getElementById('sensorLeft').textContent = Math.round(left);
    document.getElementById('sensorMiddle').textContent = Math.round(middle);
    document.getElementById('sensorRight').textContent = Math.round(right);

    drawSensors(left, middle, right);
}

// Draw sensor visualization
function drawSensors(left, middle, right) {
    const ctx = sensorCtx;
    const width = sensorCanvas.width;
    const height = sensorCanvas.height;

    // Clear canvas
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, width, height);

    // Robot position (center bottom)
    const robotX = width / 2;
    const robotY = height - 30;
    const robotSize = 30;

    // Draw robot
    ctx.fillStyle = '#4CAF50';
    ctx.fillRect(robotX - robotSize/2, robotY - robotSize/2, robotSize, robotSize);

    // Draw sensor beams
    const maxDistance = 200; // cm
    const scale = (height - 50) / maxDistance;

    // Left sensor
    drawSensorBeam(ctx, robotX - 15, robotY - 10, -30, left, scale);

    // Middle sensor
    drawSensorBeam(ctx, robotX, robotY - 15, 0, middle, scale);

    // Right sensor
    drawSensorBeam(ctx, robotX + 15, robotY - 10, 30, right, scale);
}

// Draw individual sensor beam
function drawSensorBeam(ctx, x, y, angle, distance, scale) {
    const maxDist = 200;
    const dist = Math.min(distance, maxDist);
    const length = dist * scale;

    const angleRad = (angle - 90) * Math.PI / 180;
    const endX = x + Math.cos(angleRad) * length;
    const endY = y + Math.sin(angleRad) * length;

    // Color based on distance
    let color;
    if (dist < 30) {
        color = '#f44336'; // Red - obstacle close
    } else if (dist < 50) {
        color = '#ff9800'; // Orange - warning
    } else {
        color = '#4CAF50'; // Green - clear
    }

    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(x, y);
    ctx.lineTo(endX, endY);
    ctx.stroke();

    // Draw obstacle marker if detected
    if (dist > 0 && dist < maxDist) {
        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.arc(endX, endY, 5, 0, 2 * Math.PI);
        ctx.fill();
    }
}

// Draw compass
function drawCompass(heading) {
    const ctx = compassCtx;
    const width = compassCanvas.width;
    const height = compassCanvas.height;
    const centerX = width / 2;
    const centerY = height / 2;
    const radius = Math.min(width, height) / 2 - 20;

    // Clear canvas
    ctx.fillStyle = '#1a1a1a';
    ctx.fillRect(0, 0, width, height);

    // Draw compass circle
    ctx.strokeStyle = '#404040';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
    ctx.stroke();

    // Draw cardinal directions
    ctx.fillStyle = '#888';
    ctx.font = '14px Arial';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';

    ctx.fillText('N', centerX, centerY - radius - 10);
    ctx.fillText('S', centerX, centerY + radius + 10);
    ctx.fillText('E', centerX + radius + 10, centerY);
    ctx.fillText('W', centerX - radius - 10, centerY);

    // Draw heading arrow
    const headingRad = (heading - 90) * Math.PI / 180;
    const arrowX = centerX + Math.cos(headingRad) * (radius - 10);
    const arrowY = centerY + Math.sin(headingRad) * (radius - 10);

    ctx.strokeStyle = '#4CAF50';
    ctx.fillStyle = '#4CAF50';
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(centerX, centerY);
    ctx.lineTo(arrowX, arrowY);
    ctx.stroke();

    // Arrow head
    ctx.beginPath();
    ctx.arc(arrowX, arrowY, 6, 0, 2 * Math.PI);
    ctx.fill();
}

// Add log entry
function addLog(level, message) {
    const logsContainer = document.getElementById('logs');
    const logEntry = document.createElement('div');
    logEntry.className = `log-entry ${level.toLowerCase()}`;

    const timestamp = new Date().toLocaleTimeString();
    logEntry.textContent = `[${timestamp}] [${level.toUpperCase()}] ${message}`;

    logsContainer.appendChild(logEntry);

    // Auto-scroll if enabled
    if (autoScroll) {
        logsContainer.scrollTop = logsContainer.scrollHeight;
    }

    // Limit log entries to 100
    while (logsContainer.children.length > 100) {
        logsContainer.removeChild(logsContainer.firstChild);
    }
}

// Clear logs
function clearLogs() {
    document.getElementById('logs').innerHTML = '';
    addLog('info', 'Logs ryddet');
}

// Control button handlers
async function handleStart() {
    try {
        const response = await fetch('/api/start', { method: 'POST' });
        if (response.ok) {
            addLog('info', 'Start kommando sendt');
        }
    } catch(e) {
        addLog('error', 'Fejl ved start kommando');
    }
}

async function handleStop() {
    try {
        const response = await fetch('/api/stop', { method: 'POST' });
        if (response.ok) {
            addLog('info', 'Stop kommando sendt');
        }
    } catch(e) {
        addLog('error', 'Fejl ved stop kommando');
    }
}

async function handlePause() {
    try {
        const response = await fetch('/api/pause', { method: 'POST' });
        if (response.ok) {
            addLog('info', 'Pause kommando sendt');
        }
    } catch(e) {
        addLog('error', 'Fejl ved pause kommando');
    }
}

async function handleCalibrate() {
    try {
        const response = await fetch('/api/calibrate', { method: 'POST' });
        if (response.ok) {
            addLog('info', 'Kalibrering startet');
        }
    } catch(e) {
        addLog('error', 'Fejl ved kalibrering');
    }
}

// Save settings
async function handleSaveSettings() {
    const settings = {
        obstacleThreshold: document.getElementById('obstacleThreshold').value,
        rowWidth: document.getElementById('rowWidth').value,
        motorSpeed: document.getElementById('motorSpeed').value
    };

    try {
        const response = await fetch('/api/settings', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(settings)
        });

        if (response.ok) {
            addLog('info', 'Indstillinger gemt');
        }
    } catch(e) {
        addLog('error', 'Fejl ved gem indstillinger');
    }
}
