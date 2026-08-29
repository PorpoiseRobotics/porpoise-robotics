/*
  pathfinder_ps3.ino
  Porpoise Robotics - Pathfinder vehicle, beginner program (PS3 controller)

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives the Pathfinder with a PS3 controller, aims two servos with the right
  thumbstick, and runs the 32 LEDs as headlights, tail lights, turn signals,
  and a "KITT" scanner (like Knight Rider, or a Cylon in Battlestar Galactica).

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > Boards Manager, search "esp32", choose version 3.0.7.
       Then pick Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  2. Libraries (Tools > Manage Libraries):
       - "PS3 Controller Host" by Jeffrey van Pernis   (gives us Ps3Controller.h)
       - "Adafruit NeoPixel" by Adafruit               (gives us Adafruit_NeoPixel.h)
  A library is code somebody else already wrote that your program can use. The
  compiler is what turns your code and the libraries together into the machine
  language the ESP32 actually runs.

  PAIRING THE CONTROLLER
  ----------------------
  A PS3 controller only talks to the one Bluetooth address it was paired with.
  Use SixaxisPairTool on a PC to write the address below into the controller.
  Any address works, as long as the controller and this program use the SAME one.
  Write that address into PS3_MAC_ADDRESS below.

  CONTROLS
  --------
    Left stick        Drive. Up = forward, down = reverse, left/right = turn.
    Left stick CLICK  Sharp steering on / off  (this is the L3 button)
    Right stick X     Servo 1 (pin 25)
    Right stick Y     Servo 2 (pin 26)
    TRIANGLE          KITT scanner on / off
    SQUARE            All lights on / off
    D-pad UP          Headlights bright
    D-pad DOWN        Headlights dim

  HOW THE LEDS ARE ARRANGED
  -------------------------
  The 32 LEDs run in one loop around the vehicle:
      LEDs  0..15  across the FRONT, left to right
      LEDs 16..31  across the REAR,  right to left
  So LED 0 and LED 31 are both on the left side of the vehicle, and LED 15 and
  LED 16 are both on the right. That means the LED directly behind front LED
  number "p" is always LED number "31 - p". The scanner uses that trick to keep
  the front dot and the rear dot lined up with each other.

  SAFETY
  ------
  Put the vehicle up on a block so the wheels spin free the first time you
  upload a change. It is much easier to debug a robot that cannot drive away.
*/

#include <Ps3Controller.h>
#include <Adafruit_NeoPixel.h>

// ===================================================================
// SETTINGS - change these numbers to change how the vehicle behaves
// ===================================================================

// --- Bluetooth address this program answers to -----------------------
const char *PS3_MAC_ADDRESS = "02:02:03:04:05:08"; // The first number in the MAC address needs to be even for the program to work

// --- LED strip -------------------------------------------------------
const int LED_PIN        = 5;    // GPIO 5 controls the LED strip
const int LED_COUNT      = 32;   // Two bars of 16 LEDs
const int LED_BRIGHTNESS = 120;  // Master brightness, 0 (off) to 255 (blinding)

// LED numbers for each part of the loop. Naming them keeps the code readable.
const int FRONT_FIRST = 0; // Front left
const int FRONT_LAST  = 15; // Front right
const int REAR_FIRST  = 16; // Rear right
const int REAR_LAST   = 31; // Rear left

// The strip object. We talk to the LEDs through this.
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Motors ----------------------------------------------------------
// Each motor is wired to TWO pins on its DRV8871 driver board.
// Put power on pin A and the motor spins one way, put it on pin B and it
// spins the other way. If one motor runs backwards on your vehicle, swap
// that motor's two pin numbers here. All wires to A are red and all wires to B are black.
const int FRONT_LEFT_A  = 12, FRONT_LEFT_B  = 13;
const int REAR_LEFT_A   = 18, REAR_LEFT_B   = 19;
const int FRONT_RIGHT_A = 22, FRONT_RIGHT_B = 23;
const int REAR_RIGHT_A  = 16, REAR_RIGHT_B  = 17;

