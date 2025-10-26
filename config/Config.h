#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// ROBOT MOWER CONFIGURATION
// ============================================================================
// Alle konstanter og pin-definitioner for robot plæneklipperen
// ============================================================================

// ============================================================================
// PIN DEFINITIONER - ESP32-S3 (Heltec WiFi Kit 32 V3)
// ============================================================================

// Motor Pins (L298N Motor Driver)
#define MOTOR_LEFT_PWM      5      // PWM til venstre motor hastighed
#define MOTOR_LEFT_IN1      19     // Retning 1 for venstre motor
#define MOTOR_LEFT_IN2      18     // Retning 2 for venstre motor

#define MOTOR_RIGHT_PWM     17     // PWM til højre motor hastighed
#define MOTOR_RIGHT_IN1     16     // Retning 1 for højre motor
#define MOTOR_RIGHT_IN2     15     // Retning 2 for højre motor

// Ultralyd Sensor Pins (HC-SR04)
#define SENSOR_LEFT_TRIG    21     // Venstre sensor trigger
#define SENSOR_LEFT_ECHO    47     // Venstre sensor echo

#define SENSOR_MIDDLE_TRIG  48     // Midter sensor trigger
#define SENSOR_MIDDLE_ECHO  35     // Midter sensor echo

#define SENSOR_RIGHT_TRIG   36     // Højre sensor trigger
#define SENSOR_RIGHT_ECHO   37     // Højre sensor echo

// IMU Pins (MPU-9250/6050 - I2C)
#define IMU_SDA             41     // I2C Data
#define IMU_SCL             42     // I2C Clock

// Klippermotor Pin (Relay)
#define CUTTING_RELAY       38     // Relay til klippermotor

// Batteri Monitoring Pin
#define BATTERY_PIN         1      // ADC pin til batteri spændingsmåling

// Display Pins (OLED - indbygget i Heltec)
// Disse er hardware defineret af Heltec board
#define DISPLAY_SDA         41     // Samme som IMU (delt I2C bus)
#define DISPLAY_SCL         42     // Samme som IMU (delt I2C bus)
#define DISPLAY_RST         -1     // Reset (ikke brugt)

// Status LED (indbygget i Heltec)
#define LED_BUILTIN         35     // Indbygget LED

// ============================================================================
// SENSOR KONSTANTER
// ============================================================================

// Ultralyd sensorer
#define ULTRASONIC_MAX_DISTANCE     200    // Maksimal måleafstand (cm)
#define OBSTACLE_THRESHOLD          30     // Forhindring tærskel (cm)
#define OBSTACLE_CRITICAL           15     // Kritisk forhindring afstand (cm)
#define SENSOR_UPDATE_INTERVAL      100    // Sensor opdaterings interval (ms)
#define SENSOR_TIMEOUT              30000  // Sensor timeout (microsekunder)

// IMU kalibrering
#define IMU_CALIBRATION_SAMPLES     100    // Antal samples til kalibrering
#define IMU_UPDATE_INTERVAL         50     // IMU opdaterings interval (ms)
#define HEADING_TOLERANCE           5.0    // Acceptabel heading afvigelse (grader)

// ============================================================================
// MOTOR KONSTANTER
// ============================================================================

#define MOTOR_MAX_SPEED             255    // Maksimal PWM værdi
#define MOTOR_MIN_SPEED             100    // Minimal PWM for at overvinde friktion
#define MOTOR_CRUISE_SPEED          200    // Normal kørehastighed
#define MOTOR_TURN_SPEED            180    // Hastighed under drejning
#define MOTOR_SLOW_SPEED            150    // Langsom hastighed (nær forhindringer)
#define MOTOR_BACKUP_SPEED          170    // Hastighed ved bakning

#define MOTOR_PWM_FREQUENCY         1000   // PWM frekvens (Hz)
#define MOTOR_PWM_RESOLUTION        8      // PWM opløsning (bits)

// PID konstanter for lige kørsel
#define MOTOR_KP                    2.0    // Proportional gain
#define MOTOR_KI                    0.1    // Integral gain
#define MOTOR_KD                    0.5    // Derivative gain

// ============================================================================
// NAVIGATION KONSTANTER
// ============================================================================

// Klipningsmønster
#define MOWING_PATTERN_WIDTH        30     // Afstand mellem rækker (cm)
#define ROW_LENGTH_MAX              500    // Maksimal række længde før drejning (cm)
#define TURN_ANGLE                  90.0   // Drejnings vinkel (grader)
#define TURN_DURATION               2000   // Estimeret tid for 90° drejning (ms)

// Bevægelse
#define BACKUP_DISTANCE             30     // Afstand at bakke ved forhindring (cm)
#define BACKUP_DURATION             1500   // Tid at bakke (ms)
#define MIN_TURNING_RADIUS          50     // Minimum drejnings radius (cm)

