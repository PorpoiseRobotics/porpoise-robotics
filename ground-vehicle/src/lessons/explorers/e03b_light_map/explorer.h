/*
  explorer.h  -  THE ENGINE ROOM
  Porpoise Robotics - Pathfinder Explorers course (ages 9 to 13)

  Hi, Explorer!

  This tab does the hard jobs so that YOUR program can stay short. It talks to
  the four motors, the 32 lights, and your controller. Your program just says
  what it wants:

      setLight(0, RED);        turn light number 0 red
      forward(1000);           drive forward for 1000 milliseconds (1 second)
      if (buttonA()) { }       do something while A is held down

  You never need to change this file. You ARE allowed to read it. Every line
  is ordinary Arduino code, and by the end of the course you will recognize a
  lot of it.

  ---------------------------------------------------------------------------
  EVERYTHING YOUR PROGRAM CAN USE
  ---------------------------------------------------------------------------
  Getting ready - put these in setup():
    startRover()                  Wake up the lights and motors. Always first.
    startController()             Connect to THIS vehicle's own controller.
    setSpeedLimit(percent)        The rover's top speed. 50 means half power.

  Lights - the numbers go from 0 to 31. Look at the light map.
    setLight(number, color)       One light
    setLights(first, last, color) A row of lights, like setLights(0, 7, BLUE)
    setAllLights(color)           All 32
    setFrontLights(color)         Numbers 0 to 15
    setBackLights(color)          Numbers 16 to 31
    setLeftLights(color)          The whole left side, front and back
    setRightLights(color)         The whole right side, front and back
    lightsOff()                   All 32 off
    behind(number)                The back light right behind a front light
    setBrightness(percent)        How bright everything is, 0 to 100

  Colors:
    RED  ORANGE  YELLOW  GREEN  CYAN  BLUE  PURPLE  PINK  WHITE  OFF
    mixColor(red, green, blue)    Mix your own. Each number is 0 to 255.
    rainbowColor(percent)         A color from around the rainbow, 0 to 100
    randomColor()                 Surprise!

  Moving - speeds are percent of top speed, from -100 to 100. Minus = backward.
    drive(leftSpeed, rightSpeed)  Set the left wheels and the right wheels
    stopMotors()                  Stop
    setDriveSpeed(percent)        How fast the autopilot moves below go
    forward(ms)   backward(ms)    Autopilot: drive for this many milliseconds
    spinLeft(ms)  spinRight(ms)   Autopilot: turn on the spot
    pause(ms)                     Autopilot: stop and wait
    countdown(seconds)            3... 2... 1... GO! on the lights

  Controller - call controllerReady() ONCE, at the very top of loop():
    controllerReady()             true when your controller is connected
    buttonA() buttonB() buttonX() buttonY()   true the WHOLE TIME it is held
    tappedA() tappedB() tappedX() tappedY()   true ONCE, the moment you press
    buttonL() buttonR() buttonZL() buttonZR() the shoulder buttons
    dpadUp() dpadDown() dpadLeft() dpadRight()
    leftStickX()  leftStickY()    -100 to 100. RIGHT and UP are plus.
    rightStickX() rightStickY()
    rumble(ms)                    Buzz the controller, if it can buzz

  Timing:
    blinkIsOn()                   true, false, true, false... every 0.4 seconds
    timeToPrint()                 true about 4 times a second, for the
                                  Serial Monitor

  Autopilot safety: while an autopilot move is running in a program that uses
  the controller, holding B stops the vehicle and cancels the rest of the
  moves. B is for Brake.

  ---------------------------------------------------------------------------
  FOR INSTRUCTORS
  ---------------------------------------------------------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0.
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Library: "Adafruit NeoPixel" by Adafruit.
  This is the Nintendo Switch track. The PWM calls are the channel-based ones
  that board package needs (ledcSetup / ledcAttachPin / ledcWrite on a
  channel), the same as pathfinder_nintendoswitch.

  ONE FILE, MANY COPIES. Arduino compiles a sketch from its own folder only, so
  every Explorers sketch folder carries its own copy of this file. The master
  copy is ground-vehicle/src/lessons/explorers/explorer.h. Edit that one, then
  run ground-vehicle/docs/courses/generator/sync_explorer_h.py to copy it into
  every sketch. The course build refuses to run while any copy differs.

  WHICH CONTROLLER. e00_claim_controller saves a controller's Bluetooth address
  in the vehicle's own flash memory (NVS, through the Preferences library).
  Uploading other programs does not erase it, so students never type an
  address. startController() reads it back on every boot and puts it on the
  Bluepad32 allowlist, so this vehicle refuses every other controller in the
  room - exactly what pathfinder_nintendoswitch does with MY_CONTROLLER.
    Lights blinking GREEN  = looking for its controller. Press a button on it.
    Lights blinking RED    = no controller has been claimed. Run e00.
  Tools > "Erase All Flash Before Sketch Upload: Enabled" wipes the claim.
  Leave it Disabled, which is the default.

  SPEED. Every speed a student writes is a percent of the vehicle's top speed,
  and setSpeedLimit() sets the top speed as a percent of full power. So at a
  limit of 50, drive(100, 100) is half power and drive(50, 50) is a quarter.
  Scaling rather than clipping keeps the steering working at full stick: with
  a clip, a full-forward stick would hold both sides at the limit and a gentle
  turn would do nothing. The full operating programs have no limit at all;
  this is for the Explorers sketches only. The default is 50.

  LIGHTS. The light functions only change a list of 32 colors in memory. A
  small background task sends that list to the strip once the program has
  stopped changing it for a moment, and controllerReady() sends it at the top
  of every loop, when the picture is finished. Students' loops redraw the
  lights thousands of times a second; sending only finished pictures is what
  keeps the strip from flickering, and keeps show() from being called often
  enough to starve the Bluetooth radio. Students never have to call show().
*/

