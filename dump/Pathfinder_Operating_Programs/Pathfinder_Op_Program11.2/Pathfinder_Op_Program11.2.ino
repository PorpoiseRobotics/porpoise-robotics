/**
 * Pathfinder.ino
 * Pathfinder vehicle controlled by a Bluetooth gamepad via Bluepad32.
 * Features motor ramping, coast timeout, and WS2812B LED lighting
 * with startup sequence, connection status, and animated turn signals.
 *
 * Hardware:
 * - ESP32 microcontroller
 * - INA219 Voltage and current sensor
 * - 4 x DC brushed motors driven by TI DRV8871 H-bridges (30 kHz PWM)
 * - 32 x WS2812B addressable LEDs in series (data on GPIO 5)
 * - Jumper IO34 to P3V3 to initiate self-test sequence.
 *
 * - Left thumbstick:
 *   - -Y Forward
 *   - +Y Reverse
 *   - -X Left
 *   - +X Right
 * - Right thumbstick:
 *   - -Y Servo 1: 0° to 180°
 *   - +Y Servo 3: 180° to 0°
 *   - -X Servo 2: 180° to 0°
 *   - +X Servo 4: 0° to 180°
 * - D-pad UP:   Front headlights full bright white (100%)
 * - D-pad DOWN: Front headlights back to default
 * - D-pad LEFT: 3 cycles of left turn signal, or hold for extended signal.
 * - D-pad RIGHT: 3 cycles of right turn signal, or hold for extended signal.
 * - Button (Square/Y): Toggle headlights + rear lights on/off
 * - Button (Triangle/X): Trigger KITT scanner
 * - Left Stick Button (L3) cycles between the following throttle scale settings:
 *   - 1. LS_X_SC = LS_Y_SC = 1.0
 *   - 2. Default LS_X_SC = 0.5, LS_Y_SC = 1.0
 *
 * - Bluetooth pairing can be initiated by pressing the 'boot' button on the ESP32 module, or by sending 'pair' in the serial console.
 *
 *   Bluepad32.h: Add the following to the Boards manager (File --> Preferences, and add to "additional boards manager URLs":
 *   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json,https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
 *   Then install esp32_bluepad32 by Ricardo Quesada and esp32 by Espressif Systems
 *   Next, select ESP32 Wrover Module from tools --> Board --> esp32_bluepad32 --> ESP32 Wrover Module
 *
 * Authors: Ken Masterson & Valentino Farre
 * Date: December 2025
 * 
 * - Change Log
 * - 2025-12-27
 *  * Added KITT car LED function
 *  * Changed names of throttle modes and set default to reduced steering
**/
#include <esp_now.h>
#include <WiFi.h>
#include <Bluepad32.h> // Bluetooth Gamepad support
#include <FastLED.h>   // Addressable LED (WS2812B) control
#include <Wire.h>      // I2C communication for the current sensor
#include <EEPROM.h>    // Permanent storage for settings (deadzone/scale)
#include <esp_mac.h>   // To get the unique Bluetooth address of the ESP32
#include <uni.h>  // Required for uni_bt_allowlist_* functions and bd_addr_t

// LED Index Mapping: Defines which LEDs in the strip belong to which corner
#define LEFT_FRONT_START   0
#define LEFT_FRONT_END     8
#define RIGHT_FRONT_START  8
#define RIGHT_FRONT_END   16
#define RIGHT_REAR_START  16
#define RIGHT_REAR_END    24
#define LEFT_REAR_START   24
#define LEFT_REAR_END     32

// ===================================================================
// CORE CONSTANTS & PERSISTENT SETTINGS
// ===================================================================

const int PWM_FREQ = 30000; // 30kHz frequency (above human hearing to avoid whining)
const int PWM_RES  = 10;    // 10-bit resolution (0 to 1023) for fine speed control
const int PWM_MAX  = 1023;   

// Input handling: Deadzone prevents "ghost" movement if the joystick doesn't center perfectly
int INPUT_DEADZONE = 32; // Global fallback deadzone
int INPUT_DEADZONE_LS_X = -1;  // Left stick X, -1 = use global
int INPUT_DEADZONE_LS_Y = -1;  // Left stick Y, -1 = use global
int INPUT_DEADZONE_RS_X = -1;  // Right stick X, -1 = use global
int INPUT_DEADZONE_RS_Y = -1;  // Right stick Y, -1 = use global

// Input handling: Scale values and throttle states
float ls_y_sc = 1.0f;
float ls_x_sc = 1.0f;
float rs_y_sc = 1.0f;
float rs_x_sc = 1.0f;
float active_ls_y_sc = 1.0f;
float active_ls_x_sc = 0.5f;
enum ThrottleMode { MODE_FAST, MODE_NORMAL };
ThrottleMode throttle_mode = MODE_NORMAL;

// EEPROM Addresses: Where we store settings so they remain after power-off
#define EEPROM_SIZE 64 			// 64 bytes of storage reserved
#define ADDR_DEADZONE 		0 	// EEPROM start page, slot 0.
#define ADDR_DEADZONE_LS_X  4
#define ADDR_DEADZONE_LS_Y  8
#define ADDR_DEADZONE_RS_X 12
#define ADDR_DEADZONE_RS_Y 16
#define ADDR_LS_Y_SC       20
#define ADDR_LS_X_SC       24
#define ADDR_RS_Y_SC       28
#define ADDR_RS_X_SC       32

#define PAIR_TRIGGER_PIN 0  // GPIO0 pulled low to trigger pairing

unsigned long last_joy_report_time = 0;
const unsigned long JOY_REPORT_INTERVAL_MS = 250;  // 4 Hz = 250 ms
uint32_t prev_reported_buttons = 0;  // To detect button changes
uint8_t prev_dpad = 0;

// ===================================================================
// VEHICLE CONFIGURATION
// ===================================================================

/**
 * BITMASKING: We use bits to toggle features. 
 * CAP_SELF_TEST is bit 0 (value 1), CAP_CURRENT_MON is bit 1 (value 2).
 */
enum VehicleCapability {
  CAP_SELF_TEST   = 1 << 0,
  CAP_CURRENT_MON = 1 << 1
};

struct VehicleConfig {
  uint8_t capabilities;
};

// Define different hardware versions
const VehicleConfig VEHICLE_GEN2 = { 0 }; // Basic model
const VehicleConfig VEHICLE_GEN3 = { CAP_SELF_TEST | CAP_CURRENT_MON };

VehicleConfig vehicle_config = VEHICLE_GEN2;

