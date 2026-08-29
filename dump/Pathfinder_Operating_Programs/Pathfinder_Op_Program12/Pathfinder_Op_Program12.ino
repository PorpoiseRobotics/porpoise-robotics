/**
 * Pathfinder_Op_Program12.ino
 * Porpoise Robotics - Pathfinder vehicle, advanced program
 *
 * Successor to Pathfinder_Op_Program11.2. Same vehicle, same feature set,
 * reorganised into tabs and with several bugs fixed. See the change log at
 * the bottom of this comment for what actually changed.
 *
 * HARDWARE
 * - ESP32 DevKitC clone, 38 pins (ESP32-D)
 * - 4 x DC brushed motors on TI DRV8871 H-bridges, 30 kHz PWM
 * - 4 x hobby servos
 * - 32 x WS2812B addressable LEDs in one loop, data on GPIO 5
 * - INA219 voltage and current sensor (Gen 3 vehicles only)
 *
 * The program detects which vehicle it is running on at boot by looking for
 * the INA219 on the I2C bus. Gen 3 gets current monitoring and the self-test;
 * Gen 2 gets neither. One binary serves both.
 *
 * BEFORE YOU CAN COMPILE THIS
 * 1. Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0.
 *      File > Preferences > "Additional boards manager URLs", add:
 *        https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
 *      Then Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
 * 2. Library: "Adafruit NeoPixel" by Adafruit.
 *    Bluepad32 itself ships with the board package.
 *
 * CONTROLS
 * - Left thumbstick:  drive. Up forward, down reverse, left/right steer.
 * - Right thumbstick: -Y servo 1, +Y servo 3, -X servo 2, +X servo 4
 * - D-pad UP:    headlights full bright
 * - D-pad DOWN:  headlights back to normal
 * - D-pad LEFT:  left turn signal, 3 cycles, or hold for longer
 * - D-pad RIGHT: right turn signal, 3 cycles, or hold for longer
 * - X button (west):  toggle all running lights
 * - Y button (north): run the KITT scanner
 * - L3 (left stick click): cycle throttle modes
 *     NORMAL - full speed, reduced steering authority (default)
 *     FAST   - full speed, full steering
 *
 * PAIRING A CONTROLLER
 * Two ways in, both do the same thing:
 * - Press the BOOT button on the ESP32 module, or
 * - Type "pair" in the Serial Monitor at 115200 baud.
 * The LEDs breathe blue while pairing. Put the controller into pairing mode
 * and wait. Once it connects, its address is stored in EEPROM and only that
 * controller may connect from then on, including after a power cycle.
 *
 * SERIAL COMMANDS (115200 baud)
 *   help      - list the commands
 *   pair      - enter pairing mode
 *   forget    - clear the paired controller and pair again from scratch
 *   diag      - toggle live diagnostics
 *   dynamics  - show or set deadzones and scale factors
 *
 * SELF-TEST (Gen 3)
 * Jumper GPIO 34 high at boot. Each motor is driven both ways while current
 * draw is watched; green blinks mean pass, red means fail. Press any button
 * on the controller to leave the result screen and drive.
 *
 * Authors: Ken Masterson & Valentino Farre
 * Original 11.2: December 2025
 *
 * CHANGE LOG since 11.2
 * - Replaced FastLED with Adafruit NeoPixel. FastLED 3.10.5 auto-enables an
 *   ESP-DSP FFT backend that does not build against the bluepad32 4.1.0 SDK,
 *   which stopped 11.2 compiling. NeoPixel also matches the beginner programs,
 *   so the whole course now uses one LED library.
 * - Servo travel corrected to the standard 1.0-2.0 ms. 11.2 used 0.5-2.5 ms,
 *   which can drive a servo into its end stops.
 * - The paired controller's address is now saved to EEPROM and restored into
 *   the Bluetooth allowlist at every boot, so a vehicle cannot lose track of
 *   its controller after a power cycle.
 * - Bluetooth addresses now print in the correct byte order.
 * - Speed ramping always reaches its target. The old integer maths could stall
 *   one count short and sit there.
 * - The self-test now compares motor current against the measured baseline it
 *   already took, rather than against fixed absolute thresholds.
 * - Standby lighting is only redrawn when something changes, instead of being
 *   pushed to the strip on every pass of loop().
 * - The KITT scanner has a fading tail, matching the beginner programs.
 * - Switched to the current Bluepad32 controller API and dropped the unused
 *   esp_now.h and WiFi.h includes and the unused servo position variables.
 * - Split one 1292-line file into tabs by subsystem.
**/

