/**
 * Pathfinder_Op_Program13.ino
 * Porpoise Robotics - Pathfinder vehicle, advanced program
 *
 * Pathfinder_Op_Program12 with police lights added, and the whole program
 * kept in this one file. Everything Op 12 does, Op 13 does the same way.
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
 *      Tools > Board > Boards Manager, search "esp32_bluepad32", install 4.1.0.
 *      Then Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
 *      If "esp32 by Espressif Systems" is installed too (the PS3 track uses
 *      it), the board menu has two "ESP32 Dev Module" entries. Pick the one
 *      under esp32_bluepad32.
 * 2. One library, from Tools > Manage Libraries:
 *      "Adafruit NeoPixel" by Adafruit (built with 1.15.5).
 *
 * Everything else ships with the board package - Bluepad32 (Bluepad32.h and
 * uni.h), Wire and EEPROM - so there is nothing more to install.
 * FastLED is NOT needed.
 *
 * BEFORE YOU UPLOAD: TELL THE VEHICLE WHICH CONTROLLER IS ITS OWN
 * In a classroom there are many vehicles and many controllers switched on at
 * once, so each vehicle must ignore every controller except its own. This
 * program is told the Bluetooth address of that one controller, and Bluepad32
 * refuses every other controller before the connection is even accepted.
 *
 * 1. Upload the sketch called pathfinder_find_controller and follow the
 *    instructions at the top of it. It prints a line that looks like this:
 *        const uint8_t MY_CONTROLLER[6] = { 0x98, 0xB6, 0xE9, 0x11, 0x22, 0x33 };
 * 2. Paste that line over the MY_CONTROLLER line at the top of the SETTINGS
 *    section below, then upload this program.
 * 3. Write the address on a sticker on both the vehicle and the controller,
 *    so the pair stays together.
 *
 * If MY_CONTROLLER is left as all zeros, the vehicle blinks RED, will not
 * connect to anything, and says why on the Serial Monitor.
 *
 * Op 12 had a pairing mode instead (the BOOT button, or typing "pair"). Op 13
 * does not: the address in this file is the only one the vehicle accepts. To
 * move a vehicle to a different controller, change MY_CONTROLLER and upload.
 *
 * CONTROLS
 * Bluepad32 names the four face buttons by where they sit, not by the letter
 * printed on them, so the letter depends on the controller:
 *
 *                      Switch pad   Xbox pad   PlayStation pad
 *   Top button             X           Y          Triangle
 *   Left button            Y           X          Square
 *   Right button           A           B          Circle
 *   Bottom button          B           A          Cross
 *
 * - Left thumbstick:  drive. Up forward, down reverse, left/right steer.
 * - Right thumbstick: -Y servo 1, +Y servo 3, -X servo 2, +X servo 4
 * - D-pad UP:    headlights full bright
 * - D-pad DOWN:  headlights back to normal
 * - D-pad LEFT:  left turn signal, 3 cycles, or hold for longer
 * - D-pad RIGHT: right turn signal, 3 cycles, or hold for longer
 * - Top button:    KITT scanner
 * - Right button:  police lights at LOW brightness (60 of 255)
 * - Bottom button: police lights at HIGH brightness (120 of 255)
 *     These three each choose an animation. Only one animation is on at a
 *     time: press a different one of these buttons and the animation
 *     switches over to it; press the same button again and it goes off.
 * - Left button:   cancel the animation, or toggle the running lights
 *     If an animation or a turn signal is showing, one press stops it and
 *     brings back the normal lights. If not, it switches all running
 *     lights on or off.
 * - L3 (left stick click): cycle throttle modes
 *     NORMAL - full speed, reduced steering authority (default)
 *     FAST   - full speed, full steering
 *
 * POLICE LIGHTS
 * The whole left side of the vehicle flashes red, then the whole right side
 * flashes blue, swapping every half second. Low and high brightness count as
 * two different animations, so with the lights on at low, the bottom button
 * switches them to high.
 *
 * HOW THE LIGHTING EFFECTS SHARE THE STRIP
 * There are three animations (KITT, police low, police high) and only one
 * can be on at a time, so there is never one hidden underneath another.
 * - Pressing an animation button ends any turn signal, so you always see
 *   what the press did straight away.
 * - A turn signal takes over while it runs. When it finishes, the animation
 *   that was on carries on, or the normal lights come back.
 * - The left button stops the animation and any turn signal.
 * - D-pad up/down still work while an animation is on, but you only see the
 *   headlights change once it goes off.
 * The Serial Monitor prints a line for every lighting button press, so you
 * can always check that a press was received.
 *
 * Change the speed and the two brightness levels in the "Police lights"
 * part of the SETTINGS section below.
 *
 * SERIAL COMMANDS (115200 baud)
 *   help      - list the commands
 *   diag      - toggle live diagnostics
 *   dynamics  - show or set deadzones and scale factors
 *
 * SELF-TEST (Gen 3)
 * Jumper GPIO 34 high at boot. Each motor is driven both ways while current
 * draw is watched; green blinks mean pass, red means fail. Press any button
 * on the controller to leave the result screen and drive.
 *
 * HOW THIS FILE IS LAID OUT
 * Op 12 is the same program split into tabs, one per subsystem. Op 13 keeps
 * it in one file so there is only one thing to copy, share or lose. The
 * compiled program is the same either way: before it compiles, the Arduino
 * IDE joins all of a sketch's tabs into one file anyway. Use Ctrl+F on a
 * section name to jump straight to it.
 *
 *    1. SETTINGS              every number you might want to change
 *    2. HARDWARE OBJECTS      the LED strip, the motors, the self-test steps
 *    3. VEHICLE STATE         the variables the whole program shares
 *    4. SETUP AND LOOP        what runs at power-on, and what runs forever
 *    5. LIGHTING              everything that touches the LED strip
 *    6. MOTORS AND SERVOS     stick math, drive, servos, throttle modes
 *    7. BLUETOOTH             connecting, and the one-controller lock
 *    8. SERIAL CONSOLE        typed commands, saved settings, diagnostics
 *    9. CURRENT SENSOR        the INA219 (Gen 3 vehicles only)
 *   10. SELF-TEST             the powered motor check (Gen 3 vehicles only)
 *
 * Authors: Ken Masterson & Valentino Farre
 * Original 11.2: December 2025
 *
 * CHANGE LOG since Op 12
 * - Added police lights, on two buttons at two brightness levels.
 * - Joined Op 12's eight tabs (seven .ino files and Config.h) into one file.
 * - The press that clears the self-test result no longer also does that
 *   button's normal job. In Op 12, clearing with the top button also started
 *   the KITT scanner.
 * - The KITT scanner runs until it is switched off. In Op 12 it ran three
 *   sweeps and stopped by itself, and could not be stopped early.
 * - The top, right and bottom buttons each choose an animation, and only one
 *   is on at a time: a different button switches straight to its animation,
 *   the same button again switches it off.
 * - The left button now cancels any animation that is showing (KITT, police
 *   lights, turn signal). With none showing, it still toggles the running
 *   lights, as it did in Op 12.
 * - The running-lights, KITT and police buttons print ON or OFF in the
 *   Serial Monitor.
 * - The controller's address is written into the program (MY_CONTROLLER),
 *   the same way pathfinder_nintendoswitch does it. Op 12's pairing mode,
 *   the BOOT button and the "pair" and "forget" commands are gone, and the
 *   paired address is no longer kept in EEPROM.
 * For what changed between 11.2 and 12, read the change log at the top of
 * Pathfinder_Op_Program12.ino.
**/