// ===================================================================
// MOTOR CONFIGURATION
// ===================================================================

struct Motor {
  int in1_pin; 
  int in2_pin; 
  uint8_t in1_channel; // ESP32 uses PWM "channels" rather than just pins
  uint8_t in2_channel;
};

Motor motors[4] = {
  {13, 12, 0, 1}, // Front Left, GPIO pins 13 & 12. Uses PWM hardware channels 0 & 1.
  {23, 22, 2, 3}, // Front Right, GPIO pins 23 & 22. Uses PWM hardware channels 2 & 3.
  {19, 18, 4, 5}, // Rear Left, GPIO pins 19 & 18. Uses PWM hardware channels 4 & 5.
  {17, 16, 6, 7}  // Rear Right, GPIO pins 17 & 16. Uses PWM hardware channels 6 & 7.
};

// Ramping variables: Prevents the robot from jerking by smoothing speed changes
int target_left = 0, target_right = 0; // 'Command' from control theory.
int current_left = 0, current_right = 0;
const float RAMP_FACTOR = 0.5; // Lower is smoother/slower acceleration
const int RAMP_DELAY_MS = 10; // How frequently the new speed is calculated, in mS.
unsigned long last_ramp_time = 0; // Time in mS since last speed update.

// ===================================================================
// SERVO CONFIGURATION (Right Joystick Control)
// ===================================================================

// Servo pins
#define SERVO1_PIN  25  // Right Stick Y down
#define SERVO2_PIN  26  // Right Stick X left
#define SERVO3_PIN  27  // Right Stick Y up (inverted)
#define SERVO4_PIN  14  // Right Stick X right (inverted)

// Use LEDC channels 8–11 for servos (0–7 used by motors)
#define SERVO1_CH   8
#define SERVO2_CH   9
#define SERVO3_CH   10
#define SERVO4_CH   11

const int SERVO_FREQ = 50;     // Standard servo frequency
const int SERVO_RES  = 16;     // 16-bit resolution for fine control
const int SERVO_MIN  = 1638;   // ~1.0ms pulse ( 0° )  @ 50Hz, 16-bit
const int SERVO_MAX  = 8192;   // ~2.0ms pulse ( 180° )
const int SERVO_MID  = 4915;   // ~1.5ms pulse ( 90° )

// ===================================================================
// LED CONFIGURATION
// ===================================================================

#define NUM_LEDS 32 
#define LED_PIN 5 
#define LED_TYPE WS2812B 
#define COLOR_ORDER GRB // Sequence in the array for each of the colors that the RGB LED decodes.
CRGB leds[NUM_LEDS]; 

// LED States
enum LEDMode { DISCONNECTED, STANDBY, MOVING, TURN_LEFT, TURN_RIGHT, PAIRING, KITT_SCANNER };
LEDMode led_mode = DISCONNECTED;
volatile bool controller_now_connected = false;

bool full_headlights = false;
bool running_lights_on = true;
int disconnected_brightness = 38;
unsigned long larson_time = 0;
const int LARSON_DELAY_MS = 50;
int larson_phase = 0;

int turn_signal_cycles = 0;
const int CYCLES_TO_RUN = 3;

bool prev_square = false;

unsigned long breathe_time = 0;
const int BREATHE_PERIOD_MS = 2000;  // Full breathe cycle = 2 seconds (adjust as needed)

// ===================================================================
// ADJUSTABLE SETTINGS: KITT SCANNER
// ===================================================================

// KITT Scanner runs when KITT button is PRESSED (edge-triggered).
// One cycle = left -> right -> left

const CRGB KITT_COLOR = CRGB(0, 0, 255);    // Scanner color
const int KITT_SPEED_MS = 35;              // Speed: milliseconds per step (lower = faster)
const int KITT_CYCLES = 3;                 // Number of full cycles to run
const uint8_t KITT_BRIGHTNESS = 77;        // 0..255 brightness cap for scanner

// KITT trigger button bitmask
const uint32_t KITT_BUTTON_MASK = 0x0008; // 0x0008 is the X button on the Nintendo Switch controller

// KITT runtime state
unsigned long kitt_time = 0;    // saves time of last KITT update
int kitt_pos = 0;                       // current light position varies 0..15
int kitt_dir = 1;                       // +1 or -1
int kitt_cycles_done = 0;               // competed full cycles
bool prev_kitt_btn = false;             // button is initially not pressed
LEDMode kitt_return_mode = STANDBY;      // program returns to standby lighting by default

// ===================================================================
// CONTROLLER BUTTON SOFT-TOGGLE INITIALIZATION
// ===================================================================

bool prev_thumbL = false;  // For detecting L3 press (edge)

uint32_t prev_buttons = 0;

// ===================================================================
// INA219 (Current Sensor) CONFIGURATION
// ===================================================================

#define INA219_I2C_ADDR   0x40 // I2C address
#define I2C_SDA_PIN       32 
#define I2C_SCL_PIN       33 

const float SHUNT_RESISTOR_OHMS = 0.0025; // Physical resistor value on the sensor board
float current_sum = 0.0;
int   current_count = 0;
float current_peak_pos = 0.0;
float current_peak_neg = 0.0;

bool diag_enabled = false; // Set to true via Serial to see live data
unsigned long last_diag_time = 0;
unsigned long last_joy_diag_time = 0;

// ===================================================================
// BLUEPAD32 HANDLERS (Bluetooth Events)
// ===================================================================

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

void onConnectedGamepad(GamepadPtr gp) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == nullptr) {
      myGamepads[i] = gp;
      controller_now_connected = true;
      led_mode = STANDBY;
      set_standby_lighting();

      // First add the address
      ControllerProperties props = gp->getProperties();
      uni_bt_allowlist_add_addr(props.btaddr);

      Serial.print(F("Added controller to allowlist: "));
      for (int j = 5; j >= 0; j--) {
        if (props.btaddr[j] < 0x10) Serial.print("0");
        Serial.print(props.btaddr[j], HEX);
        if (j > 0) Serial.print(":");
      }
      Serial.println();

      // THEN re-enable allowlist
      uni_bt_allowlist_set_enabled(true);
      Serial.println(F("Re-enabled Bluetooth allowlist."));

      // Lock down new connections
      BP32.enableNewBluetoothConnections(false);

      break;
    }
  }
}

void onDisconnectedGamepad(GamepadPtr gp) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == gp) {
      myGamepads[i] = nullptr;
      break;
    }
  }
}

// ===================================================================
// SELF-TEST CONFIGURATION
// ===================================================================