#pragma once

#include <Bluepad32.h>
#include <uni.h>                 // The Bluetooth allowlist
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>         // Remembers the claimed controller

// ===================================================================
// THE HARDWARE - the same pins as pathfinder_nintendoswitch
// ===================================================================

const int LIGHT_PIN      = 5;     // All 32 lights are wired to GPIO 5
const int LIGHT_COUNT    = 32;    // 0-15 across the front, 16-31 across the back
const int MAX_BRIGHTNESS = 120;   // What setBrightness(100) means. Kind to eyes and battery.

// Two pins per motor, and a different PWM channel number for each pin.
const int FRONT_LEFT_PIN_A  = 12, FRONT_LEFT_CH_A  = 0;
const int FRONT_LEFT_PIN_B  = 13, FRONT_LEFT_CH_B  = 1;
const int REAR_LEFT_PIN_A   = 18, REAR_LEFT_CH_A   = 2;
const int REAR_LEFT_PIN_B   = 19, REAR_LEFT_CH_B   = 3;
const int FRONT_RIGHT_PIN_A = 22, FRONT_RIGHT_CH_A = 4;
const int FRONT_RIGHT_PIN_B = 23, FRONT_RIGHT_CH_B = 5;
const int REAR_RIGHT_PIN_A  = 16, REAR_RIGHT_CH_A  = 6;
const int REAR_RIGHT_PIN_B  = 17, REAR_RIGHT_CH_B  = 7;

const int MOTOR_PWM_FREQ = 20000;   // 20 kHz, too high for people to hear
const int MOTOR_PWM_BITS = 8;       // Power goes from 0 to 255
const int MOTOR_MAX      = 255;     // Full power

// ===================================================================
// COLORS
// ===================================================================

// A color is one number that holds three: how much red, green and blue.
typedef uint32_t Color;

Color mixColor(int red, int green, int blue) {
  red   = constrain(red,   0, 255);
  green = constrain(green, 0, 255);
  blue  = constrain(blue,  0, 255);
  return ((Color)red << 16) | ((Color)green << 8) | (Color)blue;
}

// Every named color is just a mix. Change one here and it changes everywhere.
const Color RED    = mixColor(255,   0,   0);
const Color ORANGE = mixColor(255,  90,   0);
const Color YELLOW = mixColor(255, 190,   0);
const Color GREEN  = mixColor(  0, 255,   0);
const Color CYAN   = mixColor(  0, 255, 255);
const Color BLUE   = mixColor(  0,   0, 255);
const Color PURPLE = mixColor(140,   0, 255);
const Color PINK   = mixColor(255,  30, 120);
const Color WHITE  = mixColor(255, 255, 255);
const Color OFF    = mixColor(  0,   0,   0);