#include <Bluepad32.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EEPROM.h>
#include <uni.h>                // Lets us use the Bluetooth "allowlist"

// ===================================================================
// 1. SETTINGS
// ===================================================================
//
// Every tunable number and every custom type lives here, so the rest of the
// program contains logic and nothing else. If you want to change how the
// vehicle behaves, this is almost certainly the section you want.
//
// The custom types (enum and struct) must stay up here, above every function.
// The Arduino IDE writes a declaration for each of your functions and puts it
// just above the first function in the file. A type defined lower down would
// end up below those declarations, and the program would not compile.

// --- The one controller this vehicle will talk to ---
// CHANGE THIS for every vehicle. Run pathfinder_find_controller to get this
// line, then paste it over this one. See "BEFORE YOU UPLOAD" at the top.
// All zeros means "not filled in yet": the vehicle blinks red and waits.
const uint8_t MY_CONTROLLER[6] = { 0x98, 0xB6, 0x2E, 0x40, 0x39, 0xA6 };

// --- Motors ---

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

// --- Input ---

const int   AXIS_MAX      = 511;   // Bluepad32 normalises every stick to -512..511
const int   DEFAULT_DEADZONE = 32;
const int   DEADZONE_MIN  = -1;    // -1 on a per-axis deadzone means "use the global one"
const int   DEADZONE_MAX  = 511;
const float SCALE_MIN     = 0.2f;
const float SCALE_MAX     = 1.2f;

// Throttle modes are toggled with the left thumbstick button (L3).
enum ThrottleMode { MODE_FAST, MODE_NORMAL };

const float NORMAL_MODE_X_SCALE = 0.75f;  // Reduced steering authority
const float NORMAL_MODE_Y_SCALE = 1.0f;

// --- Servos ---

const int SERVO1_PIN = 25, SERVO1_CH = 8;   // Right stick Y up
const int SERVO2_PIN = 26, SERVO2_CH = 9;   // Right stick X left
const int SERVO3_PIN = 27, SERVO3_CH = 10;  // Right stick Y down
const int SERVO4_PIN = 14, SERVO4_CH = 11;  // Right stick X right

const int SERVO_FREQ = 50;   // One pulse every 20 ms
const int SERVO_RES  = 16;   // 16-bit duty resolution

// Pulse widths in microseconds. A standard hobby servo sweeps its full travel
// between 1.0 ms and 2.0 ms. Going wider can drive a servo into its
// mechanical end stops.
const int SERVO_MIN_US = 1000;
const int SERVO_MID_US = 1500;
const int SERVO_MAX_US = 2000;

// --- LEDs ---

const int LED_PIN   = 5;
const int NUM_LEDS  = 32;

// The strip is one loop around the vehicle: 0-15 across the front left to
// right, then 16-31 across the rear right to left. The LED directly behind
// front LED p is always 31 - p.
//
//                      FRONT
//            0 1 2 3 4 5 6 7 8 9 ... 15
//      LEFT                                RIGHT
//           31 30 29 ...        18 17 16
//                      REAR
//
// So the LEFT side is 0-7 at the front plus 24-31 at the rear, and the RIGHT
// side is 8-15 at the front plus 16-23 at the rear.
const int LEFT_FRONT_START  = 0;
const int RIGHT_FRONT_START = 8;
const int RIGHT_REAR_START  = 16;
const int LEFT_REAR_START   = 24;
const int CORNER_LEN        = 8;
const int FRONT_LAST        = 15;
const int REAR_LAST         = 31;

// What the strip is doing. In STANDBY it shows the animation chosen below,
// or the normal driving lights if none is chosen.
enum LEDMode { DISCONNECTED, STANDBY, TURN_LEFT, TURN_RIGHT };

// The animations the top, right and bottom buttons choose between. Only one
// can be on at a time; ANIM_NONE means the normal driving lights.
enum Animation { ANIM_NONE, ANIM_KITT, ANIM_POLICE_LOW, ANIM_POLICE_HIGH };

// Brightness levels run 0 (off) to 255 (full), because each color is 8 bits.
const uint8_t HEADLIGHT_DIM    = 77;
const uint8_t HEADLIGHT_BRIGHT = 255;
const uint8_t TAILLIGHT_LEVEL  = 77;

// Turn signals
const int LARSON_DELAY_MS = 50;
const int TURN_CYCLES     = 3;    // Cycles to finish after the D-pad is released

// Blinking red when MY_CONTROLLER has not been filled in
const unsigned long NO_ADDRESS_BLINK_MS = 500;

// KITT scanner: runs until the top button is pressed again
const int     KITT_SPEED_MS   = 35;   // Time between steps. Lower is faster.
const int     KITT_TAIL       = 3;    // Fading LEDs trailing the bright one
const uint8_t KITT_BRIGHTNESS = 200;
const uint8_t KITT_R = 0, KITT_G = 0, KITT_B = 255;   // Blue. Try 255,0,0 for Knight Rider red.

// --- Police lights ---

// How long each side stays lit before the other side takes over.
// Lower is faster: try 250 for a quicker flash.
const unsigned long POLICE_STEP_MS = 500;

// The two brightness levels, out of 255. The right face button chooses the
// police lights at LOW and the bottom face button chooses them at HIGH.
const uint8_t POLICE_LEVEL_LOW  = 60;
const uint8_t POLICE_LEVEL_HIGH = 120;

// --- Current sensor (INA219, Gen 3 vehicles only) ---

const uint8_t INA219_I2C_ADDR = 0x40;
const int     I2C_SDA_PIN     = 32;
const int     I2C_SCL_PIN     = 33;
const float   SHUNT_RESISTOR_OHMS = 0.0025f;

// --- Vehicle capabilities ---

// Bit flags, so one binary can serve more than one generation of vehicle.
enum VehicleCapability {
  CAP_SELF_TEST   = 1 << 0,
  CAP_CURRENT_MON = 1 << 1
};

struct VehicleConfig {
  uint8_t capabilities;
};

// --- Self-test (Gen 3 vehicles only) ---

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

// --- Stored settings (EEPROM) ---

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
// Bytes 36-42 held Op 12's paired controller address. Op 13 never reads or
// writes them, so a vehicle moved over from Op 12 can keep its old contents.

// --- Diagnostics ---

const unsigned long DIAG_INTERVAL_MS      = 5000;
const unsigned long JOY_REPORT_INTERVAL_MS = 250;   // 4 Hz

// A typed command with no line ending runs after this much quiet.
const unsigned long CONSOLE_IDLE_MS = 1000;

// ===================================================================
// 2. HARDWARE OBJECTS
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
// 3. VEHICLE STATE
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

// --- Animations ---
Animation animation = ANIM_NONE;     // The one animation that is on, if any

unsigned long kitt_time = 0;
int kitt_pos = 0, kitt_dir = 1;

bool    police_left_side = true;     // Which side lights on the next flash
unsigned long police_time = 0;

// --- Controller ---
ControllerPtr myControllers[BP32_MAX_CONTROLLERS];
volatile bool controller_now_connected = false;

bool prev_running_lights_btn = false;
bool prev_kitt_btn  = false;
bool prev_police_low_btn  = false;
bool prev_police_high_btn = false;
bool prev_thumbL    = false;

bool controller_address_set = false;   // false while MY_CONTROLLER is all zeros

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
// 4. SETUP AND LOOP
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

  // The strip itself stays at full brightness. Each light sets its own level
  // by scaling its color, so dimming one effect never dims the others.
  strip.begin();
  strip.setBrightness(255);
  strip.clear();
  strip.show();
  set_disconnected_lighting();

  setup_bluetooth();

  pinMode(TEST_PIN, INPUT);   // External pull-down on the board; GPIO 34 has none of its own

  if (controller_address_set) Serial.println(F("Pathfinder ready. Waiting for the controller..."));
}

