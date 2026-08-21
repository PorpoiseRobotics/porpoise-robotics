/*
  pathfinder_nintendoswitch.ino
  Porpoise Robotics - Pathfinder vehicle, beginner program (Nintendo Switch controller)

  WHAT THIS PROGRAM DOES
  ----------------------
  The same as pathfinder_ps3.ino, but for a wireless Nintendo Switch style
  controller: drives the Pathfinder, aims two servos with the right thumbstick,
  and runs the 32 LEDs as headlights, tail lights, turn signals, and a "KITT"
  scanner (like Knight Rider, or a Cylon in Battlestar Galactica).

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0.
       File > Preferences > "Additional boards manager URLs", add this line:
         https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
       Then Tools > Board > Boards Manager, search "bluepad32", and install it.
       Finally pick Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".

       WATCH OUT: you now have TWO "ESP32 Dev Module" entries in the board list,
       one under "esp32" and one under "esp32_bluepad32". This program only
       compiles under the esp32_bluepad32 one. pathfinder_ps3.ino only compiles
       under the other one. If you get a pile of errors, check this first.

  2. Library (Tools > Manage Libraries):
       - "Adafruit NeoPixel" by Adafruit
       The Bluepad32 library itself arrives with the board package, so there is
       nothing else to install.

  LOCKING ONE CONTROLLER TO THIS VEHICLE
  --------------------------------------
  In a classroom there are many vehicles and many controllers switched on at
  once, so each vehicle has to ignore every controller except its own. The PS3
  program does this by pairing the controller to one Bluetooth address. Here we
  do the mirror image: the VEHICLE is told the address of the one controller it
  is allowed to talk to, and Bluepad32 refuses every other controller before the
  connection is even accepted.

  Step 1 - find out your controller's address (do this once):
      Leave MY_CONTROLLER below as all zeros and upload. The vehicle starts in
      PAIRING MODE and its LEDs blink BLUE. Put your controller into pairing
      mode (hold its small round SYNC button until its lights run back and
      forth) and wait. When it connects, open Tools > Serial Monitor at 115200
      baud and you will see a line like this:

          const uint8_t MY_CONTROLLER[6] = { 0x98, 0xB6, 0xE9, 0x11, 0x22, 0x33 };

  Step 2 - lock the vehicle to that controller:
      Copy that whole line over the MY_CONTROLLER line below and upload again.
      The LEDs now blink GREEN while waiting, and this vehicle will only ever
      accept that one controller. Write the address on a sticker on both the
      vehicle and the controller so the pair stays together.

  CONTROLS
  --------
    Left stick        Drive. Up = forward, down = reverse, left/right = turn.
    Right stick X     Servo 1 (pin 25)
    Right stick Y     Servo 2 (pin 26)
    TOP face button   KITT scanner on / off   (marked X on most Switch pads)
    LEFT face button  All lights on / off     (marked Y on most Switch pads)
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

  Porpoise Robotics
*/

#include <Bluepad32.h>
#include <uni.h>                // Lets us use the Bluetooth "allowlist"
#include <Adafruit_NeoPixel.h>

// ===================================================================
// SETTINGS - change these numbers to change how the vehicle behaves
// ===================================================================