// ===================================================================
// LIGHTS
// ===================================================================

Adafruit_NeoPixel explorerStrip(LIGHT_COUNT, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

Color lightWanted[LIGHT_COUNT];              // The picture your program is drawing
Color lightShown[LIGHT_COUNT];               // The picture the strip is showing
volatile bool lightsChanged = false;
volatile unsigned long lightChangedAt = 0;   // micros() of the last change
SemaphoreHandle_t lightsLock = nullptr;      // Only one job may talk to the strip at a time
bool roverStarted = false;
unsigned long lastBadLightWarning = 0;
unsigned long lastLightSend = 0;

// Sends the picture to the strip, but only if it is different from what the
// strip is already showing (or if `always` says to send it anyway). At most
// 100 pictures a second, which is far faster than an eye can follow.
void sendLights(bool always) {
  if (lightsLock == nullptr) {
    return;                       // startRover() has not run yet
  }
  if (!always && millis() - lastLightSend < 10) {
    return;                       // Sent one a moment ago. It will go next time.
  }
  xSemaphoreTake(lightsLock, portMAX_DELAY);
  lightsChanged = false;

  bool different = always;
  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (lightShown[i] != lightWanted[i]) {
      different = true;
    }
  }

  if (different) {
    for (int i = 0; i < LIGHT_COUNT; i++) {
      lightShown[i] = lightWanted[i];
      explorerStrip.setPixelColor(i, lightShown[i]);
    }
    explorerStrip.show();
    lastLightSend = millis();
  }
  xSemaphoreGive(lightsLock);
}

// Runs in the background, all the time. When the program has stopped changing
// the lights for a moment - it is waiting in delay(), or it has finished - the
// new picture goes out to the strip.
void explorerLightsTask(void *unused) {
  for (;;) {
    if (lightsChanged && (micros() - lightChangedAt) > 1500) {
      sendLights(false);
    }
    vTaskDelay(pdMS_TO_TICKS(4));
  }
}

void setLight(int number, Color color) {
  if (number < 0 || number >= LIGHT_COUNT) {
    if (lastBadLightWarning == 0 || millis() - lastBadLightWarning > 1000) {
      lastBadLightWarning = millis() + 1;
      Serial.print("There is no light number ");
      Serial.print(number);
      Serial.println(". The lights go from 0 to 31.");
    }
    return;
  }
  if (lightWanted[number] != color) {
    lightWanted[number] = color;
    lightChangedAt = micros();
    lightsChanged = true;
  }
}

void setLights(int first, int last, Color color) {
  if (first > last) {
    int swap = first;
    first = last;
    last = swap;
  }
  for (int number = first; number <= last; number++) {
    setLight(number, color);
  }
}

void setAllLights(Color color)   { setLights(0, 31, color); }
void setFrontLights(Color color) { setLights(0, 15, color); }
void setBackLights(Color color)  { setLights(16, 31, color); }
void lightsOff()                 { setLights(0, 31, OFF); }

// The loop of lights goes left to right across the front (0-15), then right
// to left across the back (16-31). So the left side is 0-7 at the front and
// 24-31 at the back, and the right side is 8-15 and 16-23.
void setLeftLights(Color color) {
  setLights(0, 7, color);
  setLights(24, 31, color);
}

void setRightLights(Color color) {
  setLights(8, 15, color);
  setLights(16, 23, color);
}

// The back light directly behind a front light. Front light 0 is the far
// left, and so is back light 31. Front 15 is the far right, and so is back 16.
int behind(int frontNumber) {
  return (LIGHT_COUNT - 1) - frontNumber;
}

void setBrightness(int percent) {
  percent = constrain(percent, 0, 100);
  if (lightsLock == nullptr) {
    return;
  }
  xSemaphoreTake(lightsLock, portMAX_DELAY);
  explorerStrip.setBrightness(map(percent, 0, 100, 0, MAX_BRIGHTNESS));
  xSemaphoreGive(lightsLock);
  sendLights(true);
}

// 0 is red, about 17 is yellow, 33 is green, 50 is cyan, 67 is blue, 83 is
// purple, and 100 is back round to red again.
Color rainbowColor(int percent) {
  percent = ((percent % 100) + 100) % 100;
  uint16_t hue = (uint16_t)((long)percent * 65536L / 100);
  return explorerStrip.gamma32(explorerStrip.ColorHSV(hue));
}