#include <Bluepad32.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EEPROM.h>
#include <esp_mac.h>
#include <uni.h>

#include "Config.h"

// ===================================================================
// HARDWARE OBJECTS
// ===================================================================

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

Motor motors[4] = {
  {13, 12, 0, 1},   // Front left
  {23, 22, 2, 3},   // Front right
  {19, 18, 4, 5},   // Rear left
  {17, 16, 6, 7}    // Rear right
};

const TestStep test_sequence[] = {
  {0,  1, "FL Forward"}, {0, -1, "FL Reverse"},
  {1,  1, "FR Forward"}, {1, -1, "FR Reverse"},
  {3,  1, "RR Forward"}, {3, -1, "RR Reverse"},
  {2,  1, "RL Forward"}, {2, -1, "RL Reverse"}
};
const uint8_t NUM_TEST_STEPS = sizeof(test_sequence) / sizeof(test_sequence[0]);

// ===================================================================
// VEHICLE STATE
// ===================================================================

VehicleConfig vehicle_config = { 0 };

// --- Input tuning, loaded from EEPROM at boot ---
int INPUT_DEADZONE      = DEFAULT_DEADZONE;
int INPUT_DEADZONE_LS_X = -1;   // -1 means "use the global deadzone"
int INPUT_DEADZONE_LS_Y = -1;
int INPUT_DEADZONE_RS_X = -1;
int INPUT_DEADZONE_RS_Y = -1;

float ls_y_sc = 1.0f, ls_x_sc = 1.0f, rs_y_sc = 1.0f, rs_x_sc = 1.0f;
float active_ls_y_sc = NORMAL_MODE_Y_SCALE;
float active_ls_x_sc = NORMAL_MODE_X_SCALE;
ThrottleMode throttle_mode = MODE_NORMAL;

// --- Drive ---
int target_left = 0, target_right = 0;
int current_left = 0, current_right = 0;
unsigned long last_ramp_time = 0;

// --- Lighting ---
LEDMode led_mode = DISCONNECTED;
bool    full_headlights  = false;
bool    running_lights_on = true;
bool    standby_dirty    = true;   // Standby lighting needs redrawing

unsigned long larson_time = 0;
int larson_phase = 0;
int turn_signal_cycles = 0;

unsigned long breathe_time = 0;

unsigned long kitt_time = 0;
int kitt_pos = 0, kitt_dir = 1, kitt_cycles_done = 0;
LEDMode kitt_return_mode = STANDBY;

// --- Controller ---
ControllerPtr myControllers[BP32_MAX_CONTROLLERS];
volatile bool controller_now_connected = false;

bool prev_running_lights_btn = false;
bool prev_kitt_btn  = false;
bool prev_thumbL    = false;

// --- Pairing ---
bool    paired_addr_valid = false;
uint8_t paired_addr[6]    = { 0, 0, 0, 0, 0, 0 };

// --- Current sensing ---
float current_sum = 0.0f;
int   current_count = 0;
float current_peak_pos = 0.0f;
float current_peak_neg = 0.0f;

// --- Self-test ---
SelfTestState test_state = CHECK_TRIGGER;
unsigned long test_phase_start = 0;
uint8_t current_test_step = 0;
float   baselineCurrent = 0.0f;
bool    self_test_passed = true;

// --- Diagnostics ---
bool diag_enabled = false;
unsigned long last_diag_time = 0;
unsigned long last_joy_report_time = 0;
uint32_t prev_reported_buttons = 0;
uint8_t  prev_reported_misc = 0;
uint8_t  prev_reported_dpad = 0;

// ===================================================================
// SETUP
// ===================================================================