#define TEST_PIN 34 // If this pin is HIGH at boot, the self-test runs
const int TEST_START_DELAY = 1000;
const int BASELINE_DURATION = 500; // Time to measure current with motors OFF
const int SPIN_DURATION     = 300; // Time to spin each motor during test
const int BRAKE_DURATION    = 200; // Time to wait after braking

// Thresholds for a "healthy" motor: 
// Must draw some current when spinning up the motors (peak > 1.5A) but not stall (avg < 1.0A)
const float PEAK_THRESHOLD_A = 1.500;   
const float AVG_THRESHOLD_A  = 1.000;
const int PASS_BLINK_DELAY = 250;

// State Machine for the Self-Test sequence
enum SelfTestState {
  CHECK_TRIGGER,    // Waiting to see if we should test
  TEST_DISABLED,    // Normal driving mode
  START_TEST,       // Initializing test
  BASELINE_CURRENT, // Measuring "quiet" power draw
  RUNNING_TESTS,    // Spinning motors one by one
  TEST_RESULT       // Showing Pass (Green) or Fail (Red)
};

SelfTestState test_state = CHECK_TRIGGER;
unsigned long test_phase_start = 0;

struct TestStep {
  uint8_t motor_index; // Tracks which motor {0, 1, 2, 3}.
  int8_t  direction; // 1 for forward, -1 for reverse.
  const char* name; // The human-readable name of this test? (e.g., "FL Forward")
};

// Sequence of motor movements to verify 4-wheel drive integrity
const TestStep test_sequence[] = {
  {0,  1, "FL Forward "}, {0, -1, "FL Reverse "},
  {1,  1, "FR Forward "}, {1, -1, "FR Reverse "},
  {3,  1, "RR Forward "}, {3, -1, "RR Reverse "},
  {2,  1, "RL Forward "}, {2, -1, "RL Reverse "}
};
const uint8_t NUM_TEST_STEPS = sizeof(test_sequence) / sizeof(test_sequence[0]);

uint8_t current_test_step = 0;
float baselineCurrent = 0.0;
bool self_test_passed = true;

// ===================================================================
// INA219 FUNCTIONS
// ===================================================================

// Checks if the sensor is actually wired up
bool is_ina219_present() {
  Wire.beginTransmission(INA219_I2C_ADDR);
  return Wire.endTransmission() == 0;
}

// Sends 16-bit data to the sensor
void ina219_write16(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value >> 8);   // High byte
  Wire.write(value & 0xFF); // Low byte
  Wire.endTransmission();
}

// Reads 16-bit data from the sensor
uint16_t ina219_read16(uint8_t reg) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)INA219_I2C_ADDR, (uint8_t)2);
  return Wire.available() >= 2 ? (Wire.read() << 8) | Wire.read() : 0;
}

void ina219_init() {
  ina219_write16(0x00, 0x8000); // Reset the sensor
  delay(1);
  // Calibration for 32V, 2A range (See INA219 datasheet for bit meanings)
  ina219_write16(0x00, 0b0001100111001111);
  ina219_write16(0x05, 20999);
}

void reset_current_tracking() {
  current_sum = 0;
  current_count = 0;
  current_peak_pos = 0.0;
  current_peak_neg = 0.0;
}

// Calculate the average and peak current since the last reset
void update_ina219() {
  if (!(vehicle_config.capabilities & CAP_CURRENT_MON)) return;
  int16_t raw = (int16_t)ina219_read16(0x01);
  float current_a = (raw * 0.01) / (SHUNT_RESISTOR_OHMS * 1000.0);
  
  if (current_a > current_peak_pos) current_peak_pos = current_a;
  if (current_a < current_peak_neg) current_peak_neg = current_a;
  current_sum += current_a;
  current_count++;
}

float get_bus_voltage() {
  uint16_t value = ina219_read16(0x02);
  return (float)((value >> 3) * 4) * 0.001; // Convert raw bits to Volts
}

// ===================================================================
// MOTOR CONTROL FUNCTIONS
// ===================================================================

// Stop all motors and let them roll to a stop
void coast_all() {
  for (int i = 0; i < 4; i++) {
    ledcWrite(motors[i].in1_channel, 0);
    ledcWrite(motors[i].in2_channel, 0);
  }
  current_left = current_right = 0;
}

// Full power for the self-test
void set_motor_full(int motor_index, int direction) {
  if (direction > 0) {
    ledcWrite(motors[motor_index].in1_channel, PWM_MAX);
    ledcWrite(motors[motor_index].in2_channel, 0);
  } else if (direction < 0) {
    ledcWrite(motors[motor_index].in1_channel, 0);
    ledcWrite(motors[motor_index].in2_channel, PWM_MAX);
  } else {
    ledcWrite(motors[motor_index].in1_channel, 0);
    ledcWrite(motors[motor_index].in2_channel, 0);
  }
}

// Short-circuits the motor to make it stop instantly
void active_brake_motor(int motor_index) {
  ledcWrite(motors[motor_index].in1_channel, PWM_MAX);
  ledcWrite(motors[motor_index].in2_channel, PWM_MAX);
}

/**
 * HYBRID DRIVE: Uses a clever PWM technique.
 * One pin is held HIGH while the other pin pulses LOW.
 * This provides better torque at low speeds compared to standard PWM.
 */
void set_motor_hybrid(uint8_t in1_ch, uint8_t in2_ch, int duty) {
  duty = constrain(duty, -PWM_MAX, PWM_MAX);
  if (duty == 0) {
    ledcWrite(in1_ch, 0);
    ledcWrite(in2_ch, 0);
    return;
  }
  int abs_duty = abs(duty);
  int inv = PWM_MAX - abs_duty; // The "inverse" PWM signal
  if (duty > 0) {
    ledcWrite(in1_ch, PWM_MAX);
    ledcWrite(in2_ch, inv);
  } else {
    ledcWrite(in1_ch, inv);
    ledcWrite(in2_ch, PWM_MAX);
  }
}

// Updates physical motor speeds based on the "current_left/right" variables
void update_motors() {
  if (test_state == TEST_DISABLED) {
    set_motor_hybrid(motors[0].in1_channel, motors[0].in2_channel, current_left);
    set_motor_hybrid(motors[2].in1_channel, motors[2].in2_channel, current_left);
    set_motor_hybrid(motors[1].in1_channel, motors[1].in2_channel, current_right);
    set_motor_hybrid(motors[3].in1_channel, motors[3].in2_channel, current_right);
  }
}

// ===================================================================
// LED ANIMATION FUNCTIONS
// ===================================================================