void loop() {
  unsigned long now = millis();

  handleSerialCommands();

  // Nobody has told this vehicle which controller is its own, so there is
  // nothing safe to do. Blink red and wait for somebody to fix the program.
  if (!controller_address_set) {
    update_no_address_lighting(now);
    return;
  }

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
    update_disconnected_lighting(now);
    if (diag_enabled) prev_reported_buttons = 0;
    return;
  }

  if (diag_enabled) report_controller_state(gp, now);

  // ---- Buttons ----
  // Bluepad32 names face buttons by position, Xbox style:
  // a() bottom, b() right, x() left, y() top.
  // Each button acts once, on the moment it goes down, by comparing it with
  // what it was on the previous pass of loop().
  // The left button has two jobs. If an animation is showing, it cancels it.
  // If not, it switches the running lights on or off.
  bool lights_btn = gp->x();
  if (lights_btn && !prev_running_lights_btn) {
    if (animation_running()) {
      cancel_animations();
    } else {
      running_lights_on = !running_lights_on;
      Serial.println(running_lights_on ? F("Running lights ON") : F("Running lights OFF"));
      normal_lights_changed();
    }
  }
  prev_running_lights_btn = lights_btn;

  bool kitt_btn = gp->y();
  if (kitt_btn && !prev_kitt_btn) select_animation(ANIM_KITT);
  prev_kitt_btn = kitt_btn;

  bool police_low_btn = gp->b();
  if (police_low_btn && !prev_police_low_btn) select_animation(ANIM_POLICE_LOW);
  prev_police_low_btn = police_low_btn;

  bool police_high_btn = gp->a();
  if (police_high_btn && !prev_police_high_btn) select_animation(ANIM_POLICE_HIGH);
  prev_police_high_btn = police_high_btn;

  uint8_t dpad = gp->dpad();
  if ((dpad & DPAD_UP) && !full_headlights)   { full_headlights = true;  normal_lights_changed(); }
  if ((dpad & DPAD_DOWN) && full_headlights)  { full_headlights = false; normal_lights_changed(); }

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

// ===================================================================
// 5. LIGHTING
// ===================================================================
//
// Everything that touches the 32-LED strip. Adafruit NeoPixel has no color
// math of its own, so the few helpers we need are written out here. They are
// short, and having them in the open makes the color math visible.

// --- Color helpers ---

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return strip.Color(r, g, b);
}

// Scales a packed color toward black. 255 leaves it alone, 0 turns it off.
uint32_t scale_color(uint32_t color, uint8_t scale) {
  uint8_t r = (uint8_t)(color >> 16);
  uint8_t g = (uint8_t)(color >> 8);
  uint8_t b = (uint8_t)color;
  return strip.Color((r * scale) / 255, (g * scale) / 255, (b * scale) / 255);
}

// Fills a run of LEDs with one color.
void fill_range(int first, int count, uint32_t color) {
  for (int i = first; i < first + count && i < NUM_LEDS; i++) {
    strip.setPixelColor(i, color);
  }
}

/*
  An 8-bit sine: feed it 0..255 for one full turn and it returns 0..255 with
  128 as the midpoint. Used by the breathing and pulsing animations.
*/
uint8_t sine8(uint8_t theta) {
  float radians = (theta / 256.0f) * 2.0f * PI;
  return (uint8_t)(128.0f + 127.0f * sinf(radians));
}

// --- Static lighting ---

// A soft green glow while no controller has been found yet.
void set_disconnected_lighting() {
  fill_range(0, NUM_LEDS, rgb(0, 38, 0));
  strip.show();
}

/*
  Normal driving lights: white across the front, red across the rear.
  Only redrawn when something actually changed, rather than on every pass of
  loop().
*/
void set_standby_lighting() {
  strip.clear();

  if (running_lights_on) {
    uint8_t level = full_headlights ? HEADLIGHT_BRIGHT : HEADLIGHT_DIM;
    fill_range(LEFT_FRONT_START, CORNER_LEN * 2, rgb(level, level, level));
    fill_range(RIGHT_REAR_START, CORNER_LEN * 2, rgb(TAILLIGHT_LEVEL, 0, 0));
  }

  strip.show();
  standby_dirty = false;
}

// --- Animations ---

// Green pulse while searching for a controller.
void update_disconnected_lighting(unsigned long now) {
  static unsigned long last_update = 0;
  if (now - last_update < 10) return;
  last_update = now;

  uint8_t level = map(sine8(now / 16), 0, 255, 30, 80);
  fill_range(0, NUM_LEDS, rgb(0, level, 0));
  strip.show();
}

// Slow red blink: MY_CONTROLLER has not been filled in, so nothing can connect.
void update_no_address_lighting(unsigned long now) {
  static unsigned long last_blink = 0;
  static bool blink_on = false;
  if (now - last_blink < NO_ADDRESS_BLINK_MS) return;
  last_blink = now;

  blink_on = !blink_on;
  fill_range(0, NUM_LEDS, blink_on ? rgb(80, 0, 0) : rgb(0, 0, 0));
  strip.show();
}

/*
  Turn signals, as a bar of amber that grows outward from the center of the
  vehicle toward the corner that is turning. Front and rear animate together.
*/
void update_larson_scanner() {
  uint32_t amber = rgb(255, 100, 0);

  // Repaint the base lighting first, so the signal overlays normal lights.
  strip.clear();
  if (running_lights_on) {
    uint8_t level = full_headlights ? HEADLIGHT_BRIGHT : HEADLIGHT_DIM;
    fill_range(LEFT_FRONT_START, CORNER_LEN * 2, rgb(level, level, level));
    fill_range(RIGHT_REAR_START, CORNER_LEN * 2, rgb(TAILLIGHT_LEVEL, 0, 0));
  }

  int num_lit = (larson_phase % CORNER_LEN) + 1;

  if (led_mode == TURN_LEFT) {
    fill_range(LEFT_FRONT_START, CORNER_LEN, 0);
    fill_range(LEFT_REAR_START, CORNER_LEN, 0);
    for (int i = 0; i < num_lit; i++) {
      strip.setPixelColor(RIGHT_FRONT_START - 1 - i, amber);   // Front, center outward
      strip.setPixelColor(LEFT_REAR_START + i, amber);         // Rear, mirrored
    }
  } else if (led_mode == TURN_RIGHT) {
    fill_range(RIGHT_FRONT_START, CORNER_LEN, 0);
    fill_range(RIGHT_REAR_START, CORNER_LEN, 0);
    for (int i = 0; i < num_lit; i++) {
      strip.setPixelColor(RIGHT_FRONT_START + i, amber);
      strip.setPixelColor(LEFT_REAR_START - 1 - i, amber);
    }
  }

  strip.show();
}

/*
  KITT scanner. One bright dot sweeps across the front with a fading tail
  behind it, and the matching rear dot is drawn at REAR_LAST - position so the
  two stay side by side. Each call draws one step, then moves the dot one LED
  along, turning round at each end.
*/
void update_kitt_scanner() {
  strip.clear();

  uint32_t base = scale_color(rgb(KITT_R, KITT_G, KITT_B), KITT_BRIGHTNESS);

  for (int tail = 0; tail <= KITT_TAIL; tail++) {
    int pos = kitt_pos - (kitt_dir * tail);
    if (pos < 0 || pos > FRONT_LAST) continue;

    uint32_t color = scale_color(base, 255 >> tail);
    strip.setPixelColor(pos, color);
    strip.setPixelColor(REAR_LAST - pos, color);
  }

  strip.show();

  kitt_pos += kitt_dir;
  if (kitt_pos >= FRONT_LAST) {
    kitt_pos = FRONT_LAST;
    kitt_dir = -1;
  } else if (kitt_pos <= 0) {
    kitt_pos = 0;
    kitt_dir = 1;
  }
}

