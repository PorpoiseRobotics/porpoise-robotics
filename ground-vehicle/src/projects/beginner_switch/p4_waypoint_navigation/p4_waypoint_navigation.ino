/*
  p4_waypoint_navigation.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track)
  TAKE IT FURTHER project - see Lesson 5, "projects that fit on this vehicle"

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives a LIST of moves instead of one hard-coded square.

  Each leg of the route is a turn followed by a straight run. Press the
  start button and the vehicle drives the whole list, then stops. Push
  the stick at any point and it gives up and hands control back.

  This is Lesson 2's maneuver, grown up: the same dead reckoning, but the
  route is data at the top of the file rather than code in the middle of
  it. Change where it goes without touching the program.

  This is l5c_drive_with_lights with one thing added. Everything else in
  the file you have already met. Open the two side by side and the
  difference is the project.

  SAFETY
  ------
  Wheels off the ground for the first upload, every time. Motors stop by
  themselves if the controller disconnects.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Library: "Adafruit NeoPixel" by Adafruit.
  Bluepad32 itself arrives with the board package.

  FILL IN MY_CONTROLLER FIRST
  ---------------------------
  Paste in the line that l3a_controller_check printed for your controller.

  CONTROLS
  --------
    Left stick          Drive. Up = forward, down = reverse, left/right = turn.
    LEFT face button    All lights on / off
    TOP face button     Start the route, or stop it
    Any stick push      Aborts a running route

    Most Switch pads mark the left face button Y, the top one
    X and the right one A.

  THE IDEA
  --------
  There is no new hardware here and no new sensor. What changes is where
  the route LIVES.

  l2c_maneuver_square hard-codes four drives and four turns in setup().
  Adding a fifth leg means writing more code. Here the route is a table,
  and the program is a state machine that walks it: turn, drive, next leg,
  stop. A twenty-leg route is twenty lines of data and not one more line
  of program.

  It still cannot see. FEET_PER_SECOND and DEGREES_PER_SECOND are numbers
  YOU measured on YOUR vehicle, on the floor you are driving on, with the
  battery you have now. Change any of those and the route drifts. That is
  not a bug in the program - it is what dead reckoning is, and it is why
  every real vehicle eventually gets a sensor.

  Note that pushing the stick aborts. Anything that drives itself needs a
  way for a person to take it back, and it should be the control they
  already have in their hand.

  WHAT TO TRY
  -----------
  1. Run the route as it is. Mark where the vehicle stops with tape, then
     run it again from the same spot. How far apart are the two marks?
  2. Measure FEET_PER_SECOND and DEGREES_PER_SECOND properly, on the
     floor you are on, at CRUISE_SPEED and SPIN_SPEED. Put your numbers
     in. Does the drift get smaller?
  3. Run it on carpet and then on tile. Same numbers, different route.
  4. Add legs until the route is a pentagon. Only the table changes.
  5. Run it on a nearly flat battery. What happens, and why?
  6. Make the vehicle drive the route BACKWARDS at the end, and see
     whether it gets home.
  7. Fold this into your copy of the full program.
*/

#include <Bluepad32.h>
#include <uni.h>
#include <Adafruit_NeoPixel.h>