void setup() {
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  // Detect which generation of vehicle this is by looking for the INA219.
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (is_ina219_present()) {
    vehicle_config.capabilities = CAP_SELF_TEST | CAP_CURRENT_MON;
    ina219_init();
    Serial.println(F("Vehicle: Gen 3 (current sensor found)"));
  } else {
    vehicle_config.capabilities = 0;
    test_state = TEST_DISABLED;
    Serial.println(F("Vehicle: Gen 2 (no current sensor)"));
  }

  setup_motors();
  setup_servos();

  strip.begin();
  strip.setBrightness(255);
  strip.clear();
  strip.show();
  set_disconnected_lighting();

  setup_bluetooth();

  pinMode(PAIR_TRIGGER_PIN, INPUT_PULLUP);
  pinMode(TEST_PIN, INPUT);   // External pull-down on the board; GPIO 34 has none of its own

  Serial.println(F("Pathfinder ready."));
}

// ===================================================================
// LOOP
// ===================================================================

void loop() {
  unsigned long now = millis();

  handleSerialCommands();
  handlePairButton();

  BP32.update();
  update_ina219();

  // Periodic battery and current report
  if (diag_enabled && (now - last_diag_time >= DIAG_INTERVAL_MS)) {
    last_diag_time = now;
    if (vehicle_config.capabilities & CAP_CURRENT_MON) {
      Serial.printf("DIAG [5s]: VBat: %.2fV | I-Avg: %.3fA | I-Max: %.3fA | I-Min: %.3fA\n",
                    get_bus_voltage(), get_average_current(),
                    current_peak_pos, current_peak_neg);
      reset_current_tracking();
    }
  }

  ControllerPtr gp = firstConnectedController();

  if (controller_now_connected) {
    controller_now_connected = false;
    led_mode = STANDBY;
    standby_dirty = true;
  }

  // The self-test owns the vehicle until it finishes.
  if (vehicle_config.capabilities & CAP_SELF_TEST) run_self_test();
  if (test_state != TEST_DISABLED) return;

  // ---- Nothing connected ----
  if (gp == nullptr) {
    coast_all();
    if (led_mode == PAIRING) {
      update_pairing_breathing(now);
    } else {
      update_disconnected_lighting(now);
    }
    if (diag_enabled) prev_reported_buttons = 0;
    return;
  }

  if (diag_enabled) report_controller_state(gp, now);

  // ---- Buttons ----
  // Bluepad32 names face buttons by position, Xbox style:
  // a() bottom, b() right, x() left, y() top.
  bool lights_btn = gp->x();
  if (lights_btn && !prev_running_lights_btn) {
    running_lights_on = !running_lights_on;
    standby_dirty = true;
  }
  prev_running_lights_btn = lights_btn;

  bool kitt_btn = gp->y();
  if (kitt_btn && !prev_kitt_btn) start_kitt_scanner();
  prev_kitt_btn = kitt_btn;

  uint8_t dpad = gp->dpad();
  if ((dpad & DPAD_UP) && !full_headlights)   { full_headlights = true;  standby_dirty = true; }
  if ((dpad & DPAD_DOWN) && full_headlights)  { full_headlights = false; standby_dirty = true; }

  if (dpad & DPAD_LEFT) {
    if (led_mode != TURN_LEFT) { led_mode = TURN_LEFT; larson_phase = 0; turn_signal_cycles = 0; }
  } else if (dpad & DPAD_RIGHT) {
    if (led_mode != TURN_RIGHT) { led_mode = TURN_RIGHT; larson_phase = 0; turn_signal_cycles = 0; }
  }

  bool thumbL = gp->thumbL();
  if (thumbL && !prev_thumbL) cycleThrottleMode();
  prev_thumbL = thumbL;

  // ---- Drive ----
  int fwd = remap_axis(gp->axisY(), INPUT_DEADZONE_LS_Y, active_ls_y_sc, AXIS_MAX, PWM_MAX);
  int trn = -remap_axis(gp->axisX(), INPUT_DEADZONE_LS_X, active_ls_x_sc, AXIS_MAX, PWM_MAX);

  target_left  = constrain(fwd + trn, -PWM_MAX, PWM_MAX);
  target_right = constrain(fwd - trn, -PWM_MAX, PWM_MAX);

  if (target_left == 0 && target_right == 0) {
    coast_all();
  } else if (now - last_ramp_time >= RAMP_DELAY_MS) {
    last_ramp_time = now;
    current_left  = ramp_towards(current_left,  target_left);
    current_right = ramp_towards(current_right, target_right);
    update_motors();
  }

  // ---- Servos ----
  update_servos(gp);

  // ---- Lighting ----
  update_lighting(now, dpad);
}