Color randomColor() {
  return rainbowColor(random(100));
}

// ===================================================================
// MOTORS
// ===================================================================

int speedLimit = 50;     // Top speed, as a percent of full power
int driveSpeed = 40;     // Speed of the autopilot moves, percent of top speed

void setSpeedLimit(int percent) {
  speedLimit = constrain(percent, 0, 100);
  Serial.print("Speed limit: ");
  Serial.print(speedLimit);
  Serial.println("%");
}

void setDriveSpeed(int percent) {
  driveSpeed = constrain(percent, 0, 100);
}

void explorerAttachMotor(int pin, int channel) {
  ledcSetup(channel, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttachPin(pin, channel);
}

// One motor. Plus turns it one way, minus the other way, 0 lets it coast.
void explorerSetMotor(int channelA, int channelB, int percent) {
  percent = constrain(percent, -100, 100);
  int duty = abs(percent) * speedLimit * MOTOR_MAX / 10000;
  if (percent >= 0) {
    ledcWrite(channelA, duty);
    ledcWrite(channelB, 0);
  } else {
    ledcWrite(channelA, 0);
    ledcWrite(channelB, duty);
  }
}

// Tank drive: the two left wheels together, and the two right wheels together.
void drive(int leftSpeed, int rightSpeed) {
  if (!roverStarted) {
    return;
  }
  explorerSetMotor(FRONT_LEFT_CH_A,  FRONT_LEFT_CH_B,  leftSpeed);
  explorerSetMotor(REAR_LEFT_CH_A,   REAR_LEFT_CH_B,   leftSpeed);
  explorerSetMotor(FRONT_RIGHT_CH_A, FRONT_RIGHT_CH_B, rightSpeed);
  explorerSetMotor(REAR_RIGHT_CH_A,  REAR_RIGHT_CH_B,  rightSpeed);
}

void stopMotors() {
  drive(0, 0);
}

// ===================================================================
// TIMING
// ===================================================================

// Looks at the clock instead of waiting, so the rest of loop() keeps running.
bool blinkIsOn() {
  return (millis() / 400) % 2 == 0;
}

bool timeToPrint() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 250) {
    lastPrint = millis();
    return true;
  }
  return false;
}

bool timeToPrintWarning() {
  static unsigned long lastWarning = 0;
  if (lastWarning == 0 || millis() - lastWarning >= 3000) {
    lastWarning = millis() + 1;
    return true;
  }
  return false;
}

// ===================================================================
// STARTING UP
// ===================================================================

void startRover() {
  if (roverStarted) {
    return;
  }
  Serial.begin(115200);

  explorerAttachMotor(FRONT_LEFT_PIN_A,  FRONT_LEFT_CH_A);
  explorerAttachMotor(FRONT_LEFT_PIN_B,  FRONT_LEFT_CH_B);
  explorerAttachMotor(REAR_LEFT_PIN_A,   REAR_LEFT_CH_A);
  explorerAttachMotor(REAR_LEFT_PIN_B,   REAR_LEFT_CH_B);
  explorerAttachMotor(FRONT_RIGHT_PIN_A, FRONT_RIGHT_CH_A);
  explorerAttachMotor(FRONT_RIGHT_PIN_B, FRONT_RIGHT_CH_B);
  explorerAttachMotor(REAR_RIGHT_PIN_A,  REAR_RIGHT_CH_A);
  explorerAttachMotor(REAR_RIGHT_PIN_B,  REAR_RIGHT_CH_B);
  roverStarted = true;
  stopMotors();

  for (int i = 0; i < LIGHT_COUNT; i++) {
    lightWanted[i] = OFF;
    lightShown[i] = OFF;
  }
  lightsLock = xSemaphoreCreateMutex();
  explorerStrip.begin();
  explorerStrip.setBrightness(MAX_BRIGHTNESS / 2);
  explorerStrip.clear();
  explorerStrip.show();
  xTaskCreatePinnedToCore(explorerLightsTask, "lights", 4096, nullptr, 1,
                          nullptr, xPortGetCoreID());

  randomSeed(esp_random());
  delay(200);
  Serial.println();
  Serial.println("Pathfinder Explorer ready!");
}

// ===================================================================
// THE CONTROLLER
// ===================================================================