const int MOTOR_PWM_FREQ = 20000;  // 20 kHz is above human hearing, so no motor whine
const int MOTOR_PWM_BITS = 8;      // 8 bits of resolution means speed values 0..255
const int MOTOR_MAX      = 255;    // Full speed. This is as fast as the motors go.
const int MOTOR_MIN      = 60;     // Slowest speed that can still turn a wheel

// Steering strength. Driving forwards and backwards always gets the full
// MOTOR_MAX, but turning is held to half of that by default, which makes the
// vehicle much easier to aim. Click the left stick to switch between the two.
const int TURN_MAX_NORMAL = MOTOR_MAX / 2;   // Gentle steering, and the default
const int TURN_MAX_SHARP  = MOTOR_MAX;       // Full power turns, spins on the spot

// --- Servos ----------------------------------------------------------
// A hobby servo wants one pulse every 20 ms. The LENGTH of that pulse sets
// the angle: 1000 us = 0 degrees, 1500 us = 90 degrees, 2000 us = 180 degrees.
const int SERVO1_PIN     = 25;
const int SERVO2_PIN     = 26;
const int SERVO_PWM_FREQ = 50;     // 50 pulses per second is one every 20 ms
const int SERVO_PWM_BITS = 16;     // Plenty of resolution, so the angle is smooth
const int SERVO_MIN_US   = 1000;
const int SERVO_MID_US   = 1500;
const int SERVO_MAX_US   = 2000;

// --- Thumbsticks -----------------------------------------------------
// A PS3 stick reports -128 to +127 on each axis. Pushing UP gives a NEGATIVE
// number, which is why "forward" flips the sign further down in loop().
const int STICK_MAX      = 127;
const int STICK_DEADZONE = 20;   // Ignore small values so a worn stick cannot creep

// --- KITT scanner ----------------------------------------------------
const int SCANNER_STEP_MS = 40;  // Time between moves. Smaller number = faster scanner.
const int SCANNER_TAIL    = 3;   // How many fading LEDs trail behind the bright one
const int SCANNER_RED     = 255; // Scanner colour. Try 0, 0, 255 for Cylon blue.
const int SCANNER_GREEN   = 0;
const int SCANNER_BLUE    = 0;

// ===================================================================
// STATE - variables that remember what the vehicle is doing right now
// ===================================================================

// Which lighting picture we show while driving.
enum LightPattern { LIGHTS_STOPPED, LIGHTS_FORWARD, LIGHTS_REVERSE, LIGHTS_LEFT, LIGHTS_RIGHT };

bool         lightsOn       = true;             // SQUARE toggles this
int          headlightLevel = 80;               // D-pad up/down changes this
LightPattern lightPattern   = LIGHTS_STOPPED;
bool         lightsChanged  = true;             // true means the strip needs redrawing

bool scannerOn       = false;   // TRIANGLE toggles this
int  scannerPosition = 0;       // Which front LED the bright dot is sitting on
int  scannerStep     = 1;       // +1 while moving right, -1 while moving left
unsigned long scannerLastMove = 0;

bool wasConnected    = false;   // Used to notice the moment a controller connects
bool triangleWasDown = false;   // Used to notice the moment a button is pressed
bool squareWasDown   = false;
bool stickClickWasDown = false;

int turnMax = TURN_MAX_NORMAL;  // Clicking the left stick swaps this over

// ===================================================================
// SMALL HELPER FUNCTIONS
// ===================================================================

/*
  Returns true only on the FIRST time round loop() that a button is held down.
  Without this a single press would count hundreds of times, because loop()
  runs thousands of times per second.
*/
bool justPressed(bool isDown, bool &wasDown) {
  bool isNewPress = isDown && !wasDown;
  wasDown = isDown;
  return isNewPress;
}

/*
  Turns a thumbstick reading into a motor speed.
  Inside the deadzone the answer is 0. Outside it, the rest of the stick travel
  is stretched across MOTOR_MIN..maxSpeed, so the wheels actually move as soon
  as you leave the deadzone instead of just buzzing.

  maxSpeed is handed in rather than always being MOTOR_MAX, because driving and
  steering are allowed different limits. Note that the bottom of the range stays
  at MOTOR_MIN either way, so even a gentle turn still has enough power to break
  the wheels loose.
*/
int stickToSpeed(int stickValue, int maxSpeed) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return 0;
  }
  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, maxSpeed);
  size = constrain(size, MOTOR_MIN, maxSpeed);
  return (stickValue > 0) ? size : -size;
}

