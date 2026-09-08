/*
  p1_low_battery_warning.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track)
  TAKE IT FURTHER project - see Lesson 5, "projects that fit on this vehicle"

  WHAT THIS PROGRAM DOES
  ----------------------
  Watches the battery while you drive, and says so before it is too late.

  Above 12 volts the vehicle behaves normally. Below 12 the whole strip
  flashes yellow. Below 10 it flashes red, twice as fast. A 4S lithium
  polymer pack that is run flat is damaged, and the vehicle notices the
  sag long before you notice the vehicle getting slower.

  This is l5c_drive_with_lights with one thing added. Everything else in
  the file you have already met. Open the two side by side and the
  difference is the project.

  WHAT YOU HAVE TO WIRE UP
  ------------------------
  One resistor divider, on the top plate breadboard.

      pack +  ---[ 100k ]---+---[ 20k ]---  GND
                            |
                         GPIO 34

  The ESP32 reads 0 to 3.3 volts and nothing else, so a 16.8 volt pack
  has to be divided down first. This is Lesson 2's series circuit: the
  same current flows through both resistors, so the voltage splits in
  proportion to them, and GPIO 34 sees

      Vpack x 20 / (100 + 20)  =  Vpack / 6

  which is 2.8 volts at a full pack. GPIO 34 is INPUT ONLY, which is all
  this needs.

  Get the two resistors the right way round. 20k to ground. If you put
  the 100k to ground instead, GPIO 34 sees 14 volts and the pin is gone.

  Nothing wired up yet? The program says so on the serial monitor and
  then drives normally, so it is safe to upload before you have built it.

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
    D-pad UP / DOWN     Headlights bright / dim

    Most Switch pads mark the left face button Y, the top one
    X and the right one A.

  THE IDEA
  --------
  Reading a battery is reading a voltage, and reading a voltage on a
  microcontroller means two things: get it into range with a divider, and
  average enough samples that one motor stalling does not trip the alarm.

  The reading is taken twice a second, not every pass of loop(). Nothing
  about a battery changes in a millisecond, and the ADC is slow enough to
  be worth not asking.

  The warning is drawn on its own millis() clock, on top of the driving
  lights, which is the same layering the full program uses for the
  scanner.

  WHAT TO TRY
  -----------
  1. Charge the pack, note the reading, then drive it down and watch the
     number fall on the serial monitor. Compare it with a multimeter.
  2. Raise BATTERY_LOW_V to just under what your pack reads right now,
     and watch the warning come on. Put it back afterwards.
  3. Drive hard and watch the reading SAG while the motors pull, then
     recover when you let go. That sag is why the average is over sixteen
     samples and not one.
  4. Make the vehicle refuse to drive forward at all below BATTERY_EMPTY_V,
     the way the collision project refuses. Should it? Argue both sides.
  5. Fold this into your copy of the full program.
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


// --- Battery monitor -------------------------------------------------
// See the wiring note at the top. GPIO 34 reads the pack through a
// divider, so the number here is a sixth of the real pack voltage until
// we multiply it back up.
const int   BATTERY_PIN     = 34;
const float DIVIDER_RATIO   = 6.0f;    // (100k + 20k) / 20k
const float BATTERY_LOW_V   = 12.0f;   // Flash yellow below this
const float BATTERY_EMPTY_V = 10.0f;   // Flash red below this, twice as fast

// A pack that reads under this is not a flat pack, it is a divider nobody
// has built yet. Say so once, and then leave the driver alone.
const float BATTERY_SENSE_MIN_V = 3.0f;

const unsigned long BATTERY_READ_MS = 500;

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


enum BatteryState { BATTERY_OK, BATTERY_LOW, BATTERY_EMPTY };

BatteryState  batteryState     = BATTERY_OK;
float         packVolts        = 0.0f;
bool          batterySenseWired = true;
unsigned long lastBatteryRead  = 0;
unsigned long lastBatteryBlink = 0;
bool          batteryBlinkOn   = false;

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
  The pack voltage, in volts, read through the divider.

  analogReadMilliVolts() applies the chip's own ADC calibration, so this is
  real millivolts at the PIN rather than a raw count. Multiplying by
  DIVIDER_RATIO gets back to volts at the PACK.

  Sixteen samples averaged, because a motor pulling current makes the pack sag
  and a single sample lands wherever it happens to land. Sixteen reads take
  well under a millisecond, and this only runs twice a second anyway.
*/
float readPackVolts() {
  long total = 0;
  for (int i = 0; i < 16; i++) {
    total += analogReadMilliVolts(BATTERY_PIN);
  }
  return (total / 16.0f) * DIVIDER_RATIO / 1000.0f;
}

/*
  The battery warning, which takes the whole strip.

  Yellow for low, red for empty, blinking either way - a steady color would be
  mistaken for a driving light, and this is not information you want anybody
  to have to notice.
*/
void showBatteryWarning() {
  uint32_t color = (batteryState == BATTERY_EMPTY) ? strip.Color(255, 0, 0)
                                                   : strip.Color(255, 160, 0);
  strip.fill(batteryBlinkOn ? color : strip.Color(0, 0, 0));
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

  analogReadResolution(12);
  packVolts = readPackVolts();
  batterySenseWired = (packVolts >= BATTERY_SENSE_MIN_V);

  if (batterySenseWired) {
    Serial.print("Pack at startup: ");
    Serial.print(packVolts, 2);
    Serial.println(" V");
  } else {
    Serial.println("GPIO 34 reads almost nothing, so the divider is not built");
    Serial.println("yet. Driving normally, with the battery warning switched");
    Serial.println("off. See the wiring note at the top of this file.");
  }

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
  drive(leftSpeed, rightSpeed);

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

  // ---- The battery, checked twice a second ----
  if (batterySenseWired && millis() - lastBatteryRead >= BATTERY_READ_MS) {
    lastBatteryRead = millis();
    packVolts = readPackVolts();

    BatteryState newState = BATTERY_OK;
    if (packVolts < BATTERY_EMPTY_V)    newState = BATTERY_EMPTY;
    else if (packVolts < BATTERY_LOW_V) newState = BATTERY_LOW;

    if (newState != batteryState) {
      batteryState = newState;
      lightsChanged = true;
      Serial.print("Battery ");
      Serial.print(packVolts, 2);
      if (newState == BATTERY_OK)        Serial.println(" V  -  OK");
      else if (newState == BATTERY_LOW)  Serial.println(" V  -  LOW, bring it in");
      else                               Serial.println(" V  -  EMPTY, stop now");
    }
  }

  // ---- Redraw. The battery warning overrides the driving lights. ----
  if (batteryState != BATTERY_OK) {
    unsigned long blinkEvery = (batteryState == BATTERY_EMPTY) ? 200 : 500;
    if (millis() - lastBatteryBlink >= blinkEvery) {
      lastBatteryBlink = millis();
      batteryBlinkOn = !batteryBlinkOn;
      showBatteryWarning();
    }
    // Leave the flag set, so the driving lights come straight back when a
    // fresh pack goes in.
    lightsChanged = true;
  } else if (lightsChanged) {
    lightsChanged = false;
    showDrivingLights();
  }
}
