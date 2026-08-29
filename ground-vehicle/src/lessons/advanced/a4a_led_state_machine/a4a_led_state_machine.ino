/*
  a4a_led_state_machine.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 4

  WHAT THIS PROGRAM DOES
  ----------------------
  Runs the lighting state machine out of Pathfinder_Op_Program12, on its own,
  driven from the serial console instead of from a controller. Type a mode name
  and watch the LEDs. Nothing moves.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Library: "Adafruit NeoPixel" by Adafruit.

  COMMANDS (115200 baud)
  ----------------------
    disconnected | standby | pairing | kitt
    bright | dim | lights on | lights off
    load             report how long a pass of loop() takes
    status

  WHAT A STATE MACHINE IS, AND WHY LIGHTING NEEDS ONE
  ---------------------------------------------------
  The vehicle is always in exactly ONE lighting mode. Each mode knows how to
  draw itself and when to hand over to another. Written as an enum and a switch
  rather than a pile of booleans, two useful properties fall out:

    - Two modes can never be half-on at once, because there is one variable.
    - Adding a mode is one enum value and one branch, not a new combination to
      test against every existing flag.

  Op 12 has six: DISCONNECTED, STANDBY, TURN_LEFT, TURN_RIGHT, PAIRING and
  KITT_SCANNER. This sketch has four of them.

  EVERY ANIMATION IS NON-BLOCKING
  -------------------------------
  Not one function in here calls delay(). Each checks the clock, returns
  immediately if it is not time yet, and draws exactly one frame when it is:

        if (now - last_update < INTERVAL) return;
        last_update = now;

  That is what lets the vehicle drive, read its controller and animate at the
  same time. An animation written with delay() would stop the vehicle dead for
  the length of the animation - which is precisely what the old System Test
  program did, freezing the vehicle for two seconds while holding whatever
  motor command was last set.

  DIRTY FLAGS FOR THE STATIC STATES
  ---------------------------------
  STANDBY is not an animation. It is one picture that only changes when the
  headlights or the running lights change. Redrawing it every pass would push
  32 LEDs out thousands of times a second for nothing.

  So it has a "dirty" flag. Anything that alters the picture sets it, and the
  state machine only redraws when it is set. Op 11.2 pushed standby lighting on
  every single pass of loop(); Op 12 added standby_dirty to stop that.

  WHAT TO TRY
  -----------
  1. Switch between the modes and watch the handover.
  2. Type "load" in each mode. Which mode costs the most time per pass?
  3. In "standby", type "bright" and "dim" and watch when the redraw happens.
  4. Comment out the "standby_dirty = false" at the end of setStandbyLighting
     and check "load" again. How much did that one line cost?
  5. Add a fifth mode of your own: a slow red pulse called ALARM.
*/

#include <Adafruit_NeoPixel.h>

// ===================================================================
// CONFIGURATION
// ===================================================================

const int LED_PIN  = 5;
const int NUM_LEDS = 32;

const int FRONT_LAST = 15;
const int REAR_LAST  = 31;

const uint8_t HEADLIGHT_DIM    = 77;
const uint8_t HEADLIGHT_BRIGHT = 255;
const uint8_t TAILLIGHT_LEVEL  = 77;

const int BREATHE_PERIOD_MS = 2000;
const uint8_t BREATHE_MAX   = 77;

const int     KITT_SPEED_MS   = 35;
const int     KITT_CYCLES     = 3;
const int     KITT_TAIL       = 3;
const uint8_t KITT_BRIGHTNESS = 200;
const uint8_t KITT_R = 0, KITT_G = 0, KITT_B = 255;

enum LEDMode { DISCONNECTED, STANDBY, PAIRING, KITT_SCANNER };

// ===================================================================
// STATE
// ===================================================================

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

LEDMode led_mode = DISCONNECTED;
bool full_headlights   = false;
bool running_lights_on = true;
bool standby_dirty     = true;

unsigned long breathe_time = 0;
unsigned long kitt_time = 0;
int kitt_pos = 0, kitt_dir = 1, kitt_cycles_done = 0;
LEDMode kitt_return_mode = STANDBY;

bool report_load = false;
unsigned long load_window_start = 0;
long load_passes = 0;

String input_line = "";

// ===================================================================
// COLOUR HELPERS
// ===================================================================

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return strip.Color(r, g, b);
}

// Scales a packed colour towards black. 255 leaves it alone, 0 turns it off.
uint32_t scaleColour(uint32_t colour, uint8_t scale) {
  uint8_t r = (uint8_t)(colour >> 16);
  uint8_t g = (uint8_t)(colour >> 8);
  uint8_t b = (uint8_t)colour;
  return strip.Color((r * scale) / 255, (g * scale) / 255, (b * scale) / 255);
}

void fillRange(int first, int count, uint32_t colour) {
  for (int i = first; i < first + count && i < NUM_LEDS; i++) {
    strip.setPixelColor(i, colour);
  }
}

/*
  An 8-bit sine: feed it 0..255 for one full turn and it returns 0..255 with
  128 as the midpoint. Op 11.2 got this free from FastLED; Op 12 writes it out,
  which makes the maths visible instead of magic.
*/
uint8_t sine8(uint8_t theta) {
  float radians = (theta / 256.0f) * 2.0f * PI;
  return (uint8_t)(128.0f + 127.0f * sinf(radians));
}

// ===================================================================
// THE MODES
// ===================================================================

// STANDBY: one static picture. Redrawn only when the dirty flag is set.
void setStandbyLighting() {
  strip.clear();

  if (running_lights_on) {
    uint8_t level = full_headlights ? HEADLIGHT_BRIGHT : HEADLIGHT_DIM;
    fillRange(0, 16, rgb(level, level, level));
    fillRange(16, 16, rgb(TAILLIGHT_LEVEL, 0, 0));
  }

  strip.show();
  standby_dirty = false;
}

