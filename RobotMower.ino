/**
 * ============================================================================
 * ROBOT PLÆNEKLIPPER - ESP32-WROOM-32U
 * ============================================================================
 * Arduino IDE kompatibel fil
 *
 * VIGTIGT: Denne .ino fil er en reference/eksempel fil.
 * Hovedprojektet er et PlatformIO projekt med kode i src/ mappen.
 *
 * For Arduino IDE brug:
 * 1. Kopier denne fil til din Arduino sketchbook mappe
 * 2. Sørg for at hele src/ mappen er tilgængelig
 * 3. Se ARDUINO_IDE_SETUP.md for installation guide
 *
 * BEMÆRK: WebServer klassen er blevet omdøbt til MowerWebServer
 *         for at undgå konflikter med andre biblioteker.
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
#include "src/hardware/CuttingMechanism.h"
#include "src/hardware/Battery.h"

// Navigation
#include "src/navigation/PathPlanner.h"
#include "src/navigation/ObstacleAvoidance.h"
#include "src/navigation/Movement.h"

// Web - BEMÆRK: Bruger MowerWebServer (ikke WebServer)
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
CuttingMechanism cuttingMech;
Battery battery;

// Navigation
PathPlanner pathPlanner;
ObstacleAvoidance obstacleAvoid;
Movement movement;

// Web - VIGTIGT: Brug MowerWebServer (ikke WebServer)
MowerWebServer webServer;
WebAPI webAPI;
WebSocketHandler webSocket;

// Timers
Timer sensorUpdateTimer(SENSOR_UPDATE_INTERVAL, true);
Timer imuUpdateTimer(IMU_UPDATE_INTERVAL, true);
Timer displayUpdateTimer(DISPLAY_UPDATE_INTERVAL, true);
Timer batteryCheckTimer(BATTERY_CHECK_INTERVAL, true);
Timer statusUpdateTimer(STATUS_UPDATE_INTERVAL, true);
Timer websocketUpdateTimer(WEBSOCKET_UPDATE_INTERVAL, true);
Timer currentUpdateTimer(100, true);

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

    // Update WiFi Manager
    #if ENABLE_WIFI_MANAGER
    wifiManager.update();
    #endif

    // Update state manager
    stateManager.update();

    // Run state machine
    runStateMachine();

    // Update web server - BEMÆRK: webServer er en MowerWebServer instans
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

    if (!motors.begin()) {
        Logger::error("Failed to initialize Motors");
        stateManager.handleError("Motor initialization failed");
        return;
    }

    if (!sensors.begin()) {
        Logger::error("Failed to initialize Sensors");
        stateManager.handleError("Sensor initialization failed");
        return;
    }

    #if ENABLE_IMU
    if (!imu.begin()) {
        Logger::warning("Failed to initialize IMU - continuing without");
    }
    #endif

    if (!cuttingMech.begin()) {
        Logger::error("Failed to initialize Cutting Mechanism");
        stateManager.handleError("Cutting mechanism initialization failed");
        return;
    }

    if (!battery.begin()) {
        Logger::error("Failed to initialize Battery monitor");
        stateManager.handleError("Battery monitor initialization failed");
        return;
    }

    Logger::info("Hardware initialized successfully");
}

void initializeNavigation() {
    Logger::info("Initializing navigation...");

    if (!pathPlanner.begin()) {
        Logger::error("Failed to initialize Path Planner");
        return;
    }

    if (!obstacleAvoid.begin()) {
        Logger::error("Failed to initialize Obstacle Avoidance");
        return;
    }

    if (!movement.begin(&motors, &imu)) {
        Logger::error("Failed to initialize Movement controller");
        return;
    }

    Logger::info("Navigation initialized successfully");
}

void initializeWeb() {
    Logger::info("Initializing web server...");

    // WiFi Manager
    #if ENABLE_WIFI_MANAGER
    Logger::info("Initializing WiFi Manager...");
    if (!wifiManager.begin()) {
        Logger::warning("WiFi Manager failed - robot vil køre i AP mode");
    }
    #endif

    // Update Manager
    #if ENABLE_AUTO_UPDATE
    Logger::info("Initializing Update Manager...");
    if (!updateManager.begin()) {
        Logger::warning("Update Manager failed");
    }
    #endif

    // Web Server - BEMÆRK: webServer er en MowerWebServer instans
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

    webAPI.setHardwareReferences(&stateManager, &battery, &sensors, &imu, &motors, &cuttingMech);
    webAPI.setupRoutes();

    // WebSocket
    #if ENABLE_WEBSOCKET
    if (!webSocket.begin(&webServer)) {
        Logger::error("Failed to initialize WebSocket");
        return;
    }

    webSocket.setHardwareReferences(&stateManager, &motors, &cuttingMech);
    #endif

    Logger::info("Web server initialized successfully");

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
    motors.stop();
    cuttingMech.stop();
}

void handleManualState() {
    // Manuel kontrol - motorerne styres direkte via API
}

void handleCalibratingState() {
    static bool calibrationStarted = false;

    if (!calibrationStarted) {
        Logger::info("Starting IMU calibration - keep robot still!");
        imu.calibrate();
        calibrationStarted = true;
        stateManager.setState(STATE_IDLE);
        calibrationStarted = false;
    }
}

void handleMowingState() {
    pathPlanner.update();
    obstacleAvoid.update(&sensors);

    if (obstacleAvoid.hasObstacle()) {
        stateManager.setState(STATE_AVOIDING);
        return;
    }

    if (pathPlanner.shouldTurn()) {
        stateManager.setState(STATE_TURNING);
        pathPlanner.startTurn();
        return;
    }

    float targetHeading = pathPlanner.getTargetHeading();
    movement.setTargetHeading(targetHeading);
    movement.driveStraight(MOTOR_CRUISE_SPEED);

    if (!cuttingMech.isRunning() && !cuttingMech.isSafetyLocked()) {
        cuttingMech.start();
    }
}

void handleTurningState() {
    cuttingMech.stop();

    float targetHeading = pathPlanner.getTargetHeading();
    bool turnComplete = movement.turnToHeading(targetHeading);

    if (turnComplete) {
        pathPlanner.completeTurn();
        pathPlanner.nextRow();

        if (pathPlanner.isPatternComplete()) {
            Logger::info("Mowing pattern complete!");
            stateManager.setState(STATE_IDLE);
        } else {
            stateManager.setState(STATE_MOWING);
        }
    }
}

void handleAvoidingState() {
    cuttingMech.stop();

    AvoidanceDirection avoidDir = obstacleAvoid.getAvoidanceDirection();
    static bool avoidanceComplete = false;

    if (!avoidanceComplete) {
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

    sensors.update();
    obstacleAvoid.update(&sensors);

    if (!obstacleAvoid.hasObstacle()) {
        avoidanceComplete = false;
        stateManager.setState(STATE_MOWING);
    }
}

void handleErrorState() {
    motors.emergencyStop();
    cuttingMech.emergencyStop();
    Logger::error("ERROR STATE: " + stateManager.getErrorMessage());
}

// ============================================================================
// UPDATE FUNCTIONS
// ============================================================================

void updateSensors() {
    sensors.update();

    #if DEBUG_SENSORS
    static int sensorLogCounter = 0;
    if (++sensorLogCounter >= 50) {
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

    if (imu.isTilted() && stateManager.isActive()) {
        Logger::error("Robot tilted - emergency stop!");
        stateManager.handleError("Robot tilted");
    }
    #endif
}

void updateDisplay() {
    // Display ikke tilgængelig på ESP32-WROOM-32U
}

void updateBattery() {
    battery.update();

    if (battery.isCritical()) {
        Logger::error("Critical battery level!");
        stateManager.handleError("Critical battery");
    } else if (battery.isLow() && stateManager.isActive()) {
        Logger::warning("Low battery - returning to base");
        stateManager.setState(STATE_RETURNING);
    }
}

void updateWebStatus() {
    #if ENABLE_WEBSOCKET
    String statusJSON = webAPI.createStatusJSON();
    webSocket.broadcastStatus(statusJSON);

    webSocket.broadcastSensorData(
        sensors.getLeftDistance(),
        sensors.getMiddleDistance(),
        sensors.getRightDistance()
    );
    #endif
}

void updateMotorCurrent() {
    motors.updateCurrentReadings();

    if (motors.isCurrentWarning() && stateManager.isActive()) {
        Logger::warning("Motor current warning! Left: " +
                       String(motors.getLeftCurrent(), 2) + "A, Right: " +
                       String(motors.getRightCurrent(), 2) + "A");
    }
}

void checkSafetyConditions() {
    if (battery.isCritical() && !stateManager.hasError()) {
        stateManager.handleError("Critical battery voltage");
        return;
    }

    #if ENABLE_IMU && AUTO_STOP_ON_TILT
    if (imu.isTilted() && stateManager.isActive()) {
        stateManager.handleError("Robot tilted/flipped");
        return;
    }
    #endif

    obstacleAvoid.update(&sensors);
    if (obstacleAvoid.isCriticalObstacle() && stateManager.isActive()) {
        motors.stop();
        Logger::warning("Critical obstacle - stopped");
    }
}

// ============================================================================
// END OF FILE
// ============================================================================