// Soft green glow when no controller is found
void set_disconnected_lighting() {
  fill_solid(leds, NUM_LEDS, CRGB(CRGB::Green).nscale8(38));
  FastLED.show();
}

// Standard headlights and taillights
void set_standby_lighting() {
  if (!running_lights_on) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else {
    CRGB front_color = full_headlights ? CRGB::White : CRGB(77, 77, 77);
    fill_solid(&leds[LEFT_FRONT_START], 16, front_color);
    fill_solid(&leds[RIGHT_REAR_START], 16, CRGB(77, 0, 0)); // Red taillights
  }
  FastLED.show();
  
}

// Pulse the green lights while waiting for Bluetooth
void update_disconnected_lighting(unsigned long now) {
  static unsigned long last_update = 0;
  if (now - last_update >= 10) {
    last_update = now;
    int sine_value = sin8(now / 16); // oscillation from 0-255
    disconnected_brightness = map(sine_value, 0, 255, 30, 80);
    fill_solid(leds, NUM_LEDS, CRGB(CRGB::Green).nscale8(disconnected_brightness));
    FastLED.show();
  }
}

// Blue breathing effect when in pairing mode
void update_pairing_breathing(unsigned long now) {
  if (now - breathe_time >= 20) {  // Update every 20ms for smooth animation
    breathe_time = now;
    
    // Create a smooth sine wave (0–255)
    uint8_t sine = sin8((now % BREATHE_PERIOD_MS) * 255 / BREATHE_PERIOD_MS);
    
    // Scale down to maximum 30% brightness: 255 * 0.30 ≈ 77
    uint8_t brightness = map(sine, 0, 255, 0, 77);
    
    // Deep blue base color
    CRGB blue_color = CRGB(0, 50, 255);
    blue_color.nscale8(brightness);
    
    fill_solid(leds, NUM_LEDS, blue_color);
    FastLED.show();
  }
}

// Knight Rider / Larson Scanner style turn signals — FRONT and REAR animate IN PARALLEL
void update_larson_scanner() {
  CRGB amber(255, 100, 0);

  // Re-draw base lighting first
  if (running_lights_on) {
    CRGB front_base = full_headlights ? CRGB::White : CRGB(77,77,77);
    fill_solid(&leds[0], 16, front_base);           // Both front sections
    fill_solid(&leds[16], 16, CRGB(77,0,0));        // Both rear sections (red taillights)
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }

  // Overlay the moving amber light — front and rear run together
  if (led_mode == TURN_LEFT) {
    // Clear the turn signal areas so we can redraw the moving bar
    fill_solid(&leds[LEFT_FRONT_START], 8, CRGB::Black);
    fill_solid(&leds[LEFT_REAR_START], 8, CRGB::Black);

    // How many LEDs are currently lit (0 to 8)
    int lit = larson_phase % 8;          // 0–7 repeating
    int num_lit = lit + 1;               // 1 to 8 LEDs lit

    // Left Front: sweep outward from center to left
    for (int i = 0; i < num_lit; i++) {
      if (LEFT_FRONT_END - 1 - i >= LEFT_FRONT_START)
        leds[LEFT_FRONT_END - 1 - i] = amber;
    }

    // Left Rear: sweep outward from center to left (mirror of front)
    for (int i = 0; i < num_lit; i++) {
      if (LEFT_REAR_START + i < LEFT_REAR_END)
        leds[LEFT_REAR_START + i] = amber;
    }

  } else if (led_mode == TURN_RIGHT) {
    fill_solid(&leds[RIGHT_FRONT_START], 8, CRGB::Black);
    fill_solid(&leds[RIGHT_REAR_START], 8, CRGB::Black);

    int lit = larson_phase % 8;
    int num_lit = lit + 1;

    // Right Front: sweep outward from center to right
    for (int i = 0; i < num_lit; i++) {
      if (RIGHT_FRONT_START + i < RIGHT_FRONT_END)
        leds[RIGHT_FRONT_START + i] = amber;
    }

    // Right Rear: sweep inward from right edge to center (mirrors typical vehicle behavior)
    for (int i = 0; i < num_lit; i++) {
      if (RIGHT_REAR_END - 1 - i >= RIGHT_REAR_START)
        leds[RIGHT_REAR_END - 1 - i] = amber;
    }
  }

  FastLED.show();
}

// KITT Scan -- FRONT and REAR animate IN PARALLEL
void update_kitt_scanner() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  CRGB c = KITT_COLOR; // temporary variable to scale down brightness
  c.nscale8(KITT_BRIGHTNESS);

  leds[kitt_pos] = c;         // front light
  leds[31 - kitt_pos] = c;    // rear light

  FastLED.show();
}

// ===================================================================
// INPUT PROCESSING
// ===================================================================

// Get effective deadzone for a given axis (-1 to use global)
int get_effective_deadzone(int specific_dz) {
  return (specific_dz >= 0) ? specific_dz : INPUT_DEADZONE;
}

// Apply per-axis scaling after deadzone check
int apply_scale(int raw, float scale_factor) {
  return round(raw * scale_factor);
}

// Main remap function with per-axis deadzone and scale
int remap_axis(int raw_value, int specific_dz, float scale_factor, int in_max, int out_max) {
  int dz = get_effective_deadzone(specific_dz);
  if (abs(raw_value) < dz) return 0;

  int sign = (raw_value > 0) ? 1 : -1;
  int scaled = apply_scale(abs(raw_value), scale_factor);
  return sign * map(scaled, dz, in_max, 0, out_max);
}

// ===================================================================
// SELF-TEST LOGIC
// ===================================================================