// --- The one controller this vehicle will talk to --------------------
// All zeros means "pairing mode": see the instructions at the top of the file.
const uint8_t MY_CONTROLLER[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// --- LED strip -------------------------------------------------------
const int LED_PIN        = 5;    // Data wire for the LED strip
const int LED_COUNT      = 32;   // Four bars of 8 LEDs
const int LED_BRIGHTNESS = 120;  // Master brightness, 0 (off) to 255 (blinding)

// LED numbers for each part of the loop. Naming them keeps the code readable.
const int FRONT_FIRST = 0;
const int FRONT_LAST  = 15;
const int REAR_FIRST  = 16;
const int REAR_LAST   = 31;

// The strip object. We talk to the LEDs through this.
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Motors ----------------------------------------------------------
// Each motor is wired to TWO pins on its DRV8871 driver board.
// Put power on pin A and the motor spins one way, put it on pin B and it
// spins the other way. If one motor runs backwards on your vehicle, swap
// that motor's two PIN numbers here (leave the channel numbers alone).
//
// PWM CHANNELS: this board package is built on an older ESP32 core than
// pathfinder_ps3.ino uses, and it does not let us talk to a pin directly.
// Instead the ESP32 has 16 numbered PWM "channels". We set a channel up, plug
// it into a pin, and from then on we write speeds to the CHANNEL number.
// Every channel number has to be different. Channels 0-7 are used by the eight
// motor pins here, and 8-9 by the servos further down.
const int FRONT_LEFT_PIN_A  = 12, FRONT_LEFT_CH_A  = 0;
const int FRONT_LEFT_PIN_B  = 13, FRONT_LEFT_CH_B  = 1;
const int REAR_LEFT_PIN_A   = 18, REAR_LEFT_CH_A   = 2;
const int REAR_LEFT_PIN_B   = 19, REAR_LEFT_CH_B   = 3;
const int FRONT_RIGHT_PIN_A = 22, FRONT_RIGHT_CH_A = 4;
const int FRONT_RIGHT_PIN_B = 23, FRONT_RIGHT_CH_B = 5;
const int REAR_RIGHT_PIN_A  = 16, REAR_RIGHT_CH_A  = 6;
const int REAR_RIGHT_PIN_B  = 17, REAR_RIGHT_CH_B  = 7;

const int MOTOR_PWM_FREQ = 20000;  // 20 kHz is above human hearing, so no motor whine
const int MOTOR_PWM_BITS = 8;      // 8 bits of resolution means speed values 0..255
const int MOTOR_MAX      = 255;    // Full speed
const int MOTOR_MIN      = 60;     // Slowest speed that can still turn a wheel

// --- Servos ----------------------------------------------------------
// A hobby servo wants one pulse every 20 ms. The LENGTH of that pulse sets
// the angle: 1000 us = 0 degrees, 1500 us = 90 degrees, 2000 us = 180 degrees.
const int SERVO1_PIN     = 25, SERVO1_CH = 8;
const int SERVO2_PIN     = 26, SERVO2_CH = 9;
const int SERVO_PWM_FREQ = 50;     // 50 pulses per second is one every 20 ms
const int SERVO_PWM_BITS = 16;     // Plenty of resolution, so the angle is smooth
const int SERVO_MIN_US   = 1000;
const int SERVO_MID_US   = 1500;
const int SERVO_MAX_US   = 2000;

// --- Thumbsticks -----------------------------------------------------
// Bluepad32 reports every stick axis as -512 to +511, whatever brand of
// controller you plug in. Pushing UP gives a NEGATIVE number, which is why
// "forward" flips the sign further down in loop().
const int STICK_MAX      = 511;
const int STICK_DEADZONE = 60;   // Ignore small values so a worn stick cannot creep

// --- KITT scanner ----------------------------------------------------
const int SCANNER_STEP_MS = 40;  // Time between moves. Smaller number = faster scanner.
const int SCANNER_TAIL    = 3;   // How many fading LEDs trail behind the bright one
const int SCANNER_RED     = 255; // Scanner colour. Try 0, 0, 255 for Cylon blue.
const int SCANNER_GREEN   = 0;
const int SCANNER_BLUE    = 0;

// --- Serial Monitor help ---------------------------------------------
// Third-party controllers do not always send the button codes you expect.
// While this is true, every button press prints its code, so you can find out
// which code your own controller sends. Set it to false for quieter output.
const bool SHOW_BUTTON_CODES = true;

// ===================================================================
// STATE - variables that remember what the vehicle is doing right now
// ===================================================================

// Which lighting picture we show while driving.
enum LightPattern { LIGHTS_STOPPED, LIGHTS_FORWARD, LIGHTS_REVERSE, LIGHTS_LEFT, LIGHTS_RIGHT };

bool         lightsOn       = true;             // Left face button toggles this
int          headlightLevel = 80;               // D-pad up/down changes this
LightPattern lightPattern   = LIGHTS_STOPPED;
bool         lightsChanged  = true;             // true means the strip needs redrawing

bool scannerOn       = false;   // Top face button toggles this
int  scannerPosition = 0;       // Which front LED the bright dot is sitting on
int  scannerStep     = 1;       // +1 while moving right, -1 while moving left
unsigned long scannerLastMove = 0;

// The controller we are driving with. nullptr means "nothing connected".
ControllerPtr myController = nullptr;

bool pairingMode  = false;      // true when MY_CONTROLLER is still all zeros
bool wasConnected = false;      // Used to notice the moment a controller connects

bool scannerButtonWasDown = false;   // Used to notice the moment a button is pressed
bool lightsButtonWasDown  = false;
uint16_t lastButtonCodes  = 0;

// ===================================================================
// BLUETOOTH
// ===================================================================

/*
  Prints a Bluetooth address as the line of code you would paste into the
  MY_CONTROLLER setting at the top of this file.
*/
void printControllerAddress(const uint8_t *address) {
  Serial.print("    const uint8_t MY_CONTROLLER[6] = { ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("0x%02X", address[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println(" };");
}

/*
  Is this the controller we are allowed to drive with?
  In pairing mode we have not chosen one yet, so the first to arrive wins.
*/
bool isMyController(const uint8_t *address) {
  if (pairingMode) {
    return true;
  }
  for (int i = 0; i < 6; i++) {
    if (address[i] != MY_CONTROLLER[i]) {
      return false;
    }
  }
  return true;
}

/*
  Bluepad32 calls this by itself whenever a controller connects. We never call
  it ourselves - it is a "callback", a function we hand over for someone else
  to call at the right moment.
*/
void onConnectedController(ControllerPtr controller) {
  ControllerProperties properties = controller->getProperties();

  // One vehicle, one controller. Turn away anything extra.
  if (myController != nullptr) {
    Serial.println("Another controller tried to connect. Refused.");
    controller->disconnect();
    return;
  }

  if (!isMyController(properties.btaddr)) {
    Serial.println("A controller that does not belong to this vehicle was refused.");
    controller->disconnect();
    return;
  }

  myController = controller;
  Serial.print("Controller connected: ");
  Serial.println(controller->getModelName());
  printControllerAddress(properties.btaddr);

  if (pairingMode) {
    Serial.println("PAIRING MODE: copy the line above into MY_CONTROLLER at the");
    Serial.println("top of this program, then upload again to lock this vehicle.");
  }
}

/*
  Bluepad32 calls this by itself whenever the controller disconnects.
*/
void onDisconnectedController(ControllerPtr controller) {
  if (myController == controller) {
    myController = nullptr;
    Serial.println("Controller disconnected.");
  }
}

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
  is stretched across MOTOR_MIN..MOTOR_MAX, so the wheels actually move as soon
  as you leave the deadzone instead of just buzzing.
*/
int stickToSpeed(int stickValue) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return 0;
  }
  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, MOTOR_MAX);
  size = constrain(size, MOTOR_MIN, MOTOR_MAX);
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
  Gets one PWM channel ready and connects it to one pin.