/*
  Turns a thumbstick reading into a servo pulse length in microseconds.
  A centred stick gives a centred servo.
*/
int stickToServoMicroseconds(int stickValue) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return SERVO_MID_US;
  }
  int value = constrain(stickValue, -STICK_MAX, STICK_MAX);
  return map(value, -STICK_MAX, STICK_MAX, SERVO_MIN_US, SERVO_MAX_US);
}

// ===================================================================
// MOTOR FUNCTIONS
// ===================================================================

/*
  Drives one motor. Speed runs from -255 (full reverse) to +255 (full forward).
  Zero puts both pins low, which lets the motor coast to a stop.
*/
void setMotor(int pinA, int pinB, int speed) {
  speed = constrain(speed, -MOTOR_MAX, MOTOR_MAX);
  if (speed >= 0) {
    ledcWrite(pinA, speed);
    ledcWrite(pinB, 0);
  } else {
    ledcWrite(pinA, 0);
    ledcWrite(pinB, -speed);   // -speed turns the negative number positive
  }
}

/*
  Drives all four motors. This is "tank drive": the two wheels on the left get
  one speed and the two on the right get another. Same speed on both sides is a
  straight line, opposite speeds spin the vehicle on the spot.
*/
void drive(int leftSpeed, int rightSpeed) {
  setMotor(FRONT_LEFT_A,  FRONT_LEFT_B,  leftSpeed);
  setMotor(REAR_LEFT_A,   REAR_LEFT_B,   leftSpeed);
  setMotor(FRONT_RIGHT_A, FRONT_RIGHT_B, rightSpeed);
  setMotor(REAR_RIGHT_A,  REAR_RIGHT_B,  rightSpeed);
}

// ===================================================================
// SERVO FUNCTIONS
// ===================================================================

/*
  Sends one servo pulse of the requested length.

  The ESP32 does not think in microseconds, it thinks in "duty" counts. With 16
  bits, one whole 20 ms cycle is 65536 counts, so:
        counts = microseconds * 65536 / 20000
  We use "long" for the maths because 1500 * 65536 is far too big to fit in an int.
*/
void writeServo(int pin, int microseconds) {
  microseconds = constrain(microseconds, SERVO_MIN_US, SERVO_MAX_US);
  long duty = (long)microseconds * 65536L / 20000L;
  ledcWrite(pin, duty);
}

// ===================================================================
// LIGHTING FUNCTIONS
// ===================================================================

/*
  Draws the normal driving lights: white at the front, red at the back, amber
  on whichever side we are turning towards, and bright white at the back when
  reversing.
*/
void showDrivingLights() {
  strip.clear();   // Start from all-off, then paint on what we want

  if (lightsOn) {
    uint32_t headlight    = strip.Color(headlightLevel, headlightLevel, headlightLevel);
    uint32_t tailLight    = strip.Color(60, 0, 0);
    uint32_t brakeLight   = strip.Color(180, 0, 0);
    uint32_t reverseLight = strip.Color(200, 200, 200);
    uint32_t amber        = strip.Color(255, 100, 0);

    // Headlights across the whole front
    for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {
      strip.setPixelColor(i, headlight);
    }

    // Rear lights: brighter red when stopped (brake lights), white in reverse
    uint32_t rearColour = tailLight;
    if (lightPattern == LIGHTS_STOPPED) rearColour = brakeLight;
    if (lightPattern == LIGHTS_REVERSE) rearColour = reverseLight;
    for (int i = REAR_FIRST; i <= REAR_LAST; i++) {
      strip.setPixelColor(i, rearColour);
    }

    // Turn signals. Remember the loop: LEDs 0-7 and 24-31 are the LEFT side,
    // LEDs 8-15 and 16-23 are the RIGHT side.
    if (lightPattern == LIGHTS_LEFT) {
      for (int i = 0;  i <= 7;  i++) strip.setPixelColor(i, amber);
      for (int i = 24; i <= 31; i++) strip.setPixelColor(i, amber);
    } else if (lightPattern == LIGHTS_RIGHT) {
      for (int i = 8;  i <= 15; i++) strip.setPixelColor(i, amber);
      for (int i = 16; i <= 23; i++) strip.setPixelColor(i, amber);
    }
  }

  strip.show();   // Nothing appears on the strip until show() is called
}