void run_self_test() {
  unsigned long now = millis();
  static bool blink_state = false;
  static uint8_t sub_phase = 0; // 0 = spinning, 1 = braking
  static bool result_announced = false;

  // If the test is over, any button press on the gamepad ends the "Pass/Fail" screen
  if (test_state == TEST_RESULT) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
      if (myGamepads[i] && myGamepads[i]->isConnected() && myGamepads[i]->buttons() != 0) {
        Serial.println(F("Self-test cleared by controller. Entering DRIVE mode."));
        test_state = TEST_DISABLED;
        result_announced = false;
        set_standby_lighting(); 
        return;
      }
    }
  }

  switch (test_state) {
    case CHECK_TRIGGER:
      if (now >= TEST_START_DELAY) {
        // If Pin 34 is high, start the hardware check
        if (digitalRead(TEST_PIN) == HIGH && (vehicle_config.capabilities & CAP_SELF_TEST)) {
          test_state = START_TEST;
        } else {
          test_state = TEST_DISABLED;
        }
      }
      break;

    case START_TEST:
      Serial.println(F("\n--- STARTING SELF-TEST ---"));
      Serial.printf("VBat: %.2fV\n", get_bus_voltage());
      current_test_step = 0; 
      sub_phase = 0; 
      self_test_passed = true;
      result_announced = false;
      reset_current_tracking(); 
      test_state = BASELINE_CURRENT; 
      test_phase_start = now;
      break;

    case BASELINE_CURRENT:
      update_ina219(); // Measure background noise in electrical system
      if (now - test_phase_start >= BASELINE_DURATION) {
        baselineCurrent = current_count ? current_sum / current_count : 0.0;
        Serial.printf("Baseline Current: %.3fA\n", baselineCurrent);
        reset_current_tracking(); 
        test_state = RUNNING_TESTS; 
        test_phase_start = now;
      }
      break;

    case RUNNING_TESTS:
      if (current_test_step >= NUM_TEST_STEPS) { 
        test_state = TEST_RESULT; 
        test_phase_start = now; 
        break; 
      }

      if (sub_phase == 0) { // MOTOR SPINNING PHASE
        set_motor_full(test_sequence[current_test_step].motor_index, test_sequence[current_test_step].direction);
        update_ina219();

        if (now - test_phase_start > SPIN_DURATION) {
          float avg = current_count ? current_sum / current_count : 0.0;
          Serial.printf("TEST [%s] -> I-Avg: %.3fA | I-Max: %.3fA", 
                        test_sequence[current_test_step].name, avg, current_peak_pos);
          
          // Logic: If current spiked but stayed low, the motor is healthy
          if (current_peak_pos > PEAK_THRESHOLD_A && avg < AVG_THRESHOLD_A) {
            Serial.println(F(" | PASS"));
            sub_phase = 1; 
            test_phase_start = now;
          } else { 
            Serial.println(F(" | FAIL")); // Motor is likely stuck or disconnected
            coast_all(); 
            self_test_passed = false; 
            test_state = TEST_RESULT; 
          }
        }
      } else { // MOTOR BRAKING PHASE
        active_brake_motor(test_sequence[current_test_step].motor_index);
        if (now - test_phase_start > BRAKE_DURATION) { 
          current_test_step++; 
          sub_phase = 0; 
          reset_current_tracking();
          test_phase_start = now; 
        }
      }
      break;

    case TEST_RESULT:
      // Blink Green for success, Red for failure
      if (!result_announced) {
        Serial.printf("--- SELF-TEST %s ---\n", self_test_passed ? "PASSED" : "FAILED");
        Serial.println(F("Press any button on controller to drive..."));
        result_announced = true;
      }
      if (now - test_phase_start >= PASS_BLINK_DELAY) {
        test_phase_start = now; 
        blink_state = !blink_state;
        fill_solid(leds, NUM_LEDS, blink_state ? (self_test_passed ? CRGB::Green : CRGB::Red) : CRGB::Black);
        FastLED.show();
      }
      break;

    case TEST_DISABLED: 
      break;
  }
}

// ===================================================================
// SERIAL COMMAND INTERFACE
// ===================================================================

void printHelp() {
  Serial.println(F("\nAvailable commands:"));
  Serial.println(F("help      - Show this help"));
  Serial.println(F("pair      - Activate pairing mode"));
  Serial.println(F("diag      - Toggle diagnostics"));
  Serial.println(F("dynamics  - Show current settings"));
  Serial.println(F("dynamics [param value]... - Set specific values"));
  Serial.println(F("  Params: ls_y_dz, ls_y_sc, ls_x_dz, ls_x_sc,"));
  Serial.println(F("          rs_y_dz, rs_y_sc, rs_x_dz, rs_x_sc"));
  Serial.println(F("  dz: -1 to 511 (-1 = global), sc: 0.2 to 1.2"));
  Serial.println(F("  Example: dynamics ls_y_dz 30 ls_x_sc 1.1"));
}

void saveSettings() {
  EEPROM.put(ADDR_DEADZONE, INPUT_DEADZONE);
  EEPROM.put(ADDR_DEADZONE_LS_X, INPUT_DEADZONE_LS_X);
  EEPROM.put(ADDR_DEADZONE_LS_Y, INPUT_DEADZONE_LS_Y);
  EEPROM.put(ADDR_DEADZONE_RS_X, INPUT_DEADZONE_RS_X);
  EEPROM.put(ADDR_DEADZONE_RS_Y, INPUT_DEADZONE_RS_Y);
  EEPROM.put(ADDR_LS_Y_SC, ls_y_sc);
  EEPROM.put(ADDR_LS_X_SC, ls_x_sc);
  EEPROM.put(ADDR_RS_Y_SC, rs_y_sc);
  EEPROM.put(ADDR_RS_X_SC, rs_x_sc);
  EEPROM.commit();
}

void handlePairCommand() {
  // Force disconnect any connected gamepad(s)
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] && myGamepads[i]->isConnected()) {
      myGamepads[i]->disconnect();
      Serial.println(F("Force-disconnected current gamepad."));
      delay(500);
    }
  }

  // Clear all stored bonding keys
  BP32.forgetBluetoothKeys();
  Serial.println(F("Cleared all Bluetooth bonding keys."));

  // Clear the allowlist completely
  uni_bt_allowlist_remove_all();
  Serial.println(F("Cleared Bluetooth allowlist (all entries removed)."));

  // Temporarily disable allowlist (extra safety, though clear + empty = open)
  uni_bt_allowlist_set_enabled(false);
  Serial.println(F("Temporarily disabled Bluetooth allowlist for pairing."));

  delay(500);

  Serial.println(F("Bluetooth pairing mode activated. Put your NEW gamepad into pairing mode now!"));

  // Print out the ESP32's Bluetooth MAC
  uint8_t bt_mac[6];
  esp_err_t err = esp_read_mac(bt_mac, ESP_MAC_BT); // Get the BT MAC
  if (err == ESP_OK) {
    Serial.print(F("ESP32 Bluetooth MAC address: "));
    for (int j = 0; j < 6; j++) {
      if (bt_mac[j] < 0x10) Serial.print("0");
      Serial.print(bt_mac[j], HEX);
      if (j < 5) Serial.print(":");
    }
    Serial.println();
  } else {
    Serial.println(F("Failed to read Bluetooth MAC address."));
  }

  // Allow new connections for pairing
  BP32.enableNewBluetoothConnections(true);

  // Activate blue breathing LEDs
  led_mode = PAIRING;
  breathe_time = millis();
}