// --- Police lights ---

// The brightness of the police animation that is on, out of 255.
uint8_t police_brightness() {
  return (animation == ANIM_POLICE_HIGH) ? POLICE_LEVEL_HIGH : POLICE_LEVEL_LOW;
}

/*
  Draws one flash of the police lights: either the whole left side red or the
  whole right side blue, at the brightness of the police animation that is on,
  then swaps sides ready for the next flash.

  Each side is two runs of eight LEDs, one at the front and one at the rear,
  because the loop of LEDs wraps around the back of the vehicle.
*/
void update_police_lights() {
  strip.clear();
  uint8_t level = police_brightness();

  if (police_left_side) {
    uint32_t red = scale_color(rgb(255, 0, 0), level);
    fill_range(LEFT_FRONT_START, CORNER_LEN, red);    // LEDs 0-7
    fill_range(LEFT_REAR_START,  CORNER_LEN, red);    // LEDs 24-31
  } else {
    uint32_t blue = scale_color(rgb(0, 0, 255), level);
    fill_range(RIGHT_FRONT_START, CORNER_LEN, blue);  // LEDs 8-15
    fill_range(RIGHT_REAR_START,  CORNER_LEN, blue);  // LEDs 16-23
  }

  strip.show();
  police_left_side = !police_left_side;
  standby_dirty = false;
}

// --- Choosing an animation ---

// Prints "<animation name> ON" or "<animation name> OFF" on the Serial Monitor.
void print_animation(Animation a, bool on) {
  switch (a) {
    case ANIM_KITT:        Serial.print(F("KITT scanner"));                 break;
    case ANIM_POLICE_LOW:  Serial.printf("Police lights (brightness %d)", POLICE_LEVEL_LOW);  break;
    case ANIM_POLICE_HIGH: Serial.printf("Police lights (brightness %d)", POLICE_LEVEL_HIGH); break;
    case ANIM_NONE:        return;
  }
  Serial.println(on ? F(" ON") : F(" OFF"));
}

/*
  Called when the top, right or bottom button is pressed, with the animation
  that button belongs to. Only one animation can be on at a time:
  - If a DIFFERENT animation is on, it stops and this one starts in its place.
  - If THIS animation is already on, it stops, and the normal lights return.
  - If nothing is on, this one starts.

  The press also ends any turn signal that is showing, so you always see what
  the press did straight away.
*/
void select_animation(Animation chosen) {
  Animation old = animation;
  animation = (old == chosen) ? ANIM_NONE : chosen;

  print_animation(old, false);
  print_animation(animation, true);

  // Start the new animation from its beginning.
  if (animation == ANIM_KITT) {
    kitt_time = 0;      // Draw the first step straight away
    kitt_pos = 0;
    kitt_dir = 1;
  } else if (animation != ANIM_NONE) {
    police_left_side = true;
  }

  if (led_mode == TURN_LEFT || led_mode == TURN_RIGHT) led_mode = STANDBY;
  standby_dirty = true;   // Redraw straight away
}

/*
  Called when the running lights or headlights change. The left button only
  does this when no animation is showing, but D-pad up/down can do it at any
  time. While an animation is on, the new setting is remembered but not drawn,
  so the animation keeps its rhythm. The normal lights come back with the new
  setting when the animation goes off.
*/
void normal_lights_changed() {
  if (animation == ANIM_NONE) standby_dirty = true;
}

// --- Cancelling animations ---

// Is anything other than the normal lights on the strip right now?
bool animation_running() {
  return animation != ANIM_NONE ||
         led_mode == TURN_LEFT || led_mode == TURN_RIGHT;
}

/*
  Called when the left button is pressed while an animation is showing.
  Stops the animation and any turn signal, and goes back to the normal lights.

  If the D-pad is still held, the turn signal starts again straight away,
  because holding it means you want it.
*/
void cancel_animations() {
  if (led_mode == TURN_LEFT || led_mode == TURN_RIGHT) {
    led_mode = STANDBY;
    Serial.println(F("Turn signal cancelled"));
  }
  print_animation(animation, false);
  animation = ANIM_NONE;
  standby_dirty = true;   // Draw the normal lights straight away
}

// --- The lighting state machine ---

/*
  Called once per pass of loop(). Everything that animates is driven from here
  off millis(), so no animation ever blocks the vehicle from driving. That is
  why there are no delay() calls anywhere in this section: a delay(500) here
  would leave the motors running at their last speed for half a second.
*/
void update_lighting(unsigned long now, uint8_t dpad) {

  // ---- Turn signals take over while they run ----
  if (led_mode == TURN_LEFT || led_mode == TURN_RIGHT) {
    if (now - larson_time < LARSON_DELAY_MS) return;
    larson_time = now;

    larson_phase++;
    if (larson_phase >= CORNER_LEN) {
      larson_phase = 0;
      turn_signal_cycles++;

      // Keep going while the D-pad is held; otherwise stop after TURN_CYCLES.
      bool still_held = (led_mode == TURN_LEFT  && (dpad & DPAD_LEFT)) ||
                        (led_mode == TURN_RIGHT && (dpad & DPAD_RIGHT));
      if (!still_held && turn_signal_cycles >= TURN_CYCLES) {
        led_mode = STANDBY;     // The animation, if one is on, picks up again
        standby_dirty = true;
        return;
      }
    }
    update_larson_scanner();
    return;
  }

  if (led_mode != STANDBY) return;

  // ---- Otherwise, whichever animation is on, or the normal lights ----
  switch (animation) {
    case ANIM_KITT:
      if (now - kitt_time >= (unsigned long)KITT_SPEED_MS) {
        kitt_time = now;
        update_kitt_scanner();
      }
      break;

    case ANIM_POLICE_LOW:
    case ANIM_POLICE_HIGH:
      if (standby_dirty || now - police_time >= POLICE_STEP_MS) {
        police_time = now;
        update_police_lights();
      }
      break;

    case ANIM_NONE:
      if (standby_dirty) set_standby_lighting();
      break;
  }
}

// ===================================================================
// 6. MOTORS AND SERVOS
// ===================================================================
//
// Drive motors, servos, and the math that turns a thumbstick reading into a
// motor speed.

// --- Setup ---

void setup_motors() {
  // Channels 0-7 belong to the eight motor pins. Each channel is configured,
  // then attached to its pin; from there we write duty values to the channel.
  for (uint8_t ch = 0; ch < 8; ch++) {
    ledcSetup(ch, PWM_FREQ, PWM_RES);
  }
  for (int i = 0; i < 4; i++) {
    ledcAttachPin(motors[i].in1_pin, motors[i].in1_channel);
    ledcAttachPin(motors[i].in2_pin, motors[i].in2_channel);
  }
  coast_all();
}

void setup_servos() {
  ledcSetup(SERVO1_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO2_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO3_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO4_CH, SERVO_FREQ, SERVO_RES);

  ledcAttachPin(SERVO1_PIN, SERVO1_CH);
  ledcAttachPin(SERVO2_PIN, SERVO2_CH);
  ledcAttachPin(SERVO3_PIN, SERVO3_CH);
  ledcAttachPin(SERVO4_PIN, SERVO4_CH);

  write_servo(SERVO1_CH, SERVO_MID_US);
  write_servo(SERVO2_CH, SERVO_MID_US);
  write_servo(SERVO3_CH, SERVO_MID_US);
  write_servo(SERVO4_CH, SERVO_MID_US);
}

