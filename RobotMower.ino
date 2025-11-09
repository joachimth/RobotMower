/**
 * ============================================================================
 * ROBOT PLÆNEKLIPPER - ESP32-WROOM-32U
 * ============================================================================
 * Autonome plæneklipper med systematisk klipningsmønster
 *
 * Hardware:
 * - ESP32-WROOM-32U Development Board (38-Pin)
 * - Double BTS7960 43A H-bridge Motor Driver (2x for begge motorer)
 * - 3x HC-SR04 Ultralyd Sensorer
 * - MPU-6050/9250 IMU
 * - Relay til klippermotor
 * - 18V (5S LiPo) for motorer og klippemotor
 * - 12V (3S LiPo) for styringsdel (ESP32)
 *
 * Features:
 * - Systematisk parallelt klipningsmønster
 * - Ultralyd baseret forhindring undgåelse
 * - IMU baseret retningsbestemmelse
 * - Web interface til kontrol med manuel kontrol
 * - Real-time telemetri via WebSocket
 * - Ekstern I2C OLED display support (valgfrit)
 * - Strømovervågning for motordriver (BTS7960 current sense)
 *
 * Author: Robot Mower Project
 * Version: 1.0
 * ============================================================================
 */

// ============================================================================
// INCLUDES
// ============================================================================

// Config
#include "src/config/Config.h"
#include "src/config/Credentials.h"

// System
#include "src/system/Logger.h"
#include "src/system/StateManager.h"
#include "src/system/WiFiManager.h"
#include "src/system/UpdateManager.h"

// Hardware
#include "src/hardware/Motors.h"
#include "src/hardware/Sensors.h"
#include "src/hardware/IMU.h"
// Display deaktiveret for ESP32-WROOM-32U (ingen indbygget display)
// #include "src/hardware/Display.h"
#include "src/hardware/CuttingMechanism.h"
#include "src/hardware/Battery.h"

// Navigation
#include "src/navigation/PathPlanner.h"
#include "src/navigation/ObstacleAvoidance.h"
#include "src/navigation/Movement.h"

// Web
#include "src/web/WebServer.h"
#include "src/web/WebAPI.h"
#include "src/web/WebSocket.h"

// Utils
#include "src/utils/Timer.h"
#include "src/utils/Math.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

// System
StateManager stateManager;
WiFiManager wifiManager;
UpdateManager updateManager;

// Hardware
Motors motors;
Sensors sensors;
IMU imu;
// Display deaktiveret for ESP32-WROOM-32U (ENABLE_DISPLAY = false)
// Display display;
CuttingMechanism cuttingMech;
Battery battery;

// Navigation
PathPlanner pathPlanner;
ObstacleAvoidance obstacleAvoid;
Movement movement;

// Web
WebServer webServer;
WebAPI webAPI;
WebSocketHandler webSocket;

