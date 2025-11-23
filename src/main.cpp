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
#include "config/Config.h"
#include "config/Credentials.h"

// System
#include "system/Logger.h"
#include "system/StateManager.h"
#include "system/WiFiManager.h"
#include "system/UpdateManager.h"

// Hardware
#include "hardware/Motors.h"
#include "hardware/Sensors.h"
#include "hardware/IMU.h"
// Display deaktiveret for ESP32-WROOM-32U (ingen indbygget display)
// #include "hardware/Display.h"
#include "hardware/CuttingMechanism.h"
#include "hardware/Battery.h"
#if ENABLE_PERIMETER
#include "hardware/PerimeterReceiver.h"
#include "system/PerimeterClient.h"
#endif

// Navigation
#include "navigation/PathPlanner.h"
#include "navigation/ObstacleAvoidance.h"
#include "navigation/Movement.h"

// Web
#include "web/WebServer.h"
#include "web/WebAPI.h"
#include "web/WebSocket.h"

// Utils
#include "utils/Timer.h"
#include "utils/Math.h"

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
#if ENABLE_PERIMETER
PerimeterReceiver perimeterReceiver;
PerimeterClient perimeterClient;
#endif

// Navigation
PathPlanner pathPlanner;
ObstacleAvoidance obstacleAvoid;
Movement movement;

// Web
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
Timer currentUpdateTimer(100, true); // Strømovervågning hver 100ms
#if ENABLE_PERIMETER
Timer perimeterUpdateTimer(50, true);  // Perimeter opdatering 20Hz
Timer perimeterSenderTimer(5000, true); // Sender status check hver 5 sek
#endif

// Kalibrerings type enum
enum CalibrationType {
    CAL_NONE = 0,
    CAL_GYRO,           // Standard gyro kalibrering
    CAL_MAGNETOMETER    // Magnetometer kalibrering (kræver rotation)
};

// Kalibrerings state
volatile CalibrationType pendingCalibration = CAL_NONE;

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
#if ENABLE_PERIMETER
void updatePerimeter();
void handlePerimeterBoundary();
void handleReturningState();
void handleSearchingSignalState();
void followPerimeterWire();
void searchForPerimeterSignal();
#endif

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

    #if ENABLE_PERIMETER
    if (perimeterUpdateTimer.isExpired()) {
        updatePerimeter();
        perimeterUpdateTimer.reset();
    }

    if (perimeterSenderTimer.isExpired()) {
        perimeterClient.updateStatus();
        perimeterSenderTimer.reset();
    }
    #endif

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

    // Perimeter Wire Receiver
    #if ENABLE_PERIMETER
    Logger::info("Initializing perimeter wire receiver...");
    if (!perimeterReceiver.begin()) {
        Logger::warning("Failed to initialize Perimeter Receiver - continuing without");
    }

    Logger::info("Initializing perimeter wire client...");
    if (!perimeterClient.begin()) {
        Logger::warning("Failed to connect to Perimeter Sender - will retry");
    }
    #endif

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

    #if ENABLE_PERIMETER
    // Set perimeter references for API
    webAPI.setPerimeterReferences(&perimeterReceiver, &perimeterClient);
    #endif

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

        #if ENABLE_PERIMETER
        case STATE_RETURNING:
            handleReturningState();
            break;

        case STATE_SEARCHING_SIGNAL:
            handleSearchingSignalState();
            break;
        #endif

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
    // Håndterer forskellige typer kalibrering
    static bool calibrationStarted = false;
    static CalibrationType currentCalType = CAL_NONE;

    if (!calibrationStarted) {
        // Bestem kalibrerings type
        currentCalType = pendingCalibration;
        if (currentCalType == CAL_NONE) {
            currentCalType = CAL_GYRO; // Default til gyro
        }
        pendingCalibration = CAL_NONE;
        calibrationStarted = true;

        if (currentCalType == CAL_GYRO) {
            Logger::info("Starting gyro calibration - keep robot still!");
            imu.calibrateGyro();
            Logger::info("Gyro calibration complete");
            stateManager.setState(STATE_IDLE);
            calibrationStarted = false;
        }
        else if (currentCalType == CAL_MAGNETOMETER) {
            if (!imu.hasMagnetometer()) {
                Logger::error("Magnetometer not available!");
                stateManager.setState(STATE_IDLE);
                calibrationStarted = false;
                return;
            }
            Logger::info("Starting magnetometer calibration - ROTATE robot slowly!");
            // calibrateMag() blokerer i den angivne tid (30 sek default)
            bool success = imu.calibrateMag(30);
            if (success) {
                Logger::info("Magnetometer calibration complete and saved!");
            } else {
                Logger::error("Magnetometer calibration failed!");
            }
            stateManager.setState(STATE_IDLE);
            calibrationStarted = false;
        }
    }
}