void handleDynamicsCommand(const String& args) {
  if (args.length() == 0) {
    Serial.printf("Current settings:\n");
    Serial.printf("  Global DZ: %d\n", INPUT_DEADZONE);
    Serial.printf("  LS_Y: dz=%d sc=%.2f\n", INPUT_DEADZONE_LS_Y, ls_y_sc);
    Serial.printf("  LS_X: dz=%d sc=%.2f\n", INPUT_DEADZONE_LS_X, ls_x_sc);
    Serial.printf("  RS_Y: dz=%d sc=%.2f\n", INPUT_DEADZONE_RS_Y, rs_y_sc);
    Serial.printf("  RS_X: dz=%d sc=%.2f\n", INPUT_DEADZONE_RS_X, rs_x_sc);
    return;
  }

  String remaining = args + " ";
  bool changed = false;

  while (remaining.length() > 0) {
    int spacePos = remaining.indexOf(' ');
    if (spacePos == -1) break;
    String key = remaining.substring(0, spacePos);
    remaining = remaining.substring(spacePos + 1);
    remaining.trim();

    spacePos = remaining.indexOf(' ');
    String valStr = (spacePos == -1) ? remaining : remaining.substring(0, spacePos);
    remaining = (spacePos == -1) ? "" : remaining.substring(spacePos + 1);
    remaining.trim();

    if (key == "ls_y_dz") {
      int v = valStr.toInt();
      if (v >= -1 && v <= 511) { INPUT_DEADZONE_LS_Y = v; changed = true; }
    } else if (key == "ls_x_dz") {
      int v = valStr.toInt();
      if (v >= -1 && v <= 511) { INPUT_DEADZONE_LS_X = v; changed = true; }
    } else if (key == "rs_y_dz") {
      int v = valStr.toInt();
      if (v >= -1 && v <= 511) { INPUT_DEADZONE_RS_Y = v; changed = true; }
    } else if (key == "rs_x_dz") {
      int v = valStr.toInt();
      if (v >= -1 && v <= 511) { INPUT_DEADZONE_RS_X = v; changed = true; }
    } else if (key == "ls_y_sc") {
      float v = valStr.toFloat();
      if (v >= 0.2f && v <= 1.2f) { ls_y_sc = v; changed = true; }
    } else if (key == "ls_x_sc") {
      float v = valStr.toFloat();
      if (v >= 0.2f && v <= 1.2f) { ls_x_sc = v; changed = true; }
    } else if (key == "rs_y_sc") {
      float v = valStr.toFloat();
      if (v >= 0.2f && v <= 1.2f) { rs_y_sc = v; changed = true; }
    } else if (key == "rs_x_sc") {
      float v = valStr.toFloat();
      if (v >= 0.2f && v <= 1.2f) { rs_x_sc = v; changed = true; }
    } else {
      Serial.printf("Unknown param: %s\n", key.c_str());
    }
  }

  if (changed) {
    saveSettings();
    Serial.println(F("Settings updated and saved."));
  } else {
    Serial.println(F("No valid changes applied."));
  }
}

// ===================================================================
// SETUP & LOOP
// ===================================================================

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // Load saved settings from memory
  int saved_dz;
  int saved_lsx_dz, saved_lsy_dz, saved_rsx_dz, saved_rsy_dz;
  float saved_lsy_sc, saved_lsx_sc, saved_rsy_sc, saved_rsx_sc;

  // First: Read everything from EEPROM
  EEPROM.get(ADDR_DEADZONE, saved_dz);
  EEPROM.get(ADDR_DEADZONE_LS_X, saved_lsx_dz);
  EEPROM.get(ADDR_DEADZONE_LS_Y, saved_lsy_dz);
  EEPROM.get(ADDR_DEADZONE_RS_X, saved_rsx_dz);
  EEPROM.get(ADDR_DEADZONE_RS_Y, saved_rsy_dz);
  EEPROM.get(ADDR_LS_Y_SC, saved_lsy_sc);
  EEPROM.get(ADDR_LS_X_SC, saved_lsx_sc);
  EEPROM.get(ADDR_RS_Y_SC, saved_rsy_sc);
  EEPROM.get(ADDR_RS_X_SC, saved_rsx_sc);

  // Apply range checks and assign to global persistent variables
  if (saved_dz >= 0 && saved_dz <= 511) INPUT_DEADZONE = saved_dz;
  // else keep default 32

  if (saved_lsx_dz >= -1 && saved_lsx_dz <= 511) INPUT_DEADZONE_LS_X = saved_lsx_dz;
  if (saved_lsy_dz >= -1 && saved_lsy_dz <= 511) INPUT_DEADZONE_LS_Y = saved_lsy_dz;
  if (saved_rsx_dz >= -1 && saved_rsx_dz <= 511) INPUT_DEADZONE_RS_X = saved_rsx_dz;
  if (saved_rsy_dz >= -1 && saved_rsy_dz <= 511) INPUT_DEADZONE_RS_Y = saved_rsy_dz;

  // Persistent saved scales (used by dynamics command and restored on mode switch)
  if (saved_lsy_sc >= 0.2f && saved_lsy_sc <= 1.2f) ls_y_sc = saved_lsy_sc;
  else ls_y_sc = 1.0f;

  if (saved_lsx_sc >= 0.2f && saved_lsx_sc <= 1.2f) ls_x_sc = saved_lsx_sc;
  else ls_x_sc = 1.0f;

  if (saved_rsy_sc >= 0.2f && saved_rsy_sc <= 1.2f) rs_y_sc = saved_rsy_sc;
  else rs_y_sc = 1.0f;

  if (saved_rsx_sc >= 0.2f && saved_rsx_sc <= 1.2f) rs_x_sc = saved_rsx_sc;
  else rs_x_sc = 1.0f;

  // Initialize active (runtime) scales from the validated saved values
  active_ls_y_sc = 1.0f;
  active_ls_x_sc = 0.5f;

  // Start in normal mode
  throttle_mode = MODE_NORMAL;
  
  // Start I2C bus
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (is_ina219_present()) {
    vehicle_config = VEHICLE_GEN3;
    ina219_init();
  } else {
    vehicle_config = VEHICLE_GEN2;
    test_state = TEST_DISABLED;
  }

  // Setup ESP32 PWM Channels
  for (uint8_t ch = 0; ch < 8; ch++) {
    ledcSetup(ch, PWM_FREQ, PWM_RES);
  }
  for (int i = 0; i < 4; i++) {
    ledcAttachPin(motors[i].in1_pin, motors[i].in1_channel);
    ledcAttachPin(motors[i].in2_pin, motors[i].in2_channel);
  }

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  set_disconnected_lighting();
  
  // Setup Servo PWM Channels (50Hz, 16-bit)
  ledcSetup(SERVO1_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO2_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO3_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO4_CH, SERVO_FREQ, SERVO_RES);

  ledcAttachPin(SERVO1_PIN, SERVO1_CH);
  ledcAttachPin(SERVO2_PIN, SERVO2_CH);
  ledcAttachPin(SERVO3_PIN, SERVO3_CH);
  ledcAttachPin(SERVO4_PIN, SERVO4_CH);

  // Center all servos at startup
  ledcWrite(SERVO1_CH, SERVO_MID);
  ledcWrite(SERVO2_CH, SERVO_MID);
  ledcWrite(SERVO3_CH, SERVO_MID);
  ledcWrite(SERVO4_CH, SERVO_MID);

  // Start Bluepad32 Bluetooth Stack
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

  // Enable allowlist mode — only whitelisted controllers can connect
  uni_bt_allowlist_set_enabled(true);

  // Optional extra security
  BP32.enableNewBluetoothConnections(false);
  
  // BLE Pair Trigger
  pinMode(PAIR_TRIGGER_PIN, INPUT_PULLUP);

  // Self-Test Trigger
  pinMode(TEST_PIN, INPUT_PULLDOWN);  // Enables internal ~45kΩ pull-down
}