// --- Input processing ---

// A per-axis deadzone of -1 means "fall back to the global one".
int get_effective_deadzone(int specific_dz) {
  return (specific_dz >= 0) ? specific_dz : INPUT_DEADZONE;
}

/*
  Turns a raw stick reading into an output value.
  Inside the deadzone the answer is zero. Outside it, the remaining travel is
  scaled by the axis's scale factor and stretched across the output range.
*/
int remap_axis(int raw_value, int specific_dz, float scale_factor, int in_max, int out_max) {
  int dz = get_effective_deadzone(specific_dz);
  if (abs(raw_value) < dz) return 0;

  int sign   = (raw_value > 0) ? 1 : -1;
  int scaled = lround(abs(raw_value) * scale_factor);
  scaled = constrain(scaled, dz, in_max);

  return sign * map(scaled, dz, in_max, 0, out_max);
}

/*
  Moves one step of the ramp from where we are toward where we want to be.

  Written as `current += (target - current) * RAMP_FACTOR` in integer math,
  a remaining gap of one count truncates to a step of zero, and the vehicle
  stops one count short of its target forever. Forcing a minimum step of one
  count makes sure it always arrives.
*/
int ramp_towards(int current, int target) {
  int gap = target - current;
  if (gap == 0) return current;

  int step = (int)(gap * RAMP_FACTOR);
  if (step == 0) step = (gap > 0) ? 1 : -1;

  return current + step;
}

// --- Motor output ---

// Both pins low: the motor freewheels.
void coast_all() {
  for (int i = 0; i < 4; i++) {
    ledcWrite(motors[i].in1_channel, 0);
    ledcWrite(motors[i].in2_channel, 0);
  }
  current_left = current_right = 0;
}

// Both pins high: the motor is shorted across itself and stops hard.
void active_brake_motor(int motor_index) {
  ledcWrite(motors[motor_index].in1_channel, PWM_MAX);
  ledcWrite(motors[motor_index].in2_channel, PWM_MAX);
}

// Full power in one direction, used by the self-test.
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

/*
  HYBRID DRIVE

  Rather than pulsing one pin and holding the other low, this holds one pin
  high and pulses the other one low. The DRV8871 spends the off part of each
  cycle braking instead of coasting, which gives noticeably better control at
  low duty cycles.
*/
void set_motor_hybrid(uint8_t in1_ch, uint8_t in2_ch, int duty) {
  duty = constrain(duty, -PWM_MAX, PWM_MAX);

  if (duty == 0) {
    ledcWrite(in1_ch, 0);
    ledcWrite(in2_ch, 0);
    return;
  }

  int inverse = PWM_MAX - abs(duty);
  if (duty > 0) {
    ledcWrite(in1_ch, PWM_MAX);
    ledcWrite(in2_ch, inverse);
  } else {
    ledcWrite(in1_ch, inverse);
    ledcWrite(in2_ch, PWM_MAX);
  }
}

// Left pair and right pair, tank style.
void update_motors() {
  set_motor_hybrid(motors[0].in1_channel, motors[0].in2_channel, current_left);
  set_motor_hybrid(motors[2].in1_channel, motors[2].in2_channel, current_left);
  set_motor_hybrid(motors[1].in1_channel, motors[1].in2_channel, current_right);
  set_motor_hybrid(motors[3].in1_channel, motors[3].in2_channel, current_right);
}

// --- Servos ---

/*
  Sends one servo pulse of the requested width.

  The ESP32 counts duty, not microseconds. At 16-bit resolution a whole 20 ms
  cycle is 65536 counts, so counts = microseconds * 65536 / 20000. The long
  cast matters: 2000 * 65536 overflows a 16-bit int and is uncomfortably close
  to the limit of a 32-bit one.
*/
void write_servo(uint8_t channel, int microseconds) {
  microseconds = constrain(microseconds, SERVO_MIN_US, SERVO_MAX_US);
  long duty = (long)microseconds * 65536L / 20000L;
  ledcWrite(channel, duty);
}

/*
  The right stick drives four servos, one per direction. Pushing the stick a
  servo's way moves it from center (1.5 ms) to one end of its travel (2.0 ms),
  about 90 degrees; otherwise it sits centered. So a single stick can aim four
  independent things.
*/
void update_servos(ControllerPtr gp) {
  int rs_y = remap_axis(gp->axisRY(), INPUT_DEADZONE_RS_Y, rs_y_sc, AXIS_MAX, AXIS_MAX);
  int rs_x = remap_axis(gp->axisRX(), INPUT_DEADZONE_RS_X, rs_x_sc, AXIS_MAX, AXIS_MAX);

  // Servo 1: right stick up
  write_servo(SERVO1_CH, rs_y < 0 ? map(rs_y, -AXIS_MAX, 0, SERVO_MAX_US, SERVO_MID_US)
                                  : SERVO_MID_US);
  // Servo 3: right stick down
  write_servo(SERVO3_CH, rs_y > 0 ? map(rs_y, 0, AXIS_MAX, SERVO_MID_US, SERVO_MAX_US)
                                  : SERVO_MID_US);
  // Servo 2: right stick left
  write_servo(SERVO2_CH, rs_x < 0 ? map(rs_x, -AXIS_MAX, 0, SERVO_MAX_US, SERVO_MID_US)
                                  : SERVO_MID_US);
  // Servo 4: right stick right
  write_servo(SERVO4_CH, rs_x > 0 ? map(rs_x, 0, AXIS_MAX, SERVO_MID_US, SERVO_MAX_US)
                                  : SERVO_MID_US);
}

// --- Throttle modes ---

void cycleThrottleMode() {
  if (throttle_mode == MODE_FAST) {
    throttle_mode = MODE_NORMAL;
    active_ls_x_sc = NORMAL_MODE_X_SCALE;
    active_ls_y_sc = NORMAL_MODE_Y_SCALE;
    Serial.println(F("Throttle mode: NORMAL (reduced steering)"));
  } else {
    throttle_mode = MODE_FAST;
    active_ls_x_sc = ls_x_sc;
    active_ls_y_sc = ls_y_sc;
    Serial.println(F("Throttle mode: FAST (full steering)"));
  }
}

// ===================================================================
// 7. BLUETOOTH
// ===================================================================
//
// Controller connection, and the lock that ties this vehicle to the one
// controller named in MY_CONTROLLER.
//
// HOW THE LOCK WORKS
// Bluepad32 keeps an "allowlist", a guest list of Bluetooth addresses. If an
// address is on the list, that controller may connect; if it is not, the
// connection is turned away before it is ever accepted. At every boot this
// program empties the list and puts MY_CONTROLLER on it, so this file is
// always the single source of truth about who is allowed to drive.
//
// As a second check, onConnectedController() compares the address again and
// disconnects anything that does not match.

// --- Address helpers ---

// Prints a Bluetooth address the normal way, most significant byte first.
void printAddress(const uint8_t *address) {
  for (int i = 0; i < 6; i++) {
    if (address[i] < 0x10) Serial.print('0');
    Serial.print(address[i], HEX);
    if (i < 5) Serial.print(':');
  }
}

// Is this the controller we are allowed to drive with? Only if all six
// numbers of its address match MY_CONTROLLER exactly.
bool isMyController(const uint8_t *address) {
  for (int i = 0; i < 6; i++) {
    if (address[i] != MY_CONTROLLER[i]) return false;
  }
  return true;
}

// --- Connection callbacks ---