// Funktion til at starte magnetometer kalibrering (kan kaldes fra WebAPI)
void requestMagCalibration() {
    pendingCalibration = CAL_MAGNETOMETER;
    stateManager.setState(STATE_CALIBRATING);
}

// Funktion til at starte gyro kalibrering (kan kaldes fra WebAPI)
void requestGyroCalibration() {
    pendingCalibration = CAL_GYRO;
    stateManager.setState(STATE_CALIBRATING);
}

void handleMowingState() {
    // Opdater path planner
    pathPlanner.update();

    // Tjek for perimeter grænse
    #if ENABLE_PERIMETER
    if (perimeterReceiver.hasSignal()) {
        if (perimeterReceiver.isOutside() || perimeterReceiver.getState() == PERIMETER_ON_WIRE) {
            Logger::info("Perimeter boundary detected!");
            handlePerimeterBoundary();
            return;
        }
    }
    #endif

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

    // 4. Perimeter grænse
    #if ENABLE_PERIMETER
    if (perimeterReceiver.hasSignal() && stateManager.isActive()) {
        if (perimeterReceiver.isOutside()) {
            motors.stop();
            Logger::warning("Outside perimeter - stopped!");
        }
    }
    #endif
}

// ============================================================================
// PERIMETER FUNCTIONS
// ============================================================================

#if ENABLE_PERIMETER
void updatePerimeter() {
    // Opdater perimeter modtager
    perimeterReceiver.update();

    // Debug log
    #if DEBUG_MODE
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug >= 2000) {
        if (perimeterReceiver.hasSignal()) {
            Serial.printf("[Perimeter] %s | Strength: %d%% | Dir: %s | Dist: %dcm\n",
                         perimeterReceiver.getStateString().c_str(),
                         perimeterReceiver.getSignalStrength(),
                         perimeterReceiver.getDirectionString().c_str(),
                         perimeterReceiver.getDistanceToCable());
        } else {
            Serial.println("[Perimeter] No signal");
        }
        lastDebug = millis();
    }
    #endif
}

void handlePerimeterBoundary() {
    // Stop klippermotor
    cuttingMech.stop();

    // Bak væk fra grænsen
    Logger::info("Backing up from perimeter...");
    movement.backUp(PERIMETER_BACKUP_DISTANCE);
    delay(500);

    // Tjek om vi er i et aktivt klipningsmønster
    if (stateManager.getState() == STATE_MOWING && !pathPlanner.isPatternComplete()) {
        // Mønster er aktivt - brug PathPlanner til intelligent drejning
        pathPlanner.perimeterReached();

        // Hent drejningsretning fra PathPlanner
        Direction turnDir = pathPlanner.getTurnDirection();
        Logger::info("Pattern-aware turn: " + String(turnDir == RIGHT ? "RIGHT" : "LEFT"));

        // Start drejning via state machine
        stateManager.setState(STATE_TURNING);
        pathPlanner.startTurn();

        // Clear perimeter trigger efter vi har håndteret det
        pathPlanner.clearPerimeterTrigger();
        return;
    }

    // Ikke i mønster - brug standard drejning baseret på kabel-retning
    PerimeterDirection dir = perimeterReceiver.getDirection();
    if (dir == PERIMETER_LEFT) {
        Logger::info("Perimeter on left - turning right");
        movement.turnInPlace(RIGHT, MOTOR_TURN_SPEED);
    } else if (dir == PERIMETER_RIGHT) {
        Logger::info("Perimeter on right - turning left");
        movement.turnInPlace(LEFT, MOTOR_TURN_SPEED);
    } else {
        // Ukendt retning - drej tilfældigt
        Logger::info("Perimeter direction unknown - turning right");
        movement.turnInPlace(RIGHT, MOTOR_TURN_SPEED);
    }

    // Drej i ca. 135 grader
    delay(1500);
    movement.stop();

    // Vent til vi er sikkert inden for perimeteren
    unsigned long startTime = millis();
    while (millis() - startTime < 2000) {
        perimeterReceiver.update();
        if (perimeterReceiver.isInside()) {
            break;
        }
        delay(50);
    }

    // Fortsæt klipning
    stateManager.setState(STATE_MOWING);
}

// ============================================================================
// RETURNING TO BASE - Følger perimeter wire hjem
// ============================================================================

void handleReturningState() {
    static bool initialized = false;
    static unsigned long lastUpdate = 0;

    if (!initialized) {
        Logger::info("Starting return to base sequence");
        cuttingMech.stop();
        initialized = true;
    }

    // Opdater perimeter ved hver iteration
    perimeterReceiver.update();

    // Tjek om vi har mistet signalet
    if (!perimeterReceiver.hasSignal()) {
        Logger::warning("Lost perimeter signal during return!");
        initialized = false;
        stateManager.searchForSignal();
        return;
    }

    // Følg perimeter wire
    followPerimeterWire();

    // TODO: Tjek om vi er nået frem til ladestationen
    // Dette kræver en sensor eller et stærkere signal ved stationen
    // For nu kører vi bare indtil brugeren stopper

    // Reset initialized når vi forlader staten
    if (stateManager.getState() != STATE_RETURNING) {
        initialized = false;
    }
}