/*
  Slow green blink while we wait for a controller to connect.

  Notice there is no delay() in here. We check the clock instead, so the rest of
  the program keeps running. millis() counts the milliseconds since the ESP32
  was switched on.
*/
void showWaitingLights() {
  static unsigned long lastBlink = 0;
  static bool blinkOn = false;

  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    blinkOn = !blinkOn;

    strip.fill(blinkOn ? strip.Color(0, 60, 0) : strip.Color(0, 0, 0));
    strip.show();
  }
}

/*
  The KITT scanner.

  One bright dot slides from one side of the vehicle to the other and back, with
  a short fading tail behind it, just like the scanner on KITT in Knight Rider.
  The front dot is drawn at "scannerPosition", and the matching rear dot is drawn
  at "REAR_LAST - scannerPosition" so the two stay side by side.

  Again there is no delay(): we only move the dot once every SCANNER_STEP_MS, and
  the vehicle keeps driving in between.
*/
void updateScanner() {
  if (millis() - scannerLastMove < SCANNER_STEP_MS) {
    return;   // Not time to move yet, come back next time round loop()
  }
  scannerLastMove = millis();

  strip.clear();

  // Draw the bright dot (tail = 0), then each dimmer tail LED behind it.
  for (int tail = 0; tail <= SCANNER_TAIL; tail++) {
    int position = scannerPosition - (scannerStep * tail);
    if (position < FRONT_FIRST || position > FRONT_LAST) {
      continue;   // This part of the tail has slid off the end, so skip it
    }

    // ">> tail" halves the brightness for each step back: 255, 127, 63, 31
    uint32_t colour = strip.Color(SCANNER_RED   >> tail,
                                  SCANNER_GREEN >> tail,
                                  SCANNER_BLUE  >> tail);

    strip.setPixelColor(position, colour);              // Front of the vehicle
    strip.setPixelColor(REAR_LAST - position, colour);  // Matching LED at the rear
  }

  strip.show();

  // Move the dot one place, and bounce when it reaches either end.
  scannerPosition += scannerStep;
  if (scannerPosition >= FRONT_LAST) {
    scannerPosition = FRONT_LAST;
    scannerStep = -1;
  } else if (scannerPosition <= FRONT_FIRST) {
    scannerPosition = FRONT_FIRST;
    scannerStep = 1;
  }
}

// ===================================================================
// SETUP - runs once, when the ESP32 powers up
// ===================================================================

void setup() {
  Serial.begin(115200);

  // --- LEDs ---
  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();

  // --- Motors ---
  // ledcAttach() sets up hardware PWM on a pin: (pin, frequency, resolution).
  // After that we can call ledcWrite(pin, 0..255) to set the speed.
  ledcAttach(FRONT_LEFT_A,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_LEFT_B,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_LEFT_A,   MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_LEFT_B,   MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_RIGHT_A, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_RIGHT_B, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_RIGHT_A,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_RIGHT_B,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  drive(0, 0);   // Make sure nothing is moving

  // --- Servos ---
  ledcAttach(SERVO1_PIN, SERVO_PWM_FREQ, SERVO_PWM_BITS);
  ledcAttach(SERVO2_PIN, SERVO_PWM_FREQ, SERVO_PWM_BITS);
  writeServo(SERVO1_PIN, SERVO_MID_US);
  writeServo(SERVO2_PIN, SERVO_MID_US);

  // --- Startup light show ---
  // One scanner sweep, so you can see that every LED is working.
  // delay() is fine HERE because nothing else needs to happen yet. Once the
  // vehicle is driving we never use delay(), because it freezes everything.
  for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.setPixelColor(REAR_LAST - i, strip.Color(255, 0, 0));
    strip.show();
    delay(30);
  }
  strip.clear();
  strip.show();

  // --- Bluetooth ---
  Ps3.begin(PS3_MAC_ADDRESS);
  Serial.println("Pathfinder ready. Waiting for the PS3 controller...");
}