/*
  Bluepad32 calls this when a controller connects. It runs from BP32.update(),
  so it is safe to print and to touch program state from in here.
*/
void onConnectedController(ControllerPtr ctl) {
  ControllerProperties props = ctl->getProperties();

  if (!isMyController(props.btaddr)) {
    Serial.print(F("Refused a controller that does not belong to this vehicle: "));
    printAddress(props.btaddr);
    Serial.println();
    ctl->disconnect();
    return;
  }

  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] != nullptr) continue;

    myControllers[i] = ctl;
    controller_now_connected = true;

    Serial.print(F("Controller connected: "));
    Serial.print(ctl->getModelName());
    Serial.print(F(" at "));
    printAddress(props.btaddr);
    Serial.println();

    ctl->setPlayerLEDs(0x01);
    return;
  }

  // No free slot: refuse it.
  Serial.println(F("No free controller slot. Connection refused."));
  ctl->disconnect();
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      Serial.println(F("Controller disconnected."));
      return;
    }
  }
}

ControllerPtr firstConnectedController() {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] && myControllers[i]->isConnected()) {
      return myControllers[i];
    }
  }
  return nullptr;
}

// --- Setup ---

void setup_bluetooth() {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) myControllers[i] = nullptr;
  led_mode = DISCONNECTED;

  // Has somebody filled in MY_CONTROLLER yet? All zeros means "no".
  controller_address_set = false;
  for (int i = 0; i < 6; i++) {
    if (MY_CONTROLLER[i] != 0x00) controller_address_set = true;
  }

  if (!controller_address_set) {
    Serial.println();
    Serial.println(F("MY_CONTROLLER has not been filled in, so this vehicle does"));
    Serial.println(F("not know which controller belongs to it and will not drive."));
    Serial.println(F("Upload pathfinder_find_controller, follow the instructions,"));
    Serial.println(F("then paste the line it prints into the SETTINGS section."));
    return;   // Bluetooth is never started. The LEDs blink red.
  }

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);   // Gamepads only, no virtual mouse

  // Rebuild the allowlist from MY_CONTROLLER. This has to come after
  // BP32.setup(), which is what starts Bluetooth.
  bd_addr_t allowed;
  memcpy(allowed, MY_CONTROLLER, 6);
  uni_bt_allowlist_remove_all();
  uni_bt_allowlist_add_addr(allowed);
  uni_bt_allowlist_set_enabled(true);

  // New connections stay enabled. The allowlist is what keeps other people's
  // controllers out; leaving this on means our own controller can always get
  // in, even if the stored Bluetooth bonding keys are lost.
  BP32.enableNewBluetoothConnections(true);

  Serial.print(F("This vehicle is locked to the controller at "));
  printAddress(MY_CONTROLLER);
  Serial.println();
}

// ===================================================================
// 8. SERIAL CONSOLE
// ===================================================================
//
// The serial command interface, the stored settings behind it, and the live
// diagnostics. Open the Serial Monitor at 115200 baud and type "help".
//
// Everything set here is written to EEPROM, so a vehicle keeps its tuning
// across a power cycle. That is what makes it possible to tune a vehicle at
// the track without a laptop full of toolchains.

// --- Stored settings ---