ControllerPtr myPad = nullptr;
uint8_t myPadAddress[6];
bool controllerStarted = false;
bool padClaimed = false;
bool padWasConnected = false;
bool autopilotCancelled = false;
uint16_t buttonsNow = 0;
uint16_t buttonsBefore = 0;
uint8_t dpadNow = 0;

void printAddress(const uint8_t *address) {
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", address[i]);
    if (i < 5) {
      Serial.print(":");
    }
  }
}

// The claimed controller lives in the ESP32's flash memory, under this name.
bool loadClaimedController(uint8_t address[6]) {
  Preferences store;
  if (!store.begin("explorers", true)) {
    return false;               // Nothing has ever been saved on this vehicle
  }
  size_t got = store.getBytes("controller", address, 6);
  store.end();
  if (got != 6) {
    return false;
  }
  for (int i = 0; i < 6; i++) {
    if (address[i] != 0) {
      return true;
    }
  }
  return false;
}

bool saveClaimedController(const uint8_t address[6]) {
  Preferences store;
  if (!store.begin("explorers", false)) {
    return false;
  }
  size_t put = store.putBytes("controller", address, 6);
  store.end();
  return put == 6;
}

// Bluepad32 calls these two by itself when a controller comes and goes.
void explorerPadConnected(ControllerPtr pad) {
  const uint8_t *address = pad->getProperties().btaddr;
  if (myPad != nullptr || memcmp(address, myPadAddress, 6) != 0) {
    pad->disconnect();          // Not ours. The allowlist should have stopped it.
    return;
  }
  myPad = pad;
}

void explorerPadDisconnected(ControllerPtr pad) {
  if (pad == myPad) {
    myPad = nullptr;
  }
}

void startController() {
  startRover();
  controllerStarted = true;

  padClaimed = loadClaimedController(myPadAddress);
  if (!padClaimed) {
    Serial.println();
    Serial.println("This vehicle does not know which controller is its own yet,");
    Serial.println("so it will not drive. The lights will blink RED.");
    Serial.println("Ask your teacher to run e00_claim_controller on it.");
    return;
  }

  BP32.setup(&explorerPadConnected, &explorerPadDisconnected);
  BP32.enableVirtualDevice(false);

  // The allowlist is Bluepad32's guest list. Only our controller is on it.
  bd_addr_t allowed;
  memcpy(allowed, myPadAddress, 6);
  uni_bt_allowlist_remove_all();
  uni_bt_allowlist_add_addr(allowed);
  uni_bt_allowlist_set_enabled(true);
  BP32.enableNewBluetoothConnections(true);

  Serial.print("This vehicle only listens to controller ");
  printAddress(myPadAddress);
  Serial.println();
  Serial.println("Press any button on it to connect.");
}

// Blinks all the lights slowly in one color while we wait.
void waitingBlink(Color color) {
  if ((millis() / 500) % 2 == 0) {
    setAllLights(color);
  } else {
    lightsOff();
  }
}

bool controllerReady() {
  sendLights(false);            // The picture from last time round is finished

  if (!controllerStarted || !padClaimed) {
    stopMotors();
    waitingBlink(mixColor(80, 0, 0));
    if (!controllerStarted && timeToPrintWarning()) {
      Serial.println("Put startController(); inside setup().");
    }
    return false;
  }

  BP32.update();

  if (myPad == nullptr || !myPad->isConnected()) {
    stopMotors();               // Safety first: never drive without a controller
    if (padWasConnected) {
      padWasConnected = false;
      Serial.println("Controller lost! Motors stopped.");
    }
    waitingBlink(mixColor(0, 60, 0));
    return false;
  }

  if (!padWasConnected) {
    padWasConnected = true;
    lightsOff();
    myPad->setPlayerLEDs(0x01);
    buttonsNow = myPad->buttons();   // A button held while connecting is not a tap
    dpadNow = myPad->dpad();
    Serial.println("Controller connected. Let's go!");
  }

  buttonsBefore = buttonsNow;
  buttonsNow = myPad->buttons();
  dpadNow = myPad->dpad();

  if (autopilotCancelled && !(buttonsNow & BUTTON_A)) {
    autopilotCancelled = false;      // B has been let go, so autopilot may run again
  }
  return true;
}