*/
void attachMotorPwm(int pin, int channel) {
  ledcSetup(channel, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttachPin(pin, channel);
}

/*
  Drives one motor. Speed runs from -255 (full reverse) to +255 (full forward).
  Zero puts both channels low, which lets the motor coast to a stop.
*/
void setMotor(int channelA, int channelB, int speed) {
  speed = constrain(speed, -MOTOR_MAX, MOTOR_MAX);
  if (speed >= 0) {
    ledcWrite(channelA, speed);
    ledcWrite(channelB, 0);
  } else {
    ledcWrite(channelA, 0);
    ledcWrite(channelB, -speed);   // -speed turns the negative number positive
  }
}

/*
  Drives all four motors. This is "tank drive": the two wheels on the left get
  one speed and the two on the right get another. Same speed on both sides is a
  straight line, opposite speeds spin the vehicle on the spot.
*/
void drive(int leftSpeed, int rightSpeed) {
  setMotor(FRONT_LEFT_CH_A,  FRONT_LEFT_CH_B,  leftSpeed);
  setMotor(REAR_LEFT_CH_A,   REAR_LEFT_CH_B,   leftSpeed);
  setMotor(FRONT_RIGHT_CH_A, FRONT_RIGHT_CH_B, rightSpeed);
  setMotor(REAR_RIGHT_CH_A,  REAR_RIGHT_CH_B,  rightSpeed);
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
void writeServo(int channel, int microseconds) {
  microseconds = constrain(microseconds, SERVO_MIN_US, SERVO_MAX_US);
  long duty = (long)microseconds * 65536L / 20000L;
  ledcWrite(channel, duty);
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
  Blinks slowly while we wait for a controller: BLUE in pairing mode, GREEN once
  the vehicle has been locked to its own controller.

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

    uint32_t colour = strip.Color(0, 0, 0);
    if (blinkOn) {
      colour = pairingMode ? strip.Color(0, 0, 80) : strip.Color(0, 60, 0);
    }
    strip.fill(colour);
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
  attachMotorPwm(FRONT_LEFT_PIN_A,  FRONT_LEFT_CH_A);
  attachMotorPwm(FRONT_LEFT_PIN_B,  FRONT_LEFT_CH_B);
  attachMotorPwm(REAR_LEFT_PIN_A,   REAR_LEFT_CH_A);
  attachMotorPwm(REAR_LEFT_PIN_B,   REAR_LEFT_CH_B);
  attachMotorPwm(FRONT_RIGHT_PIN_A, FRONT_RIGHT_CH_A);
  attachMotorPwm(FRONT_RIGHT_PIN_B, FRONT_RIGHT_CH_B);
  attachMotorPwm(REAR_RIGHT_PIN_A,  REAR_RIGHT_CH_A);
  attachMotorPwm(REAR_RIGHT_PIN_B,  REAR_RIGHT_CH_B);
  drive(0, 0);   // Make sure nothing is moving

  // --- Servos ---
  ledcSetup(SERVO1_CH, SERVO_PWM_FREQ, SERVO_PWM_BITS);
  ledcAttachPin(SERVO1_PIN, SERVO1_CH);
  ledcSetup(SERVO2_CH, SERVO_PWM_FREQ, SERVO_PWM_BITS);
  ledcAttachPin(SERVO2_PIN, SERVO2_CH);
  writeServo(SERVO1_CH, SERVO_MID_US);
  writeServo(SERVO2_CH, SERVO_MID_US);

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
  // Has somebody filled in MY_CONTROLLER yet? All zeros means "no".
  pairingMode = true;
  for (int i = 0; i < 6; i++) {
    if (MY_CONTROLLER[i] != 0x00) {
      pairingMode = false;
    }
  }

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);   // We only want gamepads, not virtual mice

  // The "allowlist" is Bluepad32's guest list. If an address is on the list it
  // may connect, and if it is not, the connection is turned away before it is
  // ever accepted. We clear the list and rebuild it on every start up, so the
  // sketch is always the single source of truth about who may drive.
  uni_bt_allowlist_remove_all();

  if (pairingMode) {
    uni_bt_allowlist_set_enabled(false);   // No guest list yet: anyone may knock
    BP32.forgetBluetoothKeys();            // Start pairing from a clean slate
    Serial.println();
    Serial.println("PAIRING MODE (the LEDs are blinking blue)");
    Serial.println("Hold the SYNC button on your controller until its lights run");
    Serial.println("back and forth, then wait for it to connect.");
  } else {
    bd_addr_t allowed;
    memcpy(allowed, MY_CONTROLLER, 6);
    uni_bt_allowlist_add_addr(allowed);
    uni_bt_allowlist_set_enabled(true);    // Now only that one controller gets in
    Serial.println();
    Serial.println("This vehicle is locked to one controller:");
    printControllerAddress(MY_CONTROLLER);
  }

  BP32.enableNewBluetoothConnections(true);
  Serial.println("Pathfinder ready. Waiting for the controller...");
}

// ===================================================================
// LOOP - runs over and over, thousands of times per second
// ===================================================================

void loop() {
  // Ask Bluepad32 for fresh controller data. This is also where Bluepad32 calls
  // onConnectedController() and onDisconnectedController() for us.
  BP32.update();

  // ---- No controller? Stop everything and wait. ----
  if (myController == nullptr || !myController->isConnected()) {
    if (wasConnected) {
      Serial.println("Motors stopped.");
      wasConnected = false;
    }
    drive(0, 0);           // Safety first: never drive without a controller
    showWaitingLights();
    return;                // Skip the rest of loop() and start it again
  }

  // ---- A controller has just connected ----
  if (!wasConnected) {
    wasConnected = true;
    lightsChanged = true;              // Make sure the driving lights get drawn
    myController->setPlayerLEDs(0x01); // Light up player LED 1 on the controller
  }

  // ---- Read the buttons ----
  // Bluepad32 names the four face buttons by POSITION, not by the letter
  // printed on them:  a() = bottom, b() = right, x() = left, y() = top.
  // Most Switch style pads print  bottom = B, right = A, left = Y, top = X.
  // So y() below is the button marked X on the controller, which sits in the
  // same place as TRIANGLE does on the PS3 pad.
  bool scannerButton = myController->y();   // Top face button
  bool lightsButton  = myController->x();   // Left face button

  if (SHOW_BUTTON_CODES && myController->buttons() != lastButtonCodes) {
    lastButtonCodes = myController->buttons();
    Serial.printf("Button code: 0x%04X\n", lastButtonCodes);
  }

  if (justPressed(scannerButton, scannerButtonWasDown)) {
    scannerOn = !scannerOn;
    lightsChanged = true;                     // Redraw normal lights when it goes off
    Serial.println(scannerOn ? "Scanner ON" : "Scanner OFF");
  }

  if (justPressed(lightsButton, lightsButtonWasDown)) {
    lightsOn = !lightsOn;
    lightsChanged = true;
    Serial.println(lightsOn ? "Lights ON" : "Lights OFF");
  }

  uint8_t dpad = myController->dpad();
  if ((dpad & DPAD_UP) && headlightLevel != 255) {
    headlightLevel = 255;
    lightsChanged = true;
  }
  if ((dpad & DPAD_DOWN) && headlightLevel != 80) {
    headlightLevel = 80;
    lightsChanged = true;
  }

  // ---- Read the left stick and drive ----
  int leftStickX = myController->axisX();
  int leftStickY = myController->axisY();

  // The stick gives a negative number when pushed up, so flip the sign to get a
  // "forward" number that is positive when we want to go forward.
  int forward = stickToSpeed(-leftStickY);
  int turn    = stickToSpeed(leftStickX);

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
  int rightStickX = myController->axisRX();
  int rightStickY = myController->axisRY();
  writeServo(SERVO1_CH, stickToServoMicroseconds(rightStickX));
  writeServo(SERVO2_CH, stickToServoMicroseconds(rightStickY));

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