void loop() {
  // Process Serial Commands (Non-blocking)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim(); cmd.toLowerCase();
    if (cmd == "help") printHelp();
    else if (cmd == "pair") handlePairCommand();
    else if (cmd == "diag") diag_enabled = !diag_enabled;
    else if (cmd.startsWith("dynamics")) handleDynamicsCommand(cmd.substring(9));
  }
  
  // Check GPIO0 for manual pairing trigger
  static bool last_pair_state = true;
  bool current_pair_state = digitalRead(PAIR_TRIGGER_PIN);
  if (current_pair_state == LOW && last_pair_state == HIGH) {
    handlePairCommand();  // Trigger pairing mode
  }
  last_pair_state = current_pair_state;

  // Refresh Bluetooth Data
  BP32.update();
  unsigned long now = millis();
  update_ina219(); // Always track power draw

  // Periodic Diagnostic Output
  if (diag_enabled && (now - last_diag_time >= 5000)) {
    last_diag_time = now;
    if (vehicle_config.capabilities & CAP_CURRENT_MON) {
      Serial.printf("DIAG [5s]: VBat: %.2fV | I-Avg: %.3fA | I-Max: %.3fA\n", 
                    get_bus_voltage(), (current_count > 0 ? (current_sum / current_count) : 0), current_peak_pos);
      reset_current_tracking();
    }
  }

  // Find the first connected gamepad
  GamepadPtr gp = nullptr;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] && myGamepads[i]->isConnected()) {
      gp = myGamepads[i];
      break;
    }
  }

  // Connection/Disconnection logic
  if (controller_now_connected) {
    controller_now_connected = false;
    led_mode = STANDBY;
    set_standby_lighting();
  }

  // Handle Self-Test state machine
  if (vehicle_config.capabilities & CAP_SELF_TEST) run_self_test();
  if (test_state != TEST_DISABLED) return; // Exit loop if test is running

  if (!gp) {
    if (led_mode == PAIRING) {
      update_pairing_breathing(now);
    } else {
      update_disconnected_lighting(now);  // Normal pulsing green when truly disconnected/searching
    }
    coast_all();

    // Optional: clear previous button state when disconnected
    if (diag_enabled) prev_reported_buttons = 0;

    return;
  }

