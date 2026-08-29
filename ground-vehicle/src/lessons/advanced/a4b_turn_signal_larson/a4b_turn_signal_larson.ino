/*
  a4b_turn_signal_larson.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 4

  WHAT THIS PROGRAM DOES
  ----------------------
  The turn signal animation from Pathfinder_Op_Program12, isolated so you can
  step through it one frame at a time. A bar of amber grows outward from the
  centre of the vehicle towards whichever corner is turning, front and rear
  together. Nothing moves.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Library: "Adafruit NeoPixel" by Adafruit.

  COMMANDS (115200 baud)
  ----------------------
    left | right     start a signal. Three cycles, then back to standby.
    hold left|right  keep signalling until you type 'release'
    release          stop holding
    step             advance exactly one frame, and print the indices
    speed <ms>       frame time, default 50
    cycles <n>       cycles before it gives up, default 3
    off

  THE GEOMETRY, WHICH IS THE HARD PART
  ------------------------------------
  The strip is one loop, so the two halves of each side run in opposite
  directions and the arithmetic is not symmetric:

        front, left to right :   0 .. 15      left half 0-7,   right half 8-15
        rear,  right to left :  16 .. 31      right half 16-23, left half 24-31

  For a LEFT signal, the bar has to grow from the centre outward on both bars.
  At the front the centre is index 8, and going left means counting DOWN, so
  the i-th LED is (8 - 1 - i). At the rear the left half starts at 24 and runs
  outward as the index rises, so the i-th LED is (24 + i).

        RIGHT_FRONT_START - 1 - i     front, growing left from centre
        LEFT_REAR_START + i           rear, mirroring it

  For a RIGHT signal it is the other way round:

        RIGHT_FRONT_START + i         front, growing right from centre
        LEFT_REAR_START - 1 - i       rear, mirroring it

  Use "step" and read the indices as they print. It is much easier to see than
  to derive.

  WHY IT REPAINTS THE BASE LIGHTING FIRST
  ---------------------------------------
  Each frame clears the whole strip, paints the headlights and tail lights back
  on, blanks the side that is signalling, and then draws the amber bar over the
  top. Painting only the amber would leave the previous frame underneath.

  WHY IT FINISHES ITS CYCLES
  --------------------------
  A real indicator does not stop mid-blink when you let go of the stalk. The
  D-pad only ever STARTS the animation; the animation counts its own cycles and
  keeps going until it reaches TURN_CYCLES, and only then hands back to
  STANDBY. Holding the D-pad keeps resetting that count.

  WHAT TO TRY
  -----------
  1. "left", then "right", then "step" several times and read the indices.
  2. "hold left", wait, then "release". Where does it stop?
  3. "speed 200" and watch the bar build.
  4. Change the LEFT branch to use LEFT_FRONT_START + i and see what breaks.
  5. Make it fade the trailing edge instead of switching it off abruptly.
*/

#include <Adafruit_NeoPixel.h>

const int LED_PIN  = 5;
const int NUM_LEDS = 32;

// The four quarters of the loop, as Op 12 names them.
const int LEFT_FRONT_START  = 0;
const int RIGHT_FRONT_START = 8;
const int RIGHT_REAR_START  = 16;
const int LEFT_REAR_START   = 24;
const int CORNER_LEN        = 8;

const uint8_t HEADLIGHT_DIM   = 77;
const uint8_t TAILLIGHT_LEVEL = 77;

enum LEDMode { STANDBY, TURN_LEFT, TURN_RIGHT };

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

LEDMode led_mode = STANDBY;
bool running_lights_on = true;
bool standby_dirty = true;

int larson_delay_ms = 50;
int turn_cycles_target = 3;

unsigned long larson_time = 0;
int larson_phase = 0;
int turn_signal_cycles = 0;

bool holding_left = false, holding_right = false;
bool single_step = false;      // "step" advances one frame with the timer off
bool verbose = false;

String input_line = "";

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return strip.Color(r, g, b);
}

void fillRange(int first, int count, uint32_t colour) {
  for (int i = first; i < first + count && i < NUM_LEDS; i++) {
    strip.setPixelColor(i, colour);
  }
}

void setStandbyLighting() {
  strip.clear();
  if (running_lights_on) {
    fillRange(LEFT_FRONT_START, CORNER_LEN * 2, rgb(HEADLIGHT_DIM, HEADLIGHT_DIM, HEADLIGHT_DIM));
    fillRange(RIGHT_REAR_START, CORNER_LEN * 2, rgb(TAILLIGHT_LEVEL, 0, 0));
  }
  strip.show();
  standby_dirty = false;
}

