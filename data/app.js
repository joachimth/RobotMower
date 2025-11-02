/**
 * ============================================================================
 * Robot Mower Web Interface - JavaScript
 * ============================================================================
 * WebSocket integration og API kontrol
 */

class RobotMowerController {
    constructor() {
        this.ws = null;
        this.wsUrl = `ws://${window.location.hostname}:81`;
        this.apiUrl = `http://${window.location.hostname}`;
        this.reconnectInterval = 5000;
        this.currentSpeed = 200;

        this.init();
    }

    init() {
        this.setupEventListeners();
        this.connectWebSocket();
        this.fetchInitialStatus();

        // Update speed display
        const speedSlider = document.getElementById('speedSlider');
        speedSlider.addEventListener('input', (e) => {
            this.currentSpeed = parseInt(e.target.value);
            document.getElementById('speedValue').textContent = this.currentSpeed;
        });
    }

    setupEventListeners() {
        // Manual control buttons
        document.getElementById('btnForward').addEventListener('mousedown', () => this.sendManualCommand('forward'));
        document.getElementById('btnForward').addEventListener('mouseup', () => this.sendManualCommand('stop'));
        document.getElementById('btnForward').addEventListener('touchstart', (e) => { e.preventDefault(); this.sendManualCommand('forward'); });
        document.getElementById('btnForward').addEventListener('touchend', (e) => { e.preventDefault(); this.sendManualCommand('stop'); });

        document.getElementById('btnBackward').addEventListener('mousedown', () => this.sendManualCommand('backward'));
        document.getElementById('btnBackward').addEventListener('mouseup', () => this.sendManualCommand('stop'));
        document.getElementById('btnBackward').addEventListener('touchstart', (e) => { e.preventDefault(); this.sendManualCommand('backward'); });
        document.getElementById('btnBackward').addEventListener('touchend', (e) => { e.preventDefault(); this.sendManualCommand('stop'); });

        document.getElementById('btnLeft').addEventListener('mousedown', () => this.sendManualCommand('left'));
        document.getElementById('btnLeft').addEventListener('mouseup', () => this.sendManualCommand('stop'));
        document.getElementById('btnLeft').addEventListener('touchstart', (e) => { e.preventDefault(); this.sendManualCommand('left'); });
        document.getElementById('btnLeft').addEventListener('touchend', (e) => { e.preventDefault(); this.sendManualCommand('stop'); });

        document.getElementById('btnRight').addEventListener('mousedown', () => this.sendManualCommand('right'));
        document.getElementById('btnRight').addEventListener('mouseup', () => this.sendManualCommand('stop'));
        document.getElementById('btnRight').addEventListener('touchstart', (e) => { e.preventDefault(); this.sendManualCommand('right'); });
        document.getElementById('btnRight').addEventListener('touchend', (e) => { e.preventDefault(); this.sendManualCommand('stop'); });

        document.getElementById('btnStop').addEventListener('click', () => this.sendManualCommand('stop'));

        // Cutting mechanism
        document.getElementById('btnCuttingStart').addEventListener('click', () => this.cuttingControl('start'));
        document.getElementById('btnCuttingStop').addEventListener('click', () => this.cuttingControl('stop'));

        // Auto controls
        document.getElementById('btnStart').addEventListener('click', () => this.autoControl('start'));
        document.getElementById('btnPause').addEventListener('click', () => this.autoControl('pause'));
        document.getElementById('btnStopMowing').addEventListener('click', () => this.autoControl('stop'));
        document.getElementById('btnCalibrate').addEventListener('click', () => this.autoControl('calibrate'));
    }

    connectWebSocket() {
        try {
            this.ws = new WebSocket(this.wsUrl);

            this.ws.onopen = () => {
                console.log('WebSocket connected');
                this.updateConnectionStatus(true);
            };

            this.ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    this.handleWebSocketMessage(data);
                } catch (e) {
                    console.error('Failed to parse WebSocket message:', e);
                }
            };

            this.ws.onerror = (error) => {
                console.error('WebSocket error:', error);
                this.updateConnectionStatus(false);
            };