// ========================
// DIAGNOSTIC OUTPUT
// ========================
    if (diag_enabled) {
      uint32_t current_buttons = gp->buttons();
      uint8_t misc_btns = gp->miscButtons();
      uint8_t current_dpad = gp->dpad();  // Read D-pad once

      // Report on any change: buttons, misc, or D-pad
      if (current_buttons != prev_reported_buttons ||
          misc_btns != (prev_reported_buttons >> 24) ||
          current_dpad != prev_dpad) {

        Serial.print(F("BUTTONS: 0x"));
        Serial.print(current_buttons, HEX);
        Serial.print(F(" | MISC: 0x"));
        Serial.print(misc_btns, HEX);
        Serial.print(F(" | DPAD: 0x"));
        Serial.print(current_dpad, HEX);
        Serial.print("\n");

        // Update previous states
        prev_reported_buttons = current_buttons | ((uint32_t)misc_btns << 24);
        prev_dpad = current_dpad;
      }

      // Report raw joystick values at 4 Hz if any axis is outside deadzone
      if (now - last_joy_report_time >= JOY_REPORT_INTERVAL_MS) {
        last_joy_report_time = now;

        int ly = gp->axisY();
        int lx = gp->axisX();
        int ry = gp->axisRY();
        int rx = gp->axisRX();

        int trigR = gp->throttle();
        int trigL = gp->brake();

        bool any_active = false;
        if (abs(ly) >= get_effective_deadzone(INPUT_DEADZONE_LS_Y)) any_active = true;
        if (abs(lx) >= get_effective_deadzone(INPUT_DEADZONE_LS_X)) any_active = true;
        if (abs(ry) >= get_effective_deadzone(INPUT_DEADZONE_RS_Y)) any_active = true;
        if (abs(rx) >= get_effective_deadzone(INPUT_DEADZONE_RS_X)) any_active = true;
        if (trigL >= 50) any_active = true;
        if (trigR >= 50) any_active = true;

        if (any_active) {
          Serial.printf("JOY RAW: LX=%4d LY=%4d | RX=%4d RY=%4d | L2=%4d R2=%4d\n",
                        lx, ly, rx, ry, trigL, trigR);
        }
      }
    }

  // Gamepad Logic (Buttons & D-Pad)
  uint32_t current_buttons = gp->buttons();
  
  // Toggle Running Lights (Square/Y button)
  bool square = current_buttons & 0x0004;
  if (square && !prev_square) running_lights_on = !running_lights_on;
  prev_square = square;

  // Toggle KITT Scanner
  bool kitt_btn = current_buttons & KITT_BUTTON_MASK;
  if (kitt_btn && !prev_kitt_btn) {
    if (kitt_return_mode != KITT_SCANNER) {
      kitt_return_mode = led_mode;
    }

    led_mode = KITT_SCANNER;
    kitt_time = 0;
    kitt_pos = 0;
    kitt_dir = 1;
    kitt_cycles_done = 0;
  }
  prev_kitt_btn = kitt_btn;

  // Toggle Headlights (D-Pad Up/Down)
  uint8_t dpad = gp->dpad();
  if (dpad & 0x01) full_headlights = true;   
  if (dpad & 0x02) full_headlights = false;  

  // Turn Signals (D-Pad Left/Right)
  if (dpad & 0x08) { 
    if (led_mode != TURN_LEFT) { led_mode = TURN_LEFT; larson_phase = 0; turn_signal_cycles = 0; }
  } else if (dpad & 0x04) { 
    if (led_mode != TURN_RIGHT) { led_mode = TURN_RIGHT; larson_phase = 0; turn_signal_cycles = 0; }
  }
  

  // L3 Button: Cycle throttle modes (left thumbstick press)
  bool thumbL_pressed = gp->buttons() & 0x100;
  if (thumbL_pressed && !prev_thumbL) {  // Rising edge detect
    if (throttle_mode == MODE_FAST) {
      throttle_mode = MODE_NORMAL;
      active_ls_x_sc = 0.5f;
      active_ls_y_sc = 1.0f;
      Serial.println(F("Throttle Mode: NORMAL (reduced turn sensitivity)"));
    } else {
      throttle_mode = MODE_FAST;
      active_ls_x_sc = ls_x_sc;
      active_ls_y_sc = ls_y_sc;
      Serial.println(F("Throttle Mode: FAST (full steering)"));
    }
  }
  prev_thumbL = thumbL_pressed;
  
  // Tank Drive Mix Logic
  int ly = gp->axisY();
  int lx = gp->axisX();

  int fwd = remap_axis(ly, INPUT_DEADZONE_LS_Y, active_ls_y_sc, 511, PWM_MAX);
  int trn = -remap_axis(lx, INPUT_DEADZONE_LS_X, active_ls_x_sc, 511, PWM_MAX);
  
  // Mix X and Y to create Tank Drive steering
  target_left  = constrain(fwd + trn, -PWM_MAX, PWM_MAX);
  target_right = constrain(fwd - trn, -PWM_MAX, PWM_MAX);

  // Speed Ramping (Smoother movement)
  if (target_left == 0 && target_right == 0) {
    coast_all();
  } else if (now - last_ramp_time >= RAMP_DELAY_MS) {
    last_ramp_time = now;
    current_left  += (target_left  - current_left)  * RAMP_FACTOR;
    current_right += (target_right - current_right) * RAMP_FACTOR;
    update_motors();
  }
  
  // Right Stick Servo Control - Full 180° range per direction with deadzone
  int raw_rs_y = gp->axisRY();  // Raw: Up = negative, Down = positive
  int raw_rs_x = gp->axisRX();  // Raw: Left = negative, Right = positive

  // Apply deadzone using your existing system (outputs 0 in deadzone, otherwise scaled raw value)
  int rs_y = remap_axis(raw_rs_y, INPUT_DEADZONE_RS_Y, 1.0f, 511, 511);
  int rs_x = remap_axis(raw_rs_x, INPUT_DEADZONE_RS_X, 1.0f, 511, 511);

  // Servo 1 RS_Y Up: 0° to 180°
  int servo1_val = SERVO_MID;
  if (rs_y < 0) {  // Up
    servo1_val = map(rs_y, -511, 0, SERVO_MAX, SERVO_MIN);
  } else {
    servo1_val = SERVO_MID;  // Center when not up
  }
  ledcWrite(SERVO1_CH, constrain(servo1_val, SERVO_MIN, SERVO_MAX));

  // Servo 3 RS_Y Down: 0° to 180°
  int servo3_val = SERVO_MID;
  if (rs_y > 0) {  // Down
    servo3_val = map(rs_y, 0, 511, SERVO_MIN, SERVO_MAX);
  } else {
    servo3_val = SERVO_MID;
  }
  ledcWrite(SERVO3_CH, constrain(servo3_val, SERVO_MIN, SERVO_MAX));

  // Servo 2 RS_X Left: 0° to 180°
  int servo2_val = SERVO_MID;
  if (rs_x < 0) {  // Left
    servo2_val = map(rs_x, -511, 0, SERVO_MAX, SERVO_MIN);
  } else {
    servo2_val = SERVO_MID;
  }
  ledcWrite(SERVO2_CH, constrain(servo2_val, SERVO_MIN, SERVO_MAX));

  // Servo 4: RS_X Right: 0° to 180°
  int servo4_val = SERVO_MID;
  if (rs_x > 0) {  // Right
    servo4_val = map(rs_x, 0, 511, SERVO_MIN, SERVO_MAX);
  } else {
    servo4_val = SERVO_MID;
  }
  ledcWrite(SERVO4_CH, constrain(servo4_val, SERVO_MIN, SERVO_MAX));

  // LED Mode Handling (centralized control of all lighting states)
  if (led_mode == PAIRING) {
    update_pairing_breathing(now);
  }
  else if (led_mode == TURN_LEFT || led_mode == TURN_RIGHT) {
    if (now - larson_time >= LARSON_DELAY_MS) {
      larson_time = now;
      larson_phase++;
      if (larson_phase >= 8) {
        larson_phase = 0;
        turn_signal_cycles++;
        bool dpad_active = (led_mode == TURN_LEFT && (dpad & 0x08)) || 
                           (led_mode == TURN_RIGHT && (dpad & 0x04));
        if (!dpad_active && turn_signal_cycles >= CYCLES_TO_RUN) {
          led_mode = STANDBY;
        }
      }
      update_larson_scanner();
    }
  }

  else if (led_mode == KITT_SCANNER) {
    if (now - kitt_time >= (unsigned long)KITT_SPEED_MS) {
      kitt_time = now;

      update_kitt_scanner();

      kitt_pos += kitt_dir; // advance to the next LED

      // Bounce at ends
      if (kitt_pos >= 15) {
        kitt_pos = 15;
        kitt_dir = -1;
      } else if (kitt_pos <= 0) {
        kitt_pos = 0;
        kitt_dir = 1;

        // Completed one cycle
        kitt_cycles_done++;
        if (kitt_cycles_done >= KITT_CYCLES) {
          // Return to the previous lighting mode (default STANDBY)
          led_mode = kitt_return_mode;
          if (led_mode == STANDBY) {
            set_standby_lighting();
          }
        }
      }
    }
  }

  else if (led_mode == STANDBY) {
    set_standby_lighting();
  }
  // Note: DISCONNECTED mode is handled earlier when no controller is connected
}