/*
  Draws one frame of the turn signal: base lighting, the signalling side
  blanked, then the amber bar grown out from the centre.
*/
void drawLarsonFrame() {
  uint32_t amber = rgb(255, 100, 0);

  // Repaint the base lighting first, so the signal overlays normal lights.
  strip.clear();
  if (running_lights_on) {
    fillRange(LEFT_FRONT_START, CORNER_LEN * 2, rgb(HEADLIGHT_DIM, HEADLIGHT_DIM, HEADLIGHT_DIM));
    fillRange(RIGHT_REAR_START, CORNER_LEN * 2, rgb(TAILLIGHT_LEVEL, 0, 0));
  }

  int num_lit = (larson_phase % CORNER_LEN) + 1;

  if (led_mode == TURN_LEFT) {
    fillRange(LEFT_FRONT_START, CORNER_LEN, 0);
    fillRange(LEFT_REAR_START, CORNER_LEN, 0);

    if (verbose) Serial.printf("phase %d, %d lit:  front", larson_phase, num_lit);
    for (int i = 0; i < num_lit; i++) {
      int front = RIGHT_FRONT_START - 1 - i;   // Grows left, so counts down
      int rear  = LEFT_REAR_START + i;         // Mirrors it, counting up
      strip.setPixelColor(front, amber);
      strip.setPixelColor(rear, amber);
      if (verbose) Serial.printf(" %d/%d", front, rear);
    }
    if (verbose) Serial.println();

  } else if (led_mode == TURN_RIGHT) {
    fillRange(RIGHT_FRONT_START, CORNER_LEN, 0);
    fillRange(RIGHT_REAR_START, CORNER_LEN, 0);

    if (verbose) Serial.printf("phase %d, %d lit:  front", larson_phase, num_lit);
    for (int i = 0; i < num_lit; i++) {
      int front = RIGHT_FRONT_START + i;       // Grows right, so counts up
      int rear  = LEFT_REAR_START - 1 - i;     // Mirrors it, counting down
      strip.setPixelColor(front, amber);
      strip.setPixelColor(rear, amber);
      if (verbose) Serial.printf(" %d/%d", front, rear);
    }
    if (verbose) Serial.println();
  }

  strip.show();
}

/*
  Advances the animation by one phase and hands back to STANDBY once it has
  finished its cycles, unless somebody is still holding the direction.
*/
void advanceLarson() {
  larson_phase++;

  if (larson_phase >= CORNER_LEN) {
    larson_phase = 0;
    turn_signal_cycles++;

    bool still_held = (led_mode == TURN_LEFT  && holding_left) ||
                      (led_mode == TURN_RIGHT && holding_right);

    if (!still_held && turn_signal_cycles >= turn_cycles_target) {
      led_mode = STANDBY;
      standby_dirty = true;
      Serial.println(F("signal finished"));
      return;
    }
  }

  drawLarsonFrame();
}

void startSignal(LEDMode direction) {
  led_mode = direction;
  larson_phase = 0;
  turn_signal_cycles = 0;
  larson_time = 0;
  drawLarsonFrame();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  strip.begin();
  strip.setBrightness(255);
  strip.clear();
  strip.show();

  Serial.println();
  Serial.println(F("=== Turn signals ==="));
  Serial.println(F("Type 'help'."));
}

void loop() {
  unsigned long now = millis();

  handleSerial();

  if (led_mode == STANDBY) {
    if (standby_dirty) setStandbyLighting();
    return;
  }

  if (single_step) {
    single_step = false;
    advanceLarson();
    return;
  }

  if (larson_delay_ms > 0 && now - larson_time >= (unsigned long)larson_delay_ms) {
    larson_time = now;
    advanceLarson();
  }
}

void handleSerial() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (input_line.length() > 0) {
        runCommand(input_line);
        input_line = "";
      }
    } else if (c >= 32 && c < 127 && input_line.length() < 40) {
      input_line += c;
    }
  }
}

void runCommand(String line) {
  line.trim();
  line.toLowerCase();

  if (line == "help") {
    Serial.println(F("  left | right      three cycles then standby"));
    Serial.println(F("  hold left|right   keep going until 'release'"));
    Serial.println(F("  release"));
    Serial.println(F("  step              one frame, printing the indices"));
    Serial.println(F("  speed <ms>        frame time, 0 freezes it"));
    Serial.println(F("  cycles <n>"));
    Serial.println(F("  off"));

  } else if (line == "left") {
    holding_left = holding_right = false;
    startSignal(TURN_LEFT);
    Serial.println(F("signalling LEFT"));

  } else if (line == "right") {
    holding_left = holding_right = false;
    startSignal(TURN_RIGHT);
    Serial.println(F("signalling RIGHT"));

  } else if (line == "hold left") {
    holding_left = true;
    holding_right = false;
    startSignal(TURN_LEFT);
    Serial.println(F("holding LEFT"));

  } else if (line == "hold right") {
    holding_right = true;
    holding_left = false;
    startSignal(TURN_RIGHT);
    Serial.println(F("holding RIGHT"));

  } else if (line == "release") {
    holding_left = holding_right = false;
    Serial.println(F("released - it will finish its current cycles"));

  } else if (line == "step") {
    if (led_mode == STANDBY) {
      Serial.println(F("Start a signal first."));
      return;
    }
    verbose = true;
    single_step = true;

  } else if (line == "off") {
    holding_left = holding_right = false;
    led_mode = STANDBY;
    standby_dirty = true;
    verbose = false;
    Serial.println(F("standby"));

  } else if (line.startsWith("speed ")) {
    larson_delay_ms = constrain(line.substring(6).toInt(), 0, 2000);
    Serial.printf("frame time %d ms%s\n", larson_delay_ms,
                  larson_delay_ms == 0 ? "  (frozen - use 'step')" : "");

  } else if (line.startsWith("cycles ")) {
    turn_cycles_target = constrain(line.substring(7).toInt(), 1, 50);
    Serial.printf("cycles %d\n", turn_cycles_target);

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