void saveSettings() {
  EEPROM.put(ADDR_DEADZONE,      INPUT_DEADZONE);
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

/*
  Reads everything back, keeping the compiled-in default for any value that is
  out of range. A brand new ESP32 has EEPROM full of 0xFF, which reads back as
  -1 for an int and as nonsense for a float, so every value is range checked
  rather than trusted.
*/
void loadSettings() {
  int   saved_dz, saved_lsx_dz, saved_lsy_dz, saved_rsx_dz, saved_rsy_dz;
  float saved_lsy_sc, saved_lsx_sc, saved_rsy_sc, saved_rsx_sc;

  EEPROM.get(ADDR_DEADZONE,      saved_dz);
  EEPROM.get(ADDR_DEADZONE_LS_X, saved_lsx_dz);
  EEPROM.get(ADDR_DEADZONE_LS_Y, saved_lsy_dz);
  EEPROM.get(ADDR_DEADZONE_RS_X, saved_rsx_dz);
  EEPROM.get(ADDR_DEADZONE_RS_Y, saved_rsy_dz);
  EEPROM.get(ADDR_LS_Y_SC, saved_lsy_sc);
  EEPROM.get(ADDR_LS_X_SC, saved_lsx_sc);
  EEPROM.get(ADDR_RS_Y_SC, saved_rsy_sc);
  EEPROM.get(ADDR_RS_X_SC, saved_rsx_sc);

  if (saved_dz >= 0 && saved_dz <= DEADZONE_MAX) INPUT_DEADZONE = saved_dz;

  if (saved_lsx_dz >= DEADZONE_MIN && saved_lsx_dz <= DEADZONE_MAX) INPUT_DEADZONE_LS_X = saved_lsx_dz;
  if (saved_lsy_dz >= DEADZONE_MIN && saved_lsy_dz <= DEADZONE_MAX) INPUT_DEADZONE_LS_Y = saved_lsy_dz;
  if (saved_rsx_dz >= DEADZONE_MIN && saved_rsx_dz <= DEADZONE_MAX) INPUT_DEADZONE_RS_X = saved_rsx_dz;
  if (saved_rsy_dz >= DEADZONE_MIN && saved_rsy_dz <= DEADZONE_MAX) INPUT_DEADZONE_RS_Y = saved_rsy_dz;

  ls_y_sc = (saved_lsy_sc >= SCALE_MIN && saved_lsy_sc <= SCALE_MAX) ? saved_lsy_sc : 1.0f;
  ls_x_sc = (saved_lsx_sc >= SCALE_MIN && saved_lsx_sc <= SCALE_MAX) ? saved_lsx_sc : 1.0f;
  rs_y_sc = (saved_rsy_sc >= SCALE_MIN && saved_rsy_sc <= SCALE_MAX) ? saved_rsy_sc : 1.0f;
  rs_x_sc = (saved_rsx_sc >= SCALE_MIN && saved_rsx_sc <= SCALE_MAX) ? saved_rsx_sc : 1.0f;

  // Always start in NORMAL mode, whatever the saved scales say.
  throttle_mode  = MODE_NORMAL;
  active_ls_y_sc = NORMAL_MODE_Y_SCALE;
  active_ls_x_sc = NORMAL_MODE_X_SCALE;
}

// --- Commands ---

void printHelp() {
  Serial.println(F("\nAvailable commands:"));
  Serial.println(F("  help      - show this help"));
  Serial.println(F("  diag      - toggle live diagnostics"));
  Serial.println(F("  dynamics  - show the current input settings"));
  Serial.println(F("  dynamics [param value]... - change them"));
  Serial.println(F("      params: ls_y_dz, ls_y_sc, ls_x_dz, ls_x_sc,"));
  Serial.println(F("              rs_y_dz, rs_y_sc, rs_x_dz, rs_x_sc"));
  Serial.printf ("      dz: %d to %d (%d means use the global deadzone)\n",
                 DEADZONE_MIN, DEADZONE_MAX, DEADZONE_MIN);
  Serial.printf ("      sc: %.1f to %.1f\n", SCALE_MIN, SCALE_MAX);
  Serial.println(F("      example: dynamics ls_y_dz 30 ls_x_sc 1.1"));
}

void printDynamics() {
  Serial.println(F("Current input settings:"));
  Serial.printf("  Global deadzone: %d\n", INPUT_DEADZONE);
  Serial.printf("  LS_Y: dz=%d sc=%.2f\n", INPUT_DEADZONE_LS_Y, ls_y_sc);
  Serial.printf("  LS_X: dz=%d sc=%.2f\n", INPUT_DEADZONE_LS_X, ls_x_sc);
  Serial.printf("  RS_Y: dz=%d sc=%.2f\n", INPUT_DEADZONE_RS_Y, rs_y_sc);
  Serial.printf("  RS_X: dz=%d sc=%.2f\n", INPUT_DEADZONE_RS_X, rs_x_sc);
  Serial.printf("  Throttle mode: %s\n", throttle_mode == MODE_FAST ? "FAST" : "NORMAL");
}

// Applies one "name value" pair. Returns true if it was understood and valid.
bool applyDynamicsSetting(const String &key, const String &valStr) {
  // Deadzones are whole numbers.
  int  *dz_target = nullptr;
  if      (key == "ls_y_dz") dz_target = &INPUT_DEADZONE_LS_Y;
  else if (key == "ls_x_dz") dz_target = &INPUT_DEADZONE_LS_X;
  else if (key == "rs_y_dz") dz_target = &INPUT_DEADZONE_RS_Y;
  else if (key == "rs_x_dz") dz_target = &INPUT_DEADZONE_RS_X;

  if (dz_target != nullptr) {
    int v = valStr.toInt();
    if (v < DEADZONE_MIN || v > DEADZONE_MAX) {
      Serial.printf("  %s must be between %d and %d\n", key.c_str(), DEADZONE_MIN, DEADZONE_MAX);
      return false;
    }
    *dz_target = v;
    return true;
  }

  // Scale factors are decimals.
  float *sc_target = nullptr;
  if      (key == "ls_y_sc") sc_target = &ls_y_sc;
  else if (key == "ls_x_sc") sc_target = &ls_x_sc;
  else if (key == "rs_y_sc") sc_target = &rs_y_sc;
  else if (key == "rs_x_sc") sc_target = &rs_x_sc;

  if (sc_target != nullptr) {
    float v = valStr.toFloat();
    if (v < SCALE_MIN || v > SCALE_MAX) {
      Serial.printf("  %s must be between %.1f and %.1f\n", key.c_str(), SCALE_MIN, SCALE_MAX);
      return false;
    }
    *sc_target = v;
    // A scale change should be felt straight away in FAST mode.
    if (throttle_mode == MODE_FAST) {
      active_ls_y_sc = ls_y_sc;
      active_ls_x_sc = ls_x_sc;
    }
    return true;
  }

  Serial.printf("  Unknown setting: %s\n", key.c_str());
  return false;
}

/*
  Handles "dynamics" with no arguments (show everything) or with any number of
  name/value pairs after it.
*/
void handleDynamicsCommand(const String &args) {
  String remaining = args;
  remaining.trim();

  if (remaining.length() == 0) {
    printDynamics();
    return;
  }

  bool changed = false;

  while (remaining.length() > 0) {
    int split = remaining.indexOf(' ');
    if (split == -1) {
      Serial.printf("  %s has no value\n", remaining.c_str());
      break;
    }

    String key = remaining.substring(0, split);
    remaining = remaining.substring(split + 1);
    remaining.trim();
    if (remaining.length() == 0) {
      Serial.printf("  %s has no value\n", key.c_str());
      break;
    }

    split = remaining.indexOf(' ');
    String valStr = (split == -1) ? remaining : remaining.substring(0, split);
    remaining     = (split == -1) ? ""        : remaining.substring(split + 1);
    remaining.trim();

    if (applyDynamicsSetting(key, valStr)) changed = true;
  }

  if (changed) {
    saveSettings();
    Serial.println(F("Settings updated and saved."));
  } else {
    Serial.println(F("No changes applied."));
  }
}

/*
  Collects whatever characters have arrived and runs the command once the line
  is finished. It returns immediately either way, so the vehicle keeps driving
  while you type. Serial.readStringUntil() would stop loop() - and leave the
  motors running at their last speed - until the line came in.

  A line is finished by CR, LF or both, or by CONSOLE_IDLE_MS of quiet, which
  is what makes the Serial Monitor's "No line ending" setting work too.
*/
void handleSerialCommands() {
  static String input_line = "";
  static unsigned long last_char_time = 0;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    last_char_time = millis();

    if (c == '\n' || c == '\r') {
      if (input_line.length() > 0) {
        runCommand(input_line);
        input_line = "";
      }
      continue;
    }

    // Printable characters only, and capped, so a stuck sender cannot grow
    // the string until the ESP32 runs out of memory.
    if (c >= 32 && c < 127 && input_line.length() < 80) {
      input_line += c;
    }
  }

  if (input_line.length() > 0 && millis() - last_char_time >= CONSOLE_IDLE_MS) {
    runCommand(input_line);
    input_line = "";
  }
}

void runCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();
  if (cmd.length() == 0) return;

  if (cmd == "help") {
    printHelp();
  } else if (cmd == "pair" || cmd == "forget") {
    // Op 12 had these. Explain where pairing went rather than just "unknown".
    Serial.println(F("Op 13 has no pairing mode. The controller's address is"));
    Serial.println(F("MY_CONTROLLER, at the top of the SETTINGS section. Change it"));
    Serial.println(F("there and upload to use a different controller."));
  } else if (cmd == "diag") {
    diag_enabled = !diag_enabled;
    Serial.println(diag_enabled ? F("Diagnostics ON") : F("Diagnostics OFF"));
  } else if (cmd == "dynamics") {
    printDynamics();
  } else if (cmd.startsWith("dynamics ")) {
    handleDynamicsCommand(cmd.substring(9));
  } else {
    Serial.printf("Unknown command: %s. Type 'help'.\n", cmd.c_str());
  }
}

// --- Live diagnostics ---

/*
  Prints button codes the moment they change, and raw stick values at 4 Hz
  while any axis is off center. Useful for working out what a third-party
  controller actually sends, and for spotting a stick that no longer centers.
*/
void report_controller_state(ControllerPtr gp, unsigned long now) {
  uint32_t buttons = gp->buttons();
  uint8_t  misc    = gp->miscButtons();
  uint8_t  dpad    = gp->dpad();

  if (buttons != prev_reported_buttons || misc != prev_reported_misc || dpad != prev_reported_dpad) {
    Serial.printf("BUTTONS: 0x%04X | MISC: 0x%02X | DPAD: 0x%02X\n", buttons, misc, dpad);
    prev_reported_buttons = buttons;
    prev_reported_misc    = misc;
    prev_reported_dpad    = dpad;
  }

  if (now - last_joy_report_time < JOY_REPORT_INTERVAL_MS) return;
  last_joy_report_time = now;

  int lx = gp->axisX(),  ly = gp->axisY();
  int rx = gp->axisRX(), ry = gp->axisRY();
  int trigL = gp->brake(), trigR = gp->throttle();

  bool active = abs(ly) >= get_effective_deadzone(INPUT_DEADZONE_LS_Y) ||
                abs(lx) >= get_effective_deadzone(INPUT_DEADZONE_LS_X) ||
                abs(ry) >= get_effective_deadzone(INPUT_DEADZONE_RS_Y) ||
                abs(rx) >= get_effective_deadzone(INPUT_DEADZONE_RS_X) ||
                trigL >= 50 || trigR >= 50;

  if (active) {
    Serial.printf("JOY RAW: LX=%4d LY=%4d | RX=%4d RY=%4d | L2=%4d R2=%4d\n",
                  lx, ly, rx, ry, trigL, trigR);
  }
}

