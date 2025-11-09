#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// ROBOT MOWER CONFIGURATION
// ============================================================================
// Alle konstanter og pin-definitioner for robot plæneklipperen
// ============================================================================

// ============================================================================
// PIN DEFINITIONER - ESP32-WROOM-32U 38-Pin
// ============================================================================
// Pin layout optimeret så hver komponent bruger tilstødende pins
// Se PINOUT.md for detaljeret pinout diagram

// Motor Pins (Double BTS7960 43A H-bridge High-power Motor Driver)
// Venstre motor driver - Grupperet på GPIO 32-35 (tilstødende pins)
#define MOTOR_LEFT_RPWM     32     // PWM til venstre motor fremad
#define MOTOR_LEFT_LPWM     33     // PWM til venstre motor baglæns
#define MOTOR_LEFT_R_EN     25     // Enable for højre side (fremad)
#define MOTOR_LEFT_L_EN     26     // Enable for venstre side (baglæns)
#define MOTOR_LEFT_R_IS     34     // Strømsensor højre side (ADC1_CH6, input-only)
#define MOTOR_LEFT_L_IS     35     // Strømsensor venstre side (ADC1_CH7, input-only)

// Højre motor driver - Grupperet på GPIO 13-14, 18, 27, 36, 39
#define MOTOR_RIGHT_RPWM    27     // PWM til højre motor fremad
#define MOTOR_RIGHT_LPWM    14     // PWM til højre motor baglæns
#define MOTOR_RIGHT_R_EN    18     // Enable for højre side (fremad) - FLYTTET FRA GPIO 12
#define MOTOR_RIGHT_L_EN    13     // Enable for venstre side (baglæns)
#define MOTOR_RIGHT_R_IS    36     // Strømsensor højre side (ADC1_CH0, input-only, VP)
#define MOTOR_RIGHT_L_IS    39     // Strømsensor venstre side (ADC1_CH3, input-only, VN)

// Ultralyd Sensor Pins (HC-SR04) - Grupperet sammen
#define SENSOR_LEFT_TRIG    15     // Venstre sensor trigger
#define SENSOR_LEFT_ECHO    2      // Venstre sensor echo (ADC2_CH2)

#define SENSOR_MIDDLE_TRIG  4      // Midter sensor trigger
#define SENSOR_MIDDLE_ECHO  16     // Midter sensor echo

#define SENSOR_RIGHT_TRIG   17     // Højre sensor trigger
#define SENSOR_RIGHT_ECHO   5      // Højre sensor echo

// IMU Pins (MPU-9250/6050 - I2C) - Standard ESP32 I2C pins
#define IMU_SDA             21     // I2C Data (standard)
#define IMU_SCL             22     // I2C Clock (standard)

// Klippermotor Pin (Relay)
#define CUTTING_RELAY       23     // Relay til klippermotor

// Batteri Monitoring Pin
#define BATTERY_PIN         19     // ADC pin til batteri spændingsmåling (ADC2_CH8)

// Display - IKKE I BRUG (ESP32-WROOM-32U har ikke indbygget display)
// Display funktionalitet er deaktiveret i denne version
// #define DISPLAY_SDA         21     // Ville dele I2C med IMU
// #define DISPLAY_SCL         22     // Ville dele I2C med IMU

// Status LED (kan tilsluttes eksternt hvis ønsket)
// #define LED_BUILTIN         18     // Nu brugt til MOTOR_RIGHT_R_EN
// Hvis du vil have status LED, brug GPIO 0 eller en anden ledig pin

// ============================================================================
// SENSOR KONSTANTER
// ============================================================================

// Ultralyd sensorer
#define ULTRASONIC_MAX_DISTANCE     200    // Maksimal måleafstand (cm)
#define OBSTACLE_THRESHOLD          30     // Forhindring tærskel (cm)
#define OBSTACLE_CRITICAL           15     // Kritisk forhindring afstand (cm)
#define OBSTACLE_CRITICAL_DISTANCE  15     // Alias for konsistens
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

// Strømovervågning (BTS7960)
#define MOTOR_CURRENT_MAX           43.0   // Maksimal strøm pr. driver (A)
#define MOTOR_CURRENT_WARNING       35.0   // Advarsel tærskel (A)
#define MOTOR_CURRENT_SAMPLES       10     // Antal samples til gennemsnit
#define CURRENT_SENSE_RATIO         0.01   // 10mV/A fra BTS7960
#define CURRENT_ADC_VREF            3.3    // ADC reference spænding (V)
#define CURRENT_ADC_MAX             4095.0 // 12-bit ADC

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
// OBS: Motorer og klippemotor forsynes separat med 18V (5S LiPo)
// Styringsdelen (ESP32) forsynes med 12V (3S LiPo) via regulator

#define BATTERY_MAX_VOLTAGE         12.6   // Fuldt ladet (3S LiPo for styring)
#define BATTERY_NOMINAL_VOLTAGE     11.1   // Nominal spænding
#define BATTERY_LOW_VOLTAGE         10.5   // Lav batteri advarsel
#define BATTERY_CRITICAL_VOLTAGE    10.0   // Kritisk batteri - stop operation
#define BATTERY_MIN_VOLTAGE         9.0    // Absolut minimum

// Motor batteri (18V - 5S LiPo) - kun til reference
#define MOTOR_BATTERY_MAX_VOLTAGE   21.0   // Fuldt ladet (5S LiPo)
#define MOTOR_BATTERY_NOMINAL       18.5   // Nominal spænding

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

#define ENABLE_DISPLAY              false  // Deaktiveret - ESP32-WROOM-32U har ikke display
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