// --- The one controller this vehicle will talk to --------------------
const uint8_t MY_CONTROLLER[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// --- LEDs ---
const int LED_PIN        = 5;
const int LED_COUNT      = 32;
const int LED_BRIGHTNESS = 120;

const int FRONT_FIRST = 0;    // Front left
const int FRONT_LAST  = 15;   // Front right
const int REAR_FIRST  = 16;   // Rear right
const int REAR_LAST   = 31;   // Rear left

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- Motors ---
const int FRONT_LEFT_PIN_A  = 12, FRONT_LEFT_CH_A  = 0;
const int FRONT_LEFT_PIN_B  = 13, FRONT_LEFT_CH_B  = 1;
const int REAR_LEFT_PIN_A   = 18, REAR_LEFT_CH_A   = 2;
const int REAR_LEFT_PIN_B   = 19, REAR_LEFT_CH_B   = 3;
const int FRONT_RIGHT_PIN_A = 22, FRONT_RIGHT_CH_A = 4;
const int FRONT_RIGHT_PIN_B = 23, FRONT_RIGHT_CH_B = 5;
const int REAR_RIGHT_PIN_A  = 16, REAR_RIGHT_CH_A  = 6;
const int REAR_RIGHT_PIN_B  = 17, REAR_RIGHT_CH_B  = 7;

const int MOTOR_PWM_FREQ = 20000;
const int MOTOR_PWM_BITS = 8;
const int MOTOR_MAX      = 255;
const int MOTOR_MIN      = 0;

const int turnMax = (MOTOR_MAX * 3) / 4;

// --- Thumbsticks ---
const int STICK_MAX      = 511;
const int STICK_DEADZONE = 60;


// --- The route -------------------------------------------------------
// One line per leg: how far to turn first, then how far to run straight.
// Positive degrees are a turn to the RIGHT, negative to the left, and 0
// means carry straight on. Edit this table; the program below does not
// change.
struct Leg {
  int   turnDegrees;
  float feet;
};

const Leg ROUTE[] = {
  {   0, 10.0f },   // Straight out
  {  90,  6.0f },   // Right, then across
  {  90, 10.0f },   // Right, then back
  {  90,  6.0f },   // Right, then across again
  {  90,  0.0f },   // Right, to finish pointing the way we started
};
const int ROUTE_LEGS = sizeof(ROUTE) / sizeof(ROUTE[0]);

// --- Your vehicle's numbers ------------------------------------------
// These are the two you measured in Lesson 2, and they are YOURS. They
// change with the floor, with the battery, and with the vehicle. Measure
// them again before you trust a long route.
const int   CRUISE_SPEED       = 150;
const int   SPIN_SPEED         = 130;
const float FEET_PER_SECOND    = 3.34f;    // At CRUISE_SPEED
const float DEGREES_PER_SECOND = 180.0f;   // Spinning on the spot at SPIN_SPEED

// --- State ---
enum LightPattern { LIGHTS_STOPPED, LIGHTS_FORWARD, LIGHTS_REVERSE, LIGHTS_LEFT, LIGHTS_RIGHT };

bool         lightsOn       = true;
int          headlightLevel = 80;
LightPattern lightPattern   = LIGHTS_STOPPED;
bool         lightsChanged  = true;

bool lightsButtonWasDown = false;

ControllerPtr myController = nullptr;
bool addressIsSet = false;
bool wasConnected = false;


enum RunState { RUN_IDLE, RUN_TURNING, RUN_DRIVING, RUN_DONE };

RunState      runState     = RUN_IDLE;
int           legIndex     = 0;
unsigned long legStartedAt = 0;
unsigned long legDuration  = 0;
bool          startButtonWasDown = false;

bool justPressed(bool isDown, bool &wasDown) {
  bool isNewPress = isDown && !wasDown;
  wasDown = isDown;
  return isNewPress;
}

bool isMyController(const uint8_t *address) {
  for (int i = 0; i < 6; i++) {
    if (address[i] != MY_CONTROLLER[i]) {
      return false;
    }
  }
  return true;
}

void onConnectedController(ControllerPtr controller) {
  if (myController != nullptr || !isMyController(controller->getProperties().btaddr)) {
    controller->disconnect();
    return;
  }
  myController = controller;
  Serial.print("Controller connected: ");
  Serial.println(controller->getModelName());
}

void onDisconnectedController(ControllerPtr controller) {
  if (myController == controller) {
    myController = nullptr;
  }
}

int stickToSpeed(int stickValue, int maxSpeed) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return 0;
  }
  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, maxSpeed);
  size = constrain(size, MOTOR_MIN, maxSpeed);
  return (stickValue > 0) ? size : -size;
}