// Path planning
#define PATH_UPDATE_INTERVAL        200    // Path planner opdaterings interval (ms)
#define MAX_ROWS                    50     // Maksimalt antal rækker i mønster

// ============================================================================
// TIMING KONSTANTER
// ============================================================================

#define DISPLAY_UPDATE_INTERVAL     500    // Display opdaterings interval (ms)
#define WEBSOCKET_UPDATE_INTERVAL   200    // WebSocket broadcast interval (ms)
#define STATUS_UPDATE_INTERVAL      1000   // Status opdaterings interval (ms)
#define BATTERY_CHECK_INTERVAL      5000   // Batteri check interval (ms)
#define WATCHDOG_TIMEOUT            10     // Watchdog timeout (sekunder)

// ============================================================================
// BATTERI KONSTANTER
// ============================================================================

#define BATTERY_MAX_VOLTAGE         12.6   // Fuldt ladet (3S LiPo)
#define BATTERY_NOMINAL_VOLTAGE     11.1   // Nominal spænding
#define BATTERY_LOW_VOLTAGE         10.5   // Lav batteri advarsel
#define BATTERY_CRITICAL_VOLTAGE    10.0   // Kritisk batteri - stop operation
#define BATTERY_MIN_VOLTAGE         9.0    // Absolut minimum

// Voltage divider beregning (tilpas efter dit hardware)
#define BATTERY_R1                  10000.0  // Modstand R1 (ohm)
#define BATTERY_R2                  2200.0   // Modstand R2 (ohm)
#define BATTERY_ADC_MAX             4095.0   // 12-bit ADC
#define BATTERY_ADC_VREF            3.3      // Reference spænding (V)

// ============================================================================
// WEB SERVER KONSTANTER
// ============================================================================

#define WEB_SERVER_PORT             80     // HTTP server port
#define WEBSOCKET_PORT              81     // WebSocket port (samme som HTTP normalt)
#define MDNS_HOSTNAME               "robot-mower"  // mDNS navn (robot-mower.local)

#define MAX_WEBSOCKET_CLIENTS       4      // Maksimalt antal WebSocket klienter
#define WEBSOCKET_PING_INTERVAL     30000  // WebSocket ping interval (ms)

// ============================================================================
// LOGGING KONSTANTER
// ============================================================================

#define LOG_BUFFER_SIZE             100    // Antal log entries at gemme
#define SERIAL_BAUD_RATE            115200 // Serial kommunikations hastighed
#define LOG_TO_SERIAL               true   // Log til Serial Monitor
#define LOG_TO_WEBSOCKET            true   // Log til WebSocket klienter

// ============================================================================
// STATE MACHINE KONSTANTER
// ============================================================================

#define STATE_TIMEOUT               300000 // Max tid i en state (5 minutter)
#define CALIBRATION_TIMEOUT         30000  // Kalibrerings timeout (30 sekunder)
#define MOWING_SESSION_MAX          3600000 // Max klipnings session (1 time)

// ============================================================================
// SIKKERHED KONSTANTER
// ============================================================================

#define EMERGENCY_STOP_ENABLED      true   // Aktiver emergency stop
#define AUTO_STOP_ON_TILT           true   // Stop hvis robotten vælter
#define TILT_ANGLE_THRESHOLD        30.0   // Maksimal hældnings vinkel (grader)
#define OBSTACLE_STOP_DISTANCE      10     // Stop afstand ved forhindring (cm)

// ============================================================================
// DEBUG KONSTANTER
// ============================================================================

#define DEBUG_MODE                  true   // Aktiver debug mode
#define DEBUG_SENSORS               true   // Debug sensor readings
#define DEBUG_MOTORS                true   // Debug motor commands
#define DEBUG_NAVIGATION            true   // Debug navigation logic
#define DEBUG_WEBSOCKET             false  // Debug WebSocket messages (verbose)

// ============================================================================
// FEATURE FLAGS
// ============================================================================

#define ENABLE_DISPLAY              true   // Aktiver OLED display
#define ENABLE_IMU                  true   // Aktiver IMU
#define ENABLE_WEBSOCKET            true   // Aktiver WebSocket
#define ENABLE_MDNS                 true   // Aktiver mDNS
#define ENABLE_OTA                  false  // Aktiver OTA updates (fremtidig feature)

// ============================================================================
// RETNING KONSTANTER
// ============================================================================

enum Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    STOP
};

// ============================================================================
// AVOIDANCE DIRECTION KONSTANTER
// ============================================================================

enum AvoidanceDirection {
    AVOID_LEFT,
    AVOID_RIGHT,
    AVOID_BACK
};

#endif // CONFIG_H
