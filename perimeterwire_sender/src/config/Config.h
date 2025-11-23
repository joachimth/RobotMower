#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// PERIMETER WIRE SENDER CONFIGURATION
// ============================================================================
// Konfiguration til perimeterkabel signal sender
// ============================================================================

// ============================================================================
// PIN DEFINITIONER - ESP32
// ============================================================================

// Motor Driver Pins (MC33926 eller L298N)
// PWM output til H-bridge for at generere AC signal på kablet
#define DRIVER_PWM_A        25     // PWM kanal A (IN1)
#define DRIVER_PWM_B        26     // PWM kanal B (IN2)
#define DRIVER_ENABLE       27     // Enable pin (EN)

// Strømmåling
#define CURRENT_SENSE_PIN   34     // ADC til strømmåling (ADC1_CH6, input-only)

// Status LED
#define STATUS_LED_PIN      2      // Indbygget LED på de fleste ESP32 boards

// ============================================================================
// SIGNAL KONSTANTER
// ============================================================================
// Perimeter signal baseret på Ardumower standard
// Signalet er en pseudo-random sekvens ved 3.2 kHz carrier

// Carrier frekvenser
#define SIGNAL_CARRIER_FREQ     3200    // Basis carrier frekvens (Hz)
#define SIGNAL_HIGH_FREQ        4808    // Høj puls frekvens (Hz)
#define SIGNAL_LOW_FREQ         2404    // Lav puls frekvens (Hz)

// Signal timing
#define SIGNAL_BIT_DURATION_US  208     // Bit varighed i microsekunder (1/4808)

// PWM konfiguration
#define PWM_FREQUENCY           25000   // PWM frekvens for motor driver (Hz)
#define PWM_RESOLUTION          8       // PWM opløsning (bits)
#define PWM_CHANNEL_A           0       // LEDC kanal for PWM A
#define PWM_CHANNEL_B           1       // LEDC kanal for PWM B

// Signal duty cycle (justeres via potentiometer på DC/DC converter)
#define SIGNAL_DUTY_CYCLE       128     // 50% duty cycle (0-255)

// ============================================================================
// STRØM OVERVÅGNING
// ============================================================================

#define CURRENT_MAX_MA          1000    // Maksimal strøm (mA) - juster potentiometer!
#define CURRENT_WARNING_MA      800     // Advarsel tærskel (mA)
#define CURRENT_SAMPLES         10      // Antal samples til gennemsnit

// ADC kalibrering for strømmåling
#define CURRENT_SENSE_MV_PER_A  185     // mV per Ampere (ACS712-5A: 185mV/A)
#define CURRENT_SENSE_OFFSET    2500    // Offset i mV (2.5V for ACS712)

// ============================================================================
// TIMING KONSTANTER
// ============================================================================

#define STATUS_UPDATE_INTERVAL  1000    // Status opdatering (ms)
#define CURRENT_CHECK_INTERVAL  100     // Strøm check interval (ms)
#define SIGNAL_HEALTH_INTERVAL  5000    // Signal sundhedstjek (ms)

// ============================================================================
// WEB SERVER KONSTANTER
// ============================================================================

#define WEB_SERVER_PORT         80      // HTTP server port
#define MDNS_HOSTNAME           "perimeter-sender"  // mDNS navn

// ============================================================================
// WIFI KONSTANTER
// ============================================================================

#define AP_SSID                 "PerimeterWire-Setup"
#define AP_PASSWORD             ""      // Åben AP
#define WIFI_CONNECT_TIMEOUT    10000   // WiFi forbindelsestimeout (ms)
#define WIFI_RETRY_INTERVAL     30000   // Retry interval (ms)

// ============================================================================
// SIKKERHED KONSTANTER
// ============================================================================

#define OVERCURRENT_SHUTDOWN    true    // Sluk ved overstrøm
#define THERMAL_PROTECTION      true    // Termisk beskyttelse (MC33926)
#define AUTO_RESTART_DELAY      5000    // Auto genstart efter fejl (ms)

// ============================================================================
// DEBUG KONSTANTER
// ============================================================================

#define DEBUG_MODE              true    // Debug mode
#define SERIAL_BAUD_RATE        115200  // Serial baudrate
#define DEBUG_SIGNAL            false   // Debug signal generator
#define DEBUG_CURRENT           true    // Debug strømmåling

// ============================================================================
// SIGNAL PATTERN
// ============================================================================
// Ardumower kompatibel pseudo-random sekvens
// Dette mønster gentages kontinuerligt

// Signal kode sekvens (24 bits)
// 1 = høj frekvens puls, 0 = lav frekvens puls
#define SIGNAL_CODE_LENGTH      24
static const uint8_t SIGNAL_CODE[SIGNAL_CODE_LENGTH] = {
    1, 1, 0, 0, 0, 1, 0, 0,   // Byte 1
    0, 0, 1, 1, 1, 0, 1, 0,   // Byte 2
    1, 0, 1, 0, 0, 1, 1, 0    // Byte 3
};

// ============================================================================
// SENDER TILSTANDE
// ============================================================================

enum SenderState {
    SENDER_OFF,
    SENDER_STARTING,
    SENDER_RUNNING,
    SENDER_ERROR,
    SENDER_OVERCURRENT
};

#endif // CONFIG_H