void attachMotorPwm(int pin, int channel) {
  ledcSetup(channel, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttachPin(pin, channel);
}

void setMotor(int channelA, int channelB, int speed) {
  speed = constrain(speed, -MOTOR_MAX, MOTOR_MAX);
  if (speed >= 0) {
    ledcWrite(channelA, speed);
    ledcWrite(channelB, 0);
  } else {
    ledcWrite(channelA, 0);
    ledcWrite(channelB, -speed);
  }
}

void drive(int leftSpeed, int rightSpeed) {
  setMotor(FRONT_LEFT_CH_A,  FRONT_LEFT_CH_B,  leftSpeed);
  setMotor(REAR_LEFT_CH_A,   REAR_LEFT_CH_B,   leftSpeed);
  setMotor(FRONT_RIGHT_CH_A, FRONT_RIGHT_CH_B, rightSpeed);
  setMotor(REAR_RIGHT_CH_A,  REAR_RIGHT_CH_B,  rightSpeed);
}

/*
  Draws the driving lights: white at the front, red at the back, brighter red
  when stopped, white at the back in reverse, and amber down whichever side we
  are turning toward.
*/
void showDrivingLights() {
  strip.clear();

  if (lightsOn) {
    uint32_t headlight    = strip.Color(headlightLevel, headlightLevel, headlightLevel);
    uint32_t tailLight    = strip.Color(60, 0, 0);
    uint32_t brakeLight   = strip.Color(180, 0, 0);
    uint32_t reverseLight = strip.Color(200, 200, 200);
    uint32_t amber        = strip.Color(255, 100, 0);

    for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {
      strip.setPixelColor(i, headlight);
    }

    uint32_t rearColor = tailLight;
    if (lightPattern == LIGHTS_STOPPED) rearColor = brakeLight;
    if (lightPattern == LIGHTS_REVERSE) rearColor = reverseLight;
    for (int i = REAR_FIRST; i <= REAR_LAST; i++) {
      strip.setPixelColor(i, rearColor);
    }

    // Remember the loop: 0-7 and 24-31 are LEFT, 8-15 and 16-23 are RIGHT.
    if (lightPattern == LIGHTS_LEFT) {
      for (int i = 0;  i <= 7;  i++) strip.setPixelColor(i, amber);
      for (int i = 24; i <= 31; i++) strip.setPixelColor(i, amber);
    } else if (lightPattern == LIGHTS_RIGHT) {
      for (int i = 8;  i <= 15; i++) strip.setPixelColor(i, amber);
      for (int i = 16; i <= 23; i++) strip.setPixelColor(i, amber);
    }
  }

  strip.show();
}

/*
  Slow green blink while we wait for a controller. No delay() in here - we
  check the clock instead, so the rest of the program keeps running.
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
  How long a move should take, from the numbers you measured.

  Distance over speed, and degrees over degrees per second. That is the whole
  of dead reckoning: the vehicle works out how long to drive and then never
  finds out where it actually went.
*/
unsigned long millisForFeet(float feet) {
  return (unsigned long)((feet / FEET_PER_SECOND) * 1000.0f);
}

unsigned long millisForDegrees(int degrees) {
  return (unsigned long)((abs(degrees) / DEGREES_PER_SECOND) * 1000.0f);
}

void stopRoute(const char *why) {
  runState = RUN_IDLE;
  drive(0, 0);
  lightsChanged = true;
  Serial.println(why);
}

/*
  Starts one leg: the turn first, then the straight run.

  A leg with no turn goes straight to driving. A leg past the end of the table
  finishes the route.
*/
void beginLeg(int index) {
  legIndex = index;

  if (index >= ROUTE_LEGS) {
    runState = RUN_DONE;
    drive(0, 0);
    lightsChanged = true;
    Serial.println("Route finished. Measure the gap between where it stopped");
    Serial.println("and where it was supposed to stop. That gap is the drift.");
    return;
  }

  legStartedAt = millis();
  lightsChanged = true;

  if (ROUTE[index].turnDegrees != 0) {
    runState = RUN_TURNING;
    legDuration = millisForDegrees(ROUTE[index].turnDegrees);
  } else {
    runState = RUN_DRIVING;
    legDuration = millisForFeet(ROUTE[index].feet);
  }

  Serial.print("Leg ");
  Serial.print(index + 1);
  Serial.print(" of ");
  Serial.println(ROUTE_LEGS);
}

/*
  The turn is over, so run the straight; or the straight is over, so take the
  next leg.
*/
void advanceRoute() {
  if (runState == RUN_TURNING) {
    runState = RUN_DRIVING;
    legStartedAt = millis();
    legDuration = millisForFeet(ROUTE[legIndex].feet);
    lightsChanged = true;
  } else {
    beginLeg(legIndex + 1);
  }
}

/*
  What the vehicle is doing, in one color: amber while it turns, green while
  it runs, blue when the route is finished.
*/
void showRouteLights() {
  uint32_t color = strip.Color(0, 0, 180);
  if (runState == RUN_TURNING)      color = strip.Color(255, 100, 0);
  else if (runState == RUN_DRIVING) color = strip.Color(0, 180, 0);
  strip.fill(color);
  strip.show();
}

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  strip.clear();
  strip.show();

  attachMotorPwm(FRONT_LEFT_PIN_A,  FRONT_LEFT_CH_A);
  attachMotorPwm(FRONT_LEFT_PIN_B,  FRONT_LEFT_CH_B);
  attachMotorPwm(REAR_LEFT_PIN_A,   REAR_LEFT_CH_A);
  attachMotorPwm(REAR_LEFT_PIN_B,   REAR_LEFT_CH_B);
  attachMotorPwm(FRONT_RIGHT_PIN_A, FRONT_RIGHT_CH_A);
  attachMotorPwm(FRONT_RIGHT_PIN_B, FRONT_RIGHT_CH_B);
  attachMotorPwm(REAR_RIGHT_PIN_A,  REAR_RIGHT_CH_A);
  attachMotorPwm(REAR_RIGHT_PIN_B,  REAR_RIGHT_CH_B);
  drive(0, 0);

  for (int i = 0; i < 6; i++) {
    if (MY_CONTROLLER[i] != 0x00) {
      addressIsSet = true;
    }
  }

  if (!addressIsSet) {
    Serial.println("MY_CONTROLLER has not been filled in, so this vehicle will");
    Serial.println("not drive. Run l3a_controller_check and paste the line it");
    Serial.println("prints into the top of this program.");
    return;
  }

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);

  bd_addr_t allowed;
  memcpy(allowed, MY_CONTROLLER, 6);
  uni_bt_allowlist_remove_all();
  uni_bt_allowlist_add_addr(allowed);
  uni_bt_allowlist_set_enabled(true);
  BP32.enableNewBluetoothConnections(true);

  Serial.println("Drive with lights. Waiting for the controller...");
}