void followPerimeterWire() {
    // Følg kablet ved at holde det på venstre side
    // Kør fremad og juster retning baseret på signalstyrke

    PerimeterState state = perimeterReceiver.getState();
    PerimeterDirection dir = perimeterReceiver.getDirection();
    int signalStrength = perimeterReceiver.getSignalStrength();

    // Målsætning: Hold kablet til venstre, kør langs det
    const int TARGET_STRENGTH = 30;  // Ca. 30cm fra kablet
    const int TOLERANCE = 10;

    if (state == PERIMETER_OUTSIDE) {
        // Vi er udenfor - drej til venstre for at komme ind igen
        movement.turnInPlace(LEFT, MOTOR_SLOW_SPEED);
        delay(200);
    } else if (state == PERIMETER_ON_WIRE) {
        // Vi er på kablet - drej lidt til højre
        movement.turnInPlace(RIGHT, MOTOR_SLOW_SPEED);
        delay(100);
    } else if (state == PERIMETER_INSIDE) {
        // Vi er inden for - juster baseret på signalstyrke
        if (signalStrength > TARGET_STRENGTH + TOLERANCE) {
            // For tæt på kablet - drej lidt væk (højre)
            motors.setSpeed(MOTOR_SLOW_SPEED, MOTOR_SLOW_SPEED - 30);
        } else if (signalStrength < TARGET_STRENGTH - TOLERANCE) {
            // For langt fra kablet - drej mod det (venstre)
            motors.setSpeed(MOTOR_SLOW_SPEED - 30, MOTOR_SLOW_SPEED);
        } else {
            // God afstand - kør lige frem
            motors.setSpeed(MOTOR_SLOW_SPEED, MOTOR_SLOW_SPEED);
        }
    } else {
        // Intet signal - stop og søg
        motors.stop();
        Logger::warning("No signal in followPerimeterWire");
    }
}

// ============================================================================
// SIGNAL SEARCH - Søger efter mistet perimeter signal
// ============================================================================

void handleSearchingSignalState() {
    static bool initialized = false;
    static int searchPhase = 0;
    static unsigned long phaseStartTime = 0;
    static int spiralRadius = 0;

    if (!initialized) {
        Logger::info("Starting perimeter signal search");
        cuttingMech.stop();
        searchPhase = 0;
        spiralRadius = 0;
        phaseStartTime = millis();
        initialized = true;
    }

    // Opdater perimeter
    perimeterReceiver.update();

    // Tjek om vi har fundet signalet
    if (perimeterReceiver.hasSignal()) {
        Logger::info("Perimeter signal found!");
        initialized = false;

        // Gå tilbage til forrige state eller idle
        RobotState prevState = stateManager.getPreviousState();
        if (prevState == STATE_RETURNING) {
            stateManager.setState(STATE_RETURNING);
        } else if (prevState == STATE_MOWING) {
            stateManager.setState(STATE_MOWING);
        } else {
            stateManager.setState(STATE_IDLE);
        }
        return;
    }

    // Udfør søgemønster
    searchForPerimeterSignal();

    // Timeout efter 2 minutter
    if (millis() - phaseStartTime > 120000) {
        Logger::error("Signal search timeout - could not find perimeter!");
        initialized = false;
        stateManager.handleError("Perimeter signal lost");
        return;
    }

    // Reset når vi forlader staten
    if (stateManager.getState() != STATE_SEARCHING_SIGNAL) {
        initialized = false;
    }
}

void searchForPerimeterSignal() {
    // Spiral søgemønster
    // Kør i stadigt større cirkler indtil signal findes

    static int searchStep = 0;
    static unsigned long stepStartTime = 0;
    static float currentHeading = 0;

    unsigned long now = millis();

    // Hver søge-iteration
    if (now - stepStartTime > 500) {
        stepStartTime = now;
        searchStep++;

        // Spiral ud: kør lidt fremad, drej lidt
        int forwardTime = 200 + (searchStep * 10);  // Længere og længere strækninger
        int turnTime = 300;

        // Kør fremad
        movement.driveStraight(MOTOR_SLOW_SPEED);
        delay(min(forwardTime, 2000));

        // Drej til højre (med uret spiral)
        movement.turnInPlace(RIGHT, MOTOR_TURN_SPEED);
        delay(turnTime);

        movement.stop();

        // Log progress
        if (searchStep % 10 == 0) {
            Serial.printf("[Search] Step %d - still searching...\n", searchStep);
        }
    }
}
#endif

// ============================================================================
// END OF FILE
// ============================================================================
