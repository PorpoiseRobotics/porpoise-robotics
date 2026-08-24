/*
  Config.h - Pathfinder Op Program 12

  Every tunable number and every custom type lives here, so the rest of the
  program contains logic and nothing else. If you want to change how the
  vehicle behaves, this is almost certainly the file you want.

  This is a real header file rather than another tab because the enums and
  structs below have to be defined before anything uses them. Arduino writes
  function prototypes for you and puts them at the top of the sketch, which
  would land above a type declared in a tab; putting the types in a header
  that the main sketch includes first avoids that trap entirely.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===================================================================
// MOTORS
// ===================================================================

const int PWM_FREQ = 30000;   // 30 kHz, above hearing, so the motors run quietly
const int PWM_RES  = 10;      // 10-bit resolution: duty values 0..1023
const int PWM_MAX  = 1023;

// Ramping smooths speed changes so the vehicle does not lurch.
const float RAMP_FACTOR   = 0.5f;  // Fraction of the remaining gap closed each step
const int   RAMP_DELAY_MS = 10;    // How often a new speed is calculated

struct Motor {
  int     in1_pin;
  int     in2_pin;
  uint8_t in1_channel;   // The ESP32 drives PWM through numbered channels
  uint8_t in2_channel;
};

// ===================================================================
// INPUT
// ===================================================================

const int   AXIS_MAX      = 511;   // Bluepad32 normalises every stick to -512..511
const int   DEFAULT_DEADZONE = 32;
const int   DEADZONE_MIN  = -1;    // -1 on a per-axis deadzone means "use the global one"
const int   DEADZONE_MAX  = 511;
const float SCALE_MIN     = 0.2f;
const float SCALE_MAX     = 1.2f;

// Throttle modes are toggled with the left thumbstick button (L3).
enum ThrottleMode { MODE_FAST, MODE_NORMAL };

const float NORMAL_MODE_X_SCALE = 0.5f;  // Reduced steering authority
const float NORMAL_MODE_Y_SCALE = 1.0f;

// ===================================================================
// SERVOS
// ===================================================================

const int SERVO1_PIN = 25, SERVO1_CH = 8;   // Right stick Y up
const int SERVO2_PIN = 26, SERVO2_CH = 9;   // Right stick X left
const int SERVO3_PIN = 27, SERVO3_CH = 10;  // Right stick Y down
const int SERVO4_PIN = 14, SERVO4_CH = 11;  // Right stick X right

const int SERVO_FREQ = 50;   // One pulse every 20 ms
const int SERVO_RES  = 16;   // 16-bit duty resolution

// Pulse widths in microseconds. A standard hobby servo sweeps its full travel
// between 1.0 ms and 2.0 ms; 11.2 used 0.5 ms to 2.5 ms, which can drive a
// servo into its mechanical end stops.
const int SERVO_MIN_US = 1000;
const int SERVO_MID_US = 1500;
const int SERVO_MAX_US = 2000;

// ===================================================================
// LEDS
// ===================================================================

const int LED_PIN   = 5;
const int NUM_LEDS  = 32;

// The strip is one loop around the vehicle: 0-15 across the front left to
// right, then 16-31 across the rear right to left. The LED directly behind
// front LED p is always 31 - p.
const int LEFT_FRONT_START  = 0;
const int RIGHT_FRONT_START = 8;
const int RIGHT_REAR_START  = 16;
const int LEFT_REAR_START   = 24;
const int CORNER_LEN        = 8;
const int FRONT_LAST        = 15;
const int REAR_LAST         = 31;

enum LEDMode { DISCONNECTED, STANDBY, TURN_LEFT, TURN_RIGHT, PAIRING, KITT_SCANNER };

const uint8_t HEADLIGHT_DIM    = 77;
const uint8_t HEADLIGHT_BRIGHT = 255;
const uint8_t TAILLIGHT_LEVEL  = 77;

// Turn signals
const int LARSON_DELAY_MS = 50;
const int TURN_CYCLES     = 3;    // Cycles to finish after the D-pad is released

// Pairing breathe
const int BREATHE_PERIOD_MS = 2000;
const uint8_t BREATHE_MAX   = 77;   // About 30% brightness

// KITT scanner: one cycle is left to right and back again
const int     KITT_SPEED_MS   = 35;
const int     KITT_CYCLES     = 3;
const int     KITT_TAIL       = 3;    // Fading LEDs trailing the bright one
const uint8_t KITT_BRIGHTNESS = 200;
const uint8_t KITT_R = 0, KITT_G = 0, KITT_B = 255;   // Blue. Try 255,0,0 for Knight Rider red.

// ===================================================================
// CURRENT SENSOR (INA219, Gen 3 vehicles only)
// ===================================================================

const uint8_t INA219_I2C_ADDR = 0x40;
const int     I2C_SDA_PIN     = 32;
const int     I2C_SCL_PIN     = 33;
const float   SHUNT_RESISTOR_OHMS = 0.0025f;

// ===================================================================
// VEHICLE CAPABILITIES
// ===================================================================

// Bit flags, so one binary can serve more than one generation of vehicle.
enum VehicleCapability {
  CAP_SELF_TEST   = 1 << 0,
  CAP_CURRENT_MON = 1 << 1
};

struct VehicleConfig {
  uint8_t capabilities;
};

// ===================================================================
// SELF-TEST (Gen 3 vehicles only)
// ===================================================================

// Jumper this pin high at boot to run the self-test. The board carries an
// external pull-down; GPIO 34 is input-only and has no internal pull resistors.
const int TEST_PIN = 34;

const int TEST_START_DELAY   = 1000;
const int BASELINE_DURATION  = 500;   // Time spent measuring current with motors off
const int SPIN_DURATION      = 300;   // Time each motor is driven during its test
const int BRAKE_DURATION     = 200;
const int PASS_BLINK_DELAY   = 250;

// A healthy motor draws a clear spike above its resting draw when it spins up,
// then settles. Both thresholds are measured ABOVE the baseline reading, so a
// vehicle with noisy electronics does not fail every motor.
const float PEAK_ABOVE_BASELINE_A = 1.500f;
const float AVG_ABOVE_BASELINE_A  = 1.000f;

enum SelfTestState {
  CHECK_TRIGGER,
  TEST_DISABLED,
  START_TEST,
  BASELINE_CURRENT,
  RUNNING_TESTS,
  TEST_RESULT
};

struct TestStep {
  uint8_t     motor_index;
  int8_t      direction;    // 1 forward, -1 reverse
  const char *name;
};

// ===================================================================
// BLUETOOTH
// ===================================================================

const int PAIR_TRIGGER_PIN = 0;   // GPIO 0 is the BOOT button on the dev board

// ===================================================================
// STORED SETTINGS (EEPROM)
// ===================================================================

const int EEPROM_SIZE = 64;

const int ADDR_DEADZONE       =  0;
const int ADDR_DEADZONE_LS_X  =  4;
const int ADDR_DEADZONE_LS_Y  =  8;
const int ADDR_DEADZONE_RS_X  = 12;
const int ADDR_DEADZONE_RS_Y  = 16;
const int ADDR_LS_Y_SC        = 20;
const int ADDR_LS_X_SC        = 24;
const int ADDR_RS_Y_SC        = 28;
const int ADDR_RS_X_SC        = 32;
const int ADDR_PAIRED_VALID   = 36;   // 1 byte: 0xA5 when a paired address is stored
const int ADDR_PAIRED_ADDR    = 37;   // 6 bytes: the paired controller's address

const uint8_t PAIRED_VALID_MAGIC = 0xA5;

// ===================================================================
// DIAGNOSTICS
// ===================================================================

const unsigned long DIAG_INTERVAL_MS      = 5000;
const unsigned long JOY_REPORT_INTERVAL_MS = 250;   // 4 Hz

#endif  // CONFIG_H