// ===================================================================
// 9. CURRENT SENSOR
// ===================================================================
//
// The INA219 voltage and current sensor. Gen 3 vehicles carry one; Gen 2
// vehicles do not. Everything in here does nothing on a vehicle without the
// sensor, so the same binary runs on both.
//
// The registers are driven directly rather than through a library, because the
// whole driver is about forty lines and seeing the register writes is useful
// if you ever have to read the datasheet.

// Is the sensor actually on the bus?
bool is_ina219_present() {
  Wire.beginTransmission(INA219_I2C_ADDR);
  return Wire.endTransmission() == 0;
}

void ina219_write16(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value >> 8);     // High byte first
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

uint16_t ina219_read16(uint8_t reg) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);          // Repeated start, do not release the bus
  Wire.requestFrom((uint8_t)INA219_I2C_ADDR, (uint8_t)2);
  if (Wire.available() < 2) return 0;
  // Two separate statements, so the high byte is always read first. In
  // (Wire.read() << 8) | Wire.read() C++ may call either read() first.
  uint8_t high = Wire.read();
  uint8_t low  = Wire.read();
  return (high << 8) | low;
}

void ina219_init() {
  ina219_write16(0x00, 0x8000);   // Reset
  delay(1);
  ina219_write16(0x00, 0b0011100111001111);   // 32V range (bit 13), 12-bit, continuous
  ina219_write16(0x05, 20999);                // Calibration for this shunt
}

void reset_current_tracking() {
  current_sum = 0.0f;
  current_count = 0;
  current_peak_pos = 0.0f;
  current_peak_neg = 0.0f;
}

/*
  Takes one reading and folds it into the running average and peaks.
  Called every pass of loop(), so "average" means "since the last reset".
*/
void update_ina219() {
  if (!(vehicle_config.capabilities & CAP_CURRENT_MON)) return;

  int16_t raw = (int16_t)ina219_read16(0x01);   // Shunt voltage register
  float current_a = (raw * 0.01f) / (SHUNT_RESISTOR_OHMS * 1000.0f);

  if (current_a > current_peak_pos) current_peak_pos = current_a;
  if (current_a < current_peak_neg) current_peak_neg = current_a;

  current_sum += current_a;
  current_count++;
}

float get_average_current() {
  return current_count > 0 ? current_sum / current_count : 0.0f;
}

float get_bus_voltage() {
  if (!(vehicle_config.capabilities & CAP_CURRENT_MON)) return 0.0f;
  uint16_t value = ina219_read16(0x02);
  return (float)((value >> 3) * 4) * 0.001f;   // Raw counts to volts
}

// ===================================================================
// 10. SELF-TEST
// ===================================================================
//
// The powered-up motor check, Gen 3 vehicles only.
//
// Jumper GPIO 34 high at boot and each motor is driven forward and then in
// reverse while the current sensor watches. A motor that is disconnected draws
// almost nothing; a motor that is jammed draws a lot and keeps drawing it. A
// healthy one spikes as it breaks away, then settles. Both thresholds are
// measured relative to a baseline taken with every motor stopped, so a vehicle
// whose electronics idle at half an amp is judged fairly.
//
// The whole thing is a state machine driven from loop(), so the LEDs keep
// animating and the serial console keeps responding throughout.

void run_self_test() {
  unsigned long now = millis();
  static bool    blink_state = false;
  static uint8_t sub_phase = 0;          // 0 spinning, 1 braking
  static bool    result_announced = false;

  // Any button press clears the pass/fail screen and hands over to driving.
  if (test_state == TEST_RESULT) {
    ControllerPtr gp = firstConnectedController();
    if (gp && gp->buttons() != 0) {
      Serial.println(F("Self-test cleared. Entering DRIVE mode."));
      test_state = TEST_DISABLED;
      result_announced = false;
      led_mode = STANDBY;
      standby_dirty = true;

      // The press that cleared the test must not also count as a press of
      // that button's normal job, such as switching the police lights on.
      // Recording the buttons as already down means loop() ignores them
      // until they are let go and pressed again.
      prev_running_lights_btn = gp->x();
      prev_kitt_btn           = gp->y();
      prev_police_low_btn     = gp->b();
      prev_police_high_btn    = gp->a();
      prev_thumbL             = gp->thumbL();
      return;
    }
  }

  switch (test_state) {

    case CHECK_TRIGGER:
      // Give the pin a moment to settle before believing it.
      if (now >= TEST_START_DELAY) {
        bool jumpered = (digitalRead(TEST_PIN) == HIGH);
        test_state = (jumpered && (vehicle_config.capabilities & CAP_SELF_TEST))
                     ? START_TEST : TEST_DISABLED;
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
      test_phase_start = now;
      test_state = BASELINE_CURRENT;
      break;

    case BASELINE_CURRENT:
      // Measure what the vehicle draws with everything stopped.
      update_ina219();
      if (now - test_phase_start >= BASELINE_DURATION) {
        baselineCurrent = get_average_current();
        Serial.printf("Baseline current: %.3fA\n", baselineCurrent);
        reset_current_tracking();
        test_phase_start = now;
        test_state = RUNNING_TESTS;
      }
      break;

    case RUNNING_TESTS:
      if (current_test_step >= NUM_TEST_STEPS) {
        test_phase_start = now;
        test_state = TEST_RESULT;
        break;
      }

      if (sub_phase == 0) {
        // ---- Spinning ----
        set_motor_full(test_sequence[current_test_step].motor_index,
                       test_sequence[current_test_step].direction);
        update_ina219();

        if (now - test_phase_start > SPIN_DURATION) {
          float avg  = get_average_current();
          float peak_above = current_peak_pos - baselineCurrent;
          float avg_above  = avg - baselineCurrent;

          Serial.printf("TEST [%s] -> I-Avg: %.3fA (%+.3f) | I-Max: %.3fA (%+.3f)",
                        test_sequence[current_test_step].name,
                        avg, avg_above, current_peak_pos, peak_above);

          // Healthy: a clear spike on break-away, then settles back down.
          bool healthy = (peak_above > PEAK_ABOVE_BASELINE_A) &&
                         (avg_above  < AVG_ABOVE_BASELINE_A);

          if (healthy) {
            Serial.println(F(" | PASS"));
            sub_phase = 1;
            test_phase_start = now;
          } else {
            Serial.println(F(" | FAIL"));
            Serial.println(peak_above <= PEAK_ABOVE_BASELINE_A
                           ? F("  No current spike: motor disconnected or not turning?")
                           : F("  Current stayed high: motor jammed or binding?"));
            coast_all();
            self_test_passed = false;
            test_phase_start = now;
            test_state = TEST_RESULT;
          }
        }
      } else {
        // ---- Braking, so the next motor starts from standstill ----
        active_brake_motor(test_sequence[current_test_step].motor_index);
        if (now - test_phase_start > BRAKE_DURATION) {
          coast_all();
          current_test_step++;
          sub_phase = 0;
          reset_current_tracking();
          test_phase_start = now;
        }
      }
      break;

    case TEST_RESULT:
      if (!result_announced) {
        Serial.printf("--- SELF-TEST %s ---\n", self_test_passed ? "PASSED" : "FAILED");
        Serial.println(F("Press any button on the controller to drive."));
        result_announced = true;
      }
      // Blink green for pass, red for fail, until somebody acknowledges it.
      if (now - test_phase_start >= PASS_BLINK_DELAY) {
        test_phase_start = now;
        blink_state = !blink_state;
        uint32_t color = blink_state ? (self_test_passed ? rgb(0, 255, 0) : rgb(255, 0, 0))
                                      : rgb(0, 0, 0);
        fill_range(0, NUM_LEDS, color);
        strip.show();
      }
      break;

    case TEST_DISABLED:
      break;
  }
}