// Timers
Timer sensorUpdateTimer(SENSOR_UPDATE_INTERVAL, true);
Timer imuUpdateTimer(IMU_UPDATE_INTERVAL, true);
Timer displayUpdateTimer(DISPLAY_UPDATE_INTERVAL, true);
Timer batteryCheckTimer(BATTERY_CHECK_INTERVAL, true);
Timer statusUpdateTimer(STATUS_UPDATE_INTERVAL, true);
Timer websocketUpdateTimer(WEBSOCKET_UPDATE_INTERVAL, true);
Timer currentUpdateTimer(100, true); // Strømovervågning hver 100ms

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void initializeHardware();
void initializeNavigation();
void initializeWeb();
void runStateMachine();
void handleIdleState();
void handleManualState();
void handleCalibratingState();
void handleMowingState();
void handleTurningState();
void handleAvoidingState();
void handleErrorState();
void updateSensors();
void updateIMU();
void updateDisplay();
void updateBattery();
void updateWebStatus();
void updateMotorCurrent();
void checkSafetyConditions();

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize Serial
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("============================================");
    Serial.println("   ROBOT PLÆNEKLIPPER - ESP32-WROOM-32U");
    Serial.println("============================================");

    // Initialize Logger
    Logger::begin();
    Logger::info("System starting...");

    // Initialize State Manager
    if (!stateManager.begin()) {
        Logger::error("Failed to initialize State Manager");
        while(1) delay(1000);
    }

    // Initialize Hardware
    initializeHardware();

    // Initialize Navigation
    initializeNavigation();

    // Initialize Web Server
    initializeWeb();

    // Display startup complete (deaktiveret - ingen display på ESP32-WROOM-32U)
    // #if ENABLE_DISPLAY
    // display.showSplash();
    // delay(2000);
    // #endif

    Logger::info("System initialization complete!");
    Logger::info("Ready to mow!");

    Serial.println("============================================");
    Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Update timers
    if (sensorUpdateTimer.isExpired()) {
        updateSensors();
        sensorUpdateTimer.reset();
    }

    if (imuUpdateTimer.isExpired()) {
        updateIMU();
        imuUpdateTimer.reset();
    }

    if (displayUpdateTimer.isExpired()) {
        updateDisplay();
        displayUpdateTimer.reset();
    }

    if (batteryCheckTimer.isExpired()) {
        updateBattery();
        batteryCheckTimer.reset();
    }

    if (statusUpdateTimer.isExpired()) {
        updateWebStatus();
        statusUpdateTimer.reset();
    }

    if (currentUpdateTimer.isExpired()) {
        updateMotorCurrent();
        currentUpdateTimer.reset();
    }

    // Update WiFi Manager (reconnect handling)
    #if ENABLE_WIFI_MANAGER
    wifiManager.update();
    #endif

    // Update state manager
    stateManager.update();

    // Run state machine
    runStateMachine();

    // Update web server
    webServer.update();
    webSocket.update();

    // Check safety conditions
    checkSafetyConditions();

    // Small delay to prevent watchdog issues
    delay(1);
}

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void initializeHardware() {
    Logger::info("Initializing hardware...");

    // Motors
    if (!motors.begin()) {
        Logger::error("Failed to initialize Motors");
        stateManager.handleError("Motor initialization failed");
        return;
    }

    // Sensors
    if (!sensors.begin()) {
        Logger::error("Failed to initialize Sensors");
        stateManager.handleError("Sensor initialization failed");
        return;
    }

    // IMU
    #if ENABLE_IMU
    if (!imu.begin()) {
        Logger::warning("Failed to initialize IMU - continuing without");
        // Not critical, continue
    }
    #endif

    // Display (deaktiveret for ESP32-WROOM-32U)
    // #if ENABLE_DISPLAY
    // if (!display.begin()) {
    //     Logger::warning("Failed to initialize Display - continuing without");
    //     // Not critical, continue
    // }
    // #endif

    // Cutting Mechanism
    if (!cuttingMech.begin()) {
        Logger::error("Failed to initialize Cutting Mechanism");
        stateManager.handleError("Cutting mechanism initialization failed");
        return;
    }

    // Battery
    if (!battery.begin()) {
        Logger::error("Failed to initialize Battery monitor");
        stateManager.handleError("Battery monitor initialization failed");
        return;
    }

    Logger::info("Hardware initialized successfully");
}

void initializeNavigation() {
    Logger::info("Initializing navigation...");

    // Path Planner
    if (!pathPlanner.begin()) {
        Logger::error("Failed to initialize Path Planner");
        return;
    }

    // Obstacle Avoidance
    if (!obstacleAvoid.begin()) {
        Logger::error("Failed to initialize Obstacle Avoidance");
        return;
    }

    // Movement
    if (!movement.begin(&motors, &imu)) {
        Logger::error("Failed to initialize Movement controller");
        return;
    }

    Logger::info("Navigation initialized successfully");
}

void initializeWeb() {
    Logger::info("Initializing web server...");

    // WiFi Manager - Håndterer WiFi forbindelse med captive portal
    #if ENABLE_WIFI_MANAGER
    Logger::info("Initializing WiFi Manager...");
    if (!wifiManager.begin()) {
        Logger::warning("WiFi Manager failed - robot vil køre i AP mode");
    }
    #endif

    // Update Manager - Håndterer auto-update fra GitHub
    #if ENABLE_AUTO_UPDATE
    Logger::info("Initializing Update Manager...");
    if (!updateManager.begin()) {
        Logger::warning("Update Manager failed");
    }
    #endif

    // Web Server
    #if ENABLE_WIFI_MANAGER
    webServer.setWiFiManager(&wifiManager);
    #endif
    #if ENABLE_AUTO_UPDATE
    webServer.setUpdateManager(&updateManager);
    #endif

    if (!webServer.begin()) {
        Logger::error("Failed to initialize Web Server");
        return;
    }

    // Web API
    if (!webAPI.begin(&webServer)) {
        Logger::error("Failed to initialize Web API");
        return;
    }

    // Set hardware references for API
    webAPI.setHardwareReferences(&stateManager, &battery, &sensors, &imu, &motors, &cuttingMech);

    // Setup API routes
    webAPI.setupRoutes();

    // WebSocket
    #if ENABLE_WEBSOCKET
    if (!webSocket.begin(&webServer)) {
        Logger::error("Failed to initialize WebSocket");
        return;
    }

    // Set hardware references for WebSocket
    webSocket.setHardwareReferences(&stateManager, &motors, &cuttingMech);
    #endif

    Logger::info("Web server initialized successfully");

    // Log access info
    String accessInfo = wifiManager.isAPMode() ?
        "Captive Portal: Connect to '" + String(CAPTIVE_PORTAL_SSID) + "' and open http://" + wifiManager.getIPAddress() :
        "Web Interface: http://robot-mower.local or http://" + wifiManager.getIPAddress();
    Logger::info(accessInfo);
}