// Bluepad32 names the face buttons by POSITION, the way an Xbox pad does:
// a() is the bottom one, b() the right, x() the left, y() the top. A Switch
// pad prints different letters in those places - B at the bottom, A on the
// right, Y on the left, X on the top. These functions go by the letter
// PRINTED on a Switch pad, which is the one students can see.
bool padHolding(uint16_t bit) { return (buttonsNow & bit) != 0; }
bool padTapped(uint16_t bit)  { return (buttonsNow & bit) && !(buttonsBefore & bit); }

bool buttonA()  { return padHolding(BUTTON_B); }   // Right
bool buttonB()  { return padHolding(BUTTON_A); }   // Bottom
bool buttonX()  { return padHolding(BUTTON_Y); }   // Top
bool buttonY()  { return padHolding(BUTTON_X); }   // Left
bool tappedA()  { return padTapped(BUTTON_B); }
bool tappedB()  { return padTapped(BUTTON_A); }
bool tappedX()  { return padTapped(BUTTON_Y); }
bool tappedY()  { return padTapped(BUTTON_X); }

bool buttonL()  { return padHolding(BUTTON_SHOULDER_L); }
bool buttonR()  { return padHolding(BUTTON_SHOULDER_R); }
bool buttonZL() { return padHolding(BUTTON_TRIGGER_L); }
bool buttonZR() { return padHolding(BUTTON_TRIGGER_R); }

bool dpadUp()    { return (dpadNow & DPAD_UP) != 0; }
bool dpadDown()  { return (dpadNow & DPAD_DOWN) != 0; }
bool dpadLeft()  { return (dpadNow & DPAD_LEFT) != 0; }
bool dpadRight() { return (dpadNow & DPAD_RIGHT) != 0; }

// Bluepad32 gives -512 to 511, with UP as minus. These give -100 to 100, with
// UP as plus, because that is how people think about it.
int stickPercent(int32_t raw) {
  return constrain((int)(raw * 100 / 511), -100, 100);
}

int leftStickX()  { return myPad ? stickPercent(myPad->axisX())   : 0; }
int leftStickY()  { return myPad ? stickPercent(-myPad->axisY())  : 0; }
int rightStickX() { return myPad ? stickPercent(myPad->axisRX())  : 0; }
int rightStickY() { return myPad ? stickPercent(-myPad->axisRY()) : 0; }

void rumble(int ms) {
  if (myPad != nullptr && myPad->isConnected()) {
    myPad->playDualRumble(0, ms, 0x80, 0x40);
  }
}

// ===================================================================
// AUTOPILOT
// ===================================================================

// Waits, but keeps an ear on the controller so B can cancel the move.
void autopilotWait(int ms) {
  unsigned long start = millis();
  while (millis() - start < (unsigned long)ms) {
    if (controllerStarted && padClaimed) {
      BP32.update();
      bool connected = (myPad != nullptr && myPad->isConnected());
      if (!connected || (myPad->buttons() & BUTTON_A)) {   // B on a Switch pad
        stopMotors();
        autopilotCancelled = true;
        Serial.println("Autopilot cancelled!");
        return;
      }
    }
    delay(5);
  }
}

void autopilotMove(const char *name, int leftSpeed, int rightSpeed, int ms) {
  if (autopilotCancelled) {
    return;
  }
  Serial.print(name);
  Serial.print(" for ");
  Serial.print(ms);
  Serial.println(" ms");
  drive(leftSpeed, rightSpeed);
  autopilotWait(ms);
  stopMotors();
}

void forward(int ms)   { autopilotMove("Forward",    driveSpeed,  driveSpeed, ms); }
void backward(int ms)  { autopilotMove("Backward",  -driveSpeed, -driveSpeed, ms); }
void spinLeft(int ms)  { autopilotMove("Spin left", -driveSpeed,  driveSpeed, ms); }
void spinRight(int ms) { autopilotMove("Spin right", driveSpeed, -driveSpeed, ms); }

void pause(int ms) {
  if (autopilotCancelled) {
    return;
  }
  stopMotors();
  autopilotWait(ms);
}

void countdown(int seconds) {
  for (int count = seconds; count > 0; count--) {
    Serial.print(count);
    Serial.println("...");
    setAllLights(YELLOW);
    delay(250);
    lightsOff();
    delay(750);
  }
  Serial.println("GO!");
  setAllLights(GREEN);
  delay(400);
  lightsOff();
}