void loop() {
  // Nobody told this vehicle which controller is its own, so there is nothing
  // safe to do. Blink and wait for somebody to fix the program.
  if (!addressIsSet) {
    showWaitingLights();
    return;
  }

  BP32.update();

  // ---- No controller? Stop everything and wait. ----
  if (myController == nullptr || !myController->isConnected()) {
    if (wasConnected) {
      Serial.println("Controller disconnected. Motors stopped.");
      wasConnected = false;
    }
    drive(0, 0);
    showWaitingLights();
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    lightsChanged = true;
    myController->setPlayerLEDs(0x01);
  }

  // ---- Buttons ----

  if (justPressed(myController->y(), startButtonWasDown)) {
    if (runState == RUN_TURNING || runState == RUN_DRIVING) {
      stopRoute("Stopped.");
    } else {
      Serial.println("Running the route. Push the stick to abort.");
      beginLeg(0);
    }
  }

  if (justPressed(myController->x(), lightsButtonWasDown)) {
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

  // ---- Drive ----
  int leftStickX = myController->axisX();
  int leftStickY = myController->axisY();

  int forward = stickToSpeed(-leftStickY, MOTOR_MAX);
  int turn    = stickToSpeed(leftStickX, turnMax);

  int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);
  int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);
  // ---- While a route is running, it drives - not the stick ----
  if (runState == RUN_TURNING || runState == RUN_DRIVING) {
    if (forward != 0 || turn != 0) {
      // Anything that drives itself needs a way for a person to take it back,
      // and it should be the control already in their hand.
      stopRoute("Stick pushed. Route abandoned, you have it.");
    } else if (millis() - legStartedAt >= legDuration) {
      advanceRoute();
    } else if (runState == RUN_TURNING) {
      int side = (ROUTE[legIndex].turnDegrees > 0) ? 1 : -1;
      drive(SPIN_SPEED * side, -SPIN_SPEED * side);
    } else {
      drive(CRUISE_SPEED, CRUISE_SPEED);
    }
  } else {
    drive(leftSpeed, rightSpeed);
  }

  // ---- Which lighting picture matches what we are doing? ----
  LightPattern newPattern = LIGHTS_STOPPED;
  if (forward > 0)      newPattern = LIGHTS_FORWARD;
  else if (forward < 0) newPattern = LIGHTS_REVERSE;
  else if (turn > 0)    newPattern = LIGHTS_RIGHT;
  else if (turn < 0)    newPattern = LIGHTS_LEFT;

  if (newPattern != lightPattern) {
    lightPattern = newPattern;
    lightsChanged = true;
  }

  // ---- Redraw ----
  if (lightsChanged) {
    lightsChanged = false;
    if (runState == RUN_IDLE) {
      showDrivingLights();
    } else {
      showRouteLights();
    }
  }
}