            this.ws.onclose = () => {
                console.log('WebSocket disconnected');
                this.updateConnectionStatus(false);

                // Reconnect after interval
                setTimeout(() => {
                    console.log('Attempting to reconnect...');
                    this.connectWebSocket();
                }, this.reconnectInterval);
            };
        } catch (e) {
            console.error('Failed to connect WebSocket:', e);
            this.updateConnectionStatus(false);
        }
    }

    handleWebSocketMessage(data) {
        // Update all status fields
        if (data.state) {
            document.getElementById('state').textContent = data.state;
        }

        if (data.battery) {
            const batteryText = `${data.battery.voltage.toFixed(1)}V (${data.battery.percentage}%)`;
            document.getElementById('battery').textContent = batteryText;

            const batteryEl = document.getElementById('battery');
            if (data.battery.isCritical) {
                batteryEl.style.color = 'var(--danger-color)';
            } else if (data.battery.isLow) {
                batteryEl.style.color = 'var(--warning-color)';
            } else {
                batteryEl.style.color = 'var(--success)';
            }
        }

        if (data.heading !== undefined) {
            document.getElementById('heading').textContent = `${data.heading.toFixed(0)}°`;
        }

        if (data.uptime !== undefined) {
            document.getElementById('uptime').textContent = this.formatUptime(data.uptime);
        }

        // Sensors
        if (data.sensors) {
            this.updateSensor('sensorLeft', data.sensors.left);
            this.updateSensor('sensorMiddle', data.sensors.middle);
            this.updateSensor('sensorRight', data.sensors.right);
        }

        // Motors
        if (data.motors) {
            document.getElementById('motorLeft').textContent = data.motors.left;
            document.getElementById('motorRight').textContent = data.motors.right;
            document.getElementById('motorMoving').textContent = data.motors.isMoving ? 'Kører' : 'Stoppet';

            // Current monitoring
            if (data.motors.current) {
                this.updateCurrent('currentLeft', data.motors.current.left);
                this.updateCurrent('currentRight', data.motors.current.right);
                document.getElementById('currentTotal').textContent = `${data.motors.current.total.toFixed(2)} A`;

                // Show warning if current is too high
                const warningEl = document.getElementById('currentWarning');
                if (data.motors.current.warning) {
                    warningEl.style.display = 'block';
                } else {
                    warningEl.style.display = 'none';
                }
            }
        }

        // Cutting mechanism
        if (data.cutting) {
            const statusText = data.cutting.running ? 'Kører' : 'Stoppet';
            const lockText = data.cutting.safetyLocked ? ' (Låst)' : '';
            document.getElementById('cuttingStatus').textContent = `Status: ${statusText}${lockText}`;
        }
    }

    updateSensor(elementId, distance) {
        const valueEl = document.getElementById(elementId);
        const barEl = document.getElementById(elementId + 'Bar');

        if (distance !== undefined && distance !== null) {
            valueEl.textContent = `${distance.toFixed(0)} cm`;

            // Update bar (max 200cm)
            const percentage = Math.min((distance / 200) * 100, 100);
            barEl.style.width = `${percentage}%`;
        } else {
            valueEl.textContent = '-- cm';
            barEl.style.width = '0%';
        }
    }

    updateCurrent(elementId, current) {
        const valueEl = document.getElementById(elementId);
        const barEl = document.getElementById(elementId + 'Bar');

        if (current !== undefined && current !== null) {
            valueEl.textContent = `${current.toFixed(2)} A`;

            // Update bar (max 43A for BTS7960)
            const percentage = Math.min((current / 43) * 100, 100);
            barEl.style.width = `${percentage}%`;

            // Change color based on current level
            if (current > 35) {
                barEl.style.background = 'var(--danger-color)';
            } else if (current > 25) {
                barEl.style.background = 'var(--warning-color)';
            } else {
                barEl.style.background = 'var(--success)';
            }
        } else {
            valueEl.textContent = '-- A';
            barEl.style.width = '0%';
        }
    }

    updateConnectionStatus(connected) {
        const statusDot = document.querySelector('.status-dot');
        const statusText = document.querySelector('.status-text');

        if (connected) {
            statusDot.classList.remove('offline');
            statusDot.classList.add('online');
            statusText.textContent = 'Online';
        } else {
            statusDot.classList.remove('online');
            statusDot.classList.add('offline');
            statusText.textContent = 'Offline';
        }
    }

    formatUptime(ms) {
        const seconds = Math.floor(ms / 1000);
        const minutes = Math.floor(seconds / 60);
        const hours = Math.floor(minutes / 60);
        const days = Math.floor(hours / 24);

        if (days > 0) {
            return `${days}d ${hours % 24}h ${minutes % 60}m`;
        } else if (hours > 0) {
            return `${hours}h ${minutes % 60}m`;
        } else {
            return `${minutes}m ${seconds % 60}s`;
        }
    }

    async sendManualCommand(action) {
        try {
            const formData = new FormData();
            formData.append('speed', this.currentSpeed);

            const response = await fetch(`${this.apiUrl}/api/manual/${action}`, {
                method: 'POST',
                body: formData
            });

            if (!response.ok) {
                console.error('Failed to send manual command:', action);
            }
        } catch (e) {
            console.error('Error sending manual command:', e);
        }
    }

    async cuttingControl(action) {
        try {
            const response = await fetch(`${this.apiUrl}/api/cutting/${action}`, {
                method: 'POST'
            });

            if (response.ok) {
                const data = await response.json();
                console.log('Cutting control:', data);
            } else {
                console.error('Failed to control cutting mechanism');
            }
        } catch (e) {
            console.error('Error controlling cutting mechanism:', e);
        }
    }

    async autoControl(action) {
        try {
            const response = await fetch(`${this.apiUrl}/api/${action}`, {
                method: 'POST'
            });

            if (response.ok) {
                const data = await response.json();
                console.log('Auto control:', data);
            } else {
                console.error('Failed to execute auto control');
            }
        } catch (e) {
            console.error('Error executing auto control:', e);
        }
    }

    async fetchInitialStatus() {
        try {
            const response = await fetch(`${this.apiUrl}/api/status`);
            if (response.ok) {
                const data = await response.json();
                this.handleWebSocketMessage(data);
            }
        } catch (e) {
            console.error('Error fetching initial status:', e);
        }
    }
}

// Initialize controller when page loads
document.addEventListener('DOMContentLoaded', () => {
    const controller = new RobotMowerController();
});