// ============================================================================
// STATE MACHINE
// ============================================================================

void runStateMachine() {
    RobotState currentState = stateManager.getState();

    switch (currentState) {
        case STATE_IDLE:
            handleIdleState();
            break;

        case STATE_MANUAL:
            handleManualState();
            break;

        case STATE_CALIBRATING:
            handleCalibratingState();
            break;

        case STATE_MOWING:
            handleMowingState();
            break;

        case STATE_TURNING:
            handleTurningState();
            break;

        case STATE_AVOIDING:
            handleAvoidingState();
            break;

        case STATE_ERROR:
            handleErrorState();
            break;

        default:
            break;
    }
}

void handleIdleState() {
    // Robot er idle - venter på kommando
    motors.stop();
    cuttingMech.stop();
}

void handleManualState() {
    // Manuel kontrol - state machine gør ingenting
    // Motorerne styres direkte via API kommandoer
    // Ingen automatisk stop eller overskriv af kommandoer
}

void handleCalibratingState() {
    // Kalibrerer IMU
    static bool calibrationStarted = false;

    if (!calibrationStarted) {
        Logger::info("Starting IMU calibration - keep robot still!");
        // Display deaktiveret
        // #if ENABLE_DISPLAY
        // display.showCalibration(0);
        // #endif

        imu.calibrate();
        calibrationStarted = true;

        // Efter kalibrering, gå tilbage til IDLE
        stateManager.setState(STATE_IDLE);
        calibrationStarted = false;
    }
}

void handleMowingState() {
    // Opdater path planner
    pathPlanner.update();

    // Tjek for forhindringer
    obstacleAvoid.update(&sensors);

    if (obstacleAvoid.hasObstacle()) {
        // Forhindring detekteret - skift til AVOIDING state
        stateManager.setState(STATE_AVOIDING);
        return;
    }

    // Tjek om vi skal dreje
    if (pathPlanner.shouldTurn()) {
        stateManager.setState(STATE_TURNING);
        pathPlanner.startTurn();
        return;
    }

    // Kør lige fremad
    float targetHeading = pathPlanner.getTargetHeading();
    movement.setTargetHeading(targetHeading);
    movement.driveStraight(MOTOR_CRUISE_SPEED);

    // Start klippermotor hvis ikke allerede kører
    if (!cuttingMech.isRunning() && !cuttingMech.isSafetyLocked()) {
        cuttingMech.start();
    }
}

void handleTurningState() {
    // Stop klippermotor under drejning
    cuttingMech.stop();

    // Hent target heading fra path planner
    float targetHeading = pathPlanner.getTargetHeading();

    // Drej til target heading
    bool turnComplete = movement.turnToHeading(targetHeading);

    if (turnComplete) {
        // Drejning færdig
        pathPlanner.completeTurn();
        pathPlanner.nextRow();

        // Tjek om mønster er færdigt
        if (pathPlanner.isPatternComplete()) {
            Logger::info("Mowing pattern complete!");
            stateManager.setState(STATE_IDLE);
        } else {
            // Fortsæt klipning
            stateManager.setState(STATE_MOWING);
        }
    }
}