// DISCONNECTED: a slow green pulse while looking for a controller.
void updateDisconnectedLighting(unsigned long now) {
  static unsigned long last_update = 0;
  if (now - last_update < 10) return;
  last_update = now;

  uint8_t level = map(sine8(now / 16), 0, 255, 30, 80);
  fillRange(0, NUM_LEDS, rgb(0, level, 0));
  strip.show();
}

// PAIRING: blue breathing.
void updatePairingBreathing(unsigned long now) {
  if (now - breathe_time < 20) return;
  breathe_time = now;

  uint8_t phase = (uint8_t)((now % BREATHE_PERIOD_MS) * 255 / BREATHE_PERIOD_MS);
  uint8_t level = map(sine8(phase), 0, 255, 0, BREATHE_MAX);

  fillRange(0, NUM_LEDS, scaleColour(rgb(0, 50, 255), level));
  strip.show();
}

/*
  KITT: one bright dot with a fading tail, mirrored front to rear. The LED
  directly behind front LED p is always REAR_LAST - p.
*/
void drawKittFrame() {
  strip.clear();

  uint32_t base = scaleColour(rgb(KITT_R, KITT_G, KITT_B), KITT_BRIGHTNESS);

  for (int tail = 0; tail <= KITT_TAIL; tail++) {
    int pos = kitt_pos - (kitt_dir * tail);
    if (pos < 0 || pos > FRONT_LAST) continue;

    uint32_t colour = scaleColour(base, 255 >> tail);
    strip.setPixelColor(pos, colour);
    strip.setPixelColor(REAR_LAST - pos, colour);
  }

  strip.show();
}

void startKittScanner() {
  // Remember what we were showing, so we can go back to it afterwards.
  if (led_mode != KITT_SCANNER) kitt_return_mode = led_mode;

  led_mode = KITT_SCANNER;
  kitt_time = 0;
  kitt_pos = 0;
  kitt_dir = 1;
  kitt_cycles_done = 0;
}

// ===================================================================
// THE STATE MACHINE
// ===================================================================

/*
  Called once per pass of loop(). Exactly one branch runs, and none of them
  block.
*/
void updateLighting(unsigned long now) {
  switch (led_mode) {

    case PAIRING:
      updatePairingBreathing(now);
      break;

    case DISCONNECTED:
      updateDisconnectedLighting(now);
      break;

    case KITT_SCANNER:
      if (now - kitt_time < (unsigned long)KITT_SPEED_MS) return;
      kitt_time = now;

      drawKittFrame();

      kitt_pos += kitt_dir;
      if (kitt_pos >= FRONT_LAST) {
        kitt_pos = FRONT_LAST;
        kitt_dir = -1;
      } else if (kitt_pos <= 0) {
        kitt_pos = 0;
        kitt_dir = 1;

        kitt_cycles_done++;
        if (kitt_cycles_done >= KITT_CYCLES) {
          led_mode = kitt_return_mode;
          standby_dirty = true;
          Serial.println(F("scanner finished, handing back"));
        }
      }
      break;

    case STANDBY:
      if (standby_dirty) setStandbyLighting();
      break;
  }
}

// ===================================================================
// SETUP AND LOOP
// ===================================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  strip.begin();
  strip.setBrightness(255);
  strip.clear();
  strip.show();

  Serial.println();
  Serial.println(F("=== Lighting state machine ==="));
  Serial.println(F("Type 'help'."));
}

void loop() {
  unsigned long now = millis();
  load_passes++;

  handleSerial();
  updateLighting(now);

  if (report_load && now - load_window_start >= 1000) {
    Serial.printf("%ld passes of loop() in the last second\n", load_passes);
    load_passes = 0;
    load_window_start = now;
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
    Serial.println(F("  disconnected | standby | pairing | kitt"));
    Serial.println(F("  bright | dim | lights on | lights off"));
    Serial.println(F("  load     report passes of loop() per second"));
    Serial.println(F("  status"));

  } else if (line == "disconnected") {
    led_mode = DISCONNECTED;
    Serial.println(F("mode: DISCONNECTED"));

  } else if (line == "standby") {
    led_mode = STANDBY;
    standby_dirty = true;
    Serial.println(F("mode: STANDBY"));

  } else if (line == "pairing") {
    led_mode = PAIRING;
    breathe_time = millis();
    Serial.println(F("mode: PAIRING"));

  } else if (line == "kitt") {
    startKittScanner();
    Serial.println(F("mode: KITT_SCANNER"));

  } else if (line == "bright") {
    full_headlights = true;
    standby_dirty = true;
    Serial.println(F("headlights bright"));

  } else if (line == "dim") {
    full_headlights = false;
    standby_dirty = true;
    Serial.println(F("headlights dim"));

  } else if (line == "lights on") {
    running_lights_on = true;
    standby_dirty = true;
    Serial.println(F("running lights on"));

  } else if (line == "lights off") {
    running_lights_on = false;
    standby_dirty = true;
    Serial.println(F("running lights off"));

  } else if (line == "load") {
    report_load = !report_load;
    load_passes = 0;
    load_window_start = millis();
    Serial.println(report_load ? F("load reporting on") : F("load reporting off"));

  } else if (line == "status") {
    const char *names[] = { "DISCONNECTED", "STANDBY", "PAIRING", "KITT_SCANNER" };
    Serial.printf("mode %s, headlights %s, running lights %s, dirty %s\n",
                  names[led_mode],
                  full_headlights ? "bright" : "dim",
                  running_lights_on ? "on" : "off",
                  standby_dirty ? "yes" : "no");

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