// ===================================================================
// LOOP - runs over and over, thousands of times per second
// ===================================================================

void loop() {

  // ---- No controller? Stop everything and wait. ----
  if (!Ps3.isConnected()) {
    if (wasConnected) {
      Serial.println("Controller disconnected. Motors stopped.");
      wasConnected = false;
    }
    drive(0, 0);           // Safety first: never drive without a controller
    showWaitingLights();
    return;                // Skip the rest of loop() and start it again
  }

  // ---- A controller has just connected ----
  if (!wasConnected) {
    wasConnected = true;
    lightsChanged = true;  // Make sure the driving lights get drawn
    Ps3.setPlayer(1);      // Light up player LED 1 on the controller
    Serial.println("Controller connected.");
  }

  // ---- Read the buttons ----
  if (justPressed(Ps3.data.button.triangle, triangleWasDown)) {
    scannerOn = !scannerOn;
    lightsChanged = true;                     // Redraw normal lights when it goes off
    Serial.println(scannerOn ? "Scanner ON" : "Scanner OFF");
  }

  if (justPressed(Ps3.data.button.square, squareWasDown)) {
    lightsOn = !lightsOn;
    lightsChanged = true;
    Serial.println(lightsOn ? "Lights ON" : "Lights OFF");
  }

  // Clicking the left stick swaps between gentle and sharp steering.
  if (justPressed(Ps3.data.button.l3, stickClickWasDown)) {
    turnMax = (turnMax == TURN_MAX_SHARP) ? TURN_MAX_NORMAL : TURN_MAX_SHARP;
    Serial.println(turnMax == TURN_MAX_SHARP ? "Steering: SHARP" : "Steering: NORMAL");
  }

  if (Ps3.data.button.up && headlightLevel != 255) {
    headlightLevel = 255;
    lightsChanged = true;
  }
  if (Ps3.data.button.down && headlightLevel != 80) {
    headlightLevel = 80;
    lightsChanged = true;
  }

  // ---- Read the left stick and drive ----
  int leftStickX = Ps3.data.analog.stick.lx;
  int leftStickY = Ps3.data.analog.stick.ly;

  // The stick gives a negative number when pushed up, so flip the sign to get a
  // "forward" number that is positive when we want to go forward.
  // Driving gets full power; steering gets whatever turnMax is set to.
  int forward = stickToSpeed(-leftStickY, MOTOR_MAX);
  int turn    = stickToSpeed(leftStickX, turnMax);

  // Mixing forward and turn together is what lets you steer while moving.
  int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);
  int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);
  drive(leftSpeed, rightSpeed);

  // ---- Work out which lighting picture matches what we are doing ----
  LightPattern newPattern = LIGHTS_STOPPED;
  if (forward > 0)      newPattern = LIGHTS_FORWARD;
  else if (forward < 0) newPattern = LIGHTS_REVERSE;
  else if (turn > 0)    newPattern = LIGHTS_RIGHT;
  else if (turn < 0)    newPattern = LIGHTS_LEFT;

  if (newPattern != lightPattern) {
    lightPattern = newPattern;
    lightsChanged = true;
  }

  // ---- Read the right stick and aim the servos ----
  int rightStickX = Ps3.data.analog.stick.rx;
  int rightStickY = Ps3.data.analog.stick.ry;
  writeServo(SERVO1_PIN, stickToServoMicroseconds(rightStickX));
  writeServo(SERVO2_PIN, stickToServoMicroseconds(rightStickY));

  // ---- Update the LEDs ----
  // The scanner takes over the whole strip while it is running. When it is off
  // we only redraw the driving lights if something actually changed, which keeps
  // the strip from flickering.
  if (scannerOn) {
    updateScanner();
  } else if (lightsChanged) {
    lightsChanged = false;
    showDrivingLights();
  }
}