void handleAvoidingState() {
    // Stop klippermotor
    cuttingMech.stop();

    // Hent undgåelses retning
    AvoidanceDirection avoidDir = obstacleAvoid.getAvoidanceDirection();

    static bool avoidanceComplete = false;

    if (!avoidanceComplete) {
        // Eksekvér undgåelses manøvre
        switch (avoidDir) {
            case AVOID_LEFT:
                Logger::info("Avoiding - turning left");
                movement.backUp(BACKUP_DISTANCE);
                delay(500);
                movement.turnInPlace(LEFT, MOTOR_TURN_SPEED);
                delay(1000);
                break;

            case AVOID_RIGHT:
                Logger::info("Avoiding - turning right");
                movement.backUp(BACKUP_DISTANCE);
                delay(500);
                movement.turnInPlace(RIGHT, MOTOR_TURN_SPEED);
                delay(1000);
                break;

            case AVOID_BACK:
                Logger::info("Avoiding - backing up");
                movement.backUp(BACKUP_DISTANCE * 2);
                delay(500);
                movement.turnInPlace(RIGHT, MOTOR_TURN_SPEED);
                delay(1500);
                break;
        }

        movement.stop();
        avoidanceComplete = true;
    }

    // Tjek om vejen er fri
    sensors.update();
    obstacleAvoid.update(&sensors);

    if (!obstacleAvoid.hasObstacle()) {
        // Vejen er fri - fortsæt klipning
        avoidanceComplete = false;
        stateManager.setState(STATE_MOWING);
    }
}

void handleErrorState() {
    // Stop alt
    motors.emergencyStop();
    cuttingMech.emergencyStop();

    // Vis fejl på display (deaktiveret - brug Serial Monitor i stedet)
    // #if ENABLE_DISPLAY
    // display.showError(stateManager.getErrorMessage());
    // #endif

    // Log fejl til Serial
    Logger::error("ERROR STATE: " + stateManager.getErrorMessage());

    // Kræver manuel genstart
}

// ============================================================================
// UPDATE FUNCTIONS
// ============================================================================

void updateSensors() {
    sensors.update();

    // Log sensor data periodisk
    #if DEBUG_SENSORS
    static int sensorLogCounter = 0;
    if (++sensorLogCounter >= 50) { // Ca. hver 5 sekund
        Logger::logSensorData(sensors.getLeftDistance(),
                             sensors.getMiddleDistance(),
                             sensors.getRightDistance());
        sensorLogCounter = 0;
    }
    #endif
}

void updateIMU() {
    #if ENABLE_IMU
    imu.update();

    // Tjek for væltet robot
    if (imu.isTilted() && stateManager.isActive()) {
        Logger::error("Robot tilted - emergency stop!");
        stateManager.handleError("Robot tilted");
    }
    #endif
}

void updateDisplay() {
    // Display deaktiveret for ESP32-WROOM-32U (ENABLE_DISPLAY = false)
    // #if ENABLE_DISPLAY
    // display.update();
    //
    // // Vis status
    // display.showStatus(
    //     stateManager.getStateName(),
    //     battery.getVoltage(),
    //     imu.getHeading()
    // );
    // #endif
}

void updateBattery() {
    battery.update();

    // Tjek batteri niveau
    if (battery.isCritical()) {
        Logger::error("Critical battery level!");
        stateManager.handleError("Critical battery");
    } else if (battery.isLow() && stateManager.isActive()) {
        Logger::warning("Low battery - returning to base");
        stateManager.setState(STATE_RETURNING);
    }
}

void updateWebStatus() {
    // Broadcast status via WebSocket
    #if ENABLE_WEBSOCKET
    // Status JSON kommer fra WebAPI
    String statusJSON = webAPI.createStatusJSON();
    webSocket.broadcastStatus(statusJSON);

    // Broadcast sensor data
    webSocket.broadcastSensorData(
        sensors.getLeftDistance(),
        sensors.getMiddleDistance(),
        sensors.getRightDistance()
    );
    #endif
}

void updateMotorCurrent() {
    // Opdater strømmålinger fra BTS7960 current sense pins
    motors.updateCurrentReadings();

    // Tjek for strøm advarsel
    if (motors.isCurrentWarning() && stateManager.isActive()) {
        Logger::warning("Motor current warning! Left: " +
                       String(motors.getLeftCurrent(), 2) + "A, Right: " +
                       String(motors.getRightCurrent(), 2) + "A");
    }
}

void checkSafetyConditions() {
    // Kritiske sikkerhedstjek

    // 1. Batteri kritisk
    if (battery.isCritical() && !stateManager.hasError()) {
        stateManager.handleError("Critical battery voltage");
        return;
    }

    // 2. Robot væltet
    #if ENABLE_IMU && AUTO_STOP_ON_TILT
    if (imu.isTilted() && stateManager.isActive()) {
        stateManager.handleError("Robot tilted/flipped");
        return;
    }
    #endif

    // 3. Kritisk forhindring direkte foran
    obstacleAvoid.update(&sensors);
    if (obstacleAvoid.isCriticalObstacle() && stateManager.isActive()) {
        motors.stop();
        Logger::warning("Critical obstacle - stopped");
    }
}

// ============================================================================
// END OF FILE
// ============================================================================
