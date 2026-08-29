/*
  l5b_button_toggle.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 5

  WHAT THIS PROGRAM DOES
  ----------------------
  Turns the headlights on and off with the LEFT face button, so that one press
  gives you exactly one change. Nothing moves.

  It also shows you, on the Serial Monitor, what goes wrong if you do it the
  obvious way instead - because the obvious way does not work, and finding out
  why is the whole point of this program.

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
    LEFT face button  Headlights on / off  (done properly. Marked Y on most pads.)
    TOP face button   The broken version, so you can see the difference
    BOTTOM button     Reset the counters

  WHY THE OBVIOUS WAY FAILS
  -------------------------
  From Lesson 5a you know loop() runs tens of thousands of times a second. A
  human finger holds a button down for something like 100 milliseconds, which
  is thousands of trips round the loop. So this:

        if (myController->x()) {
          lightsOn = !lightsOn;          // WRONG
        }

  flips the lights thousands of times during one press. Whether they end up on
  or off is pure luck, depending on whether you happened to let go after an odd
  or an even number of flips.

  What we actually want is the MOMENT the button goes down - the edge, not the
  level. To find an edge you have to remember what the button was doing last
  time round:

        bool isNewPress = isDown && !wasDown;
        wasDown = isDown;

  That is called EDGE DETECTION, and justPressed() below is exactly the
  function pathfinder_nintendoswitch.ino uses for every one of its buttons.

  WHAT TO TRY
  -----------
  1. Press the left face button a few times. One press, one change, every time.
  2. Now press the top face button once and look at the Serial Monitor. How
     many times did one press register?
  3. Hold the left button down for five seconds. Does anything extra happen?
     Should it?
  4. Change justPressed so it fires on RELEASE instead of on press. Which feels
     better to use?
*/

#include <Bluepad32.h>
#include <uni.h>
#include <Adafruit_NeoPixel.h>

const uint8_t MY_CONTROLLER[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const int LED_PIN     = 5;
const int LED_COUNT   = 32;
const int BRIGHTNESS  = 60;
const int FRONT_FIRST = 0;
const int FRONT_LAST  = 15;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool lightsOn       = false;   // What the left face button controls
bool brokenLightsOn = false;   // What the top face button controls, badly

bool lightsButtonWasDown = false;   // Remembers the button from last time round
bool resetButtonWasDown  = false;
bool brokenButtonWasDown = false;

long goodPressCount  = 0;     // How many times the good version changed
long brokenFlipCount = 0;     // How many times the broken version changed

ControllerPtr myController = nullptr;
bool addressIsSet = false;
bool wasConnected = false;

/*
  Returns true only on the FIRST time round loop() that a button is held down.

  The "&" on wasDown means the function is handed the actual variable rather
  than a copy of it, so what it writes there is still there next time.
*/
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
  controller->setPlayerLEDs(0x01);
}

void onDisconnectedController(ControllerPtr controller) {
  if (myController == controller) {
    myController = nullptr;
  }
}

void showHeadlights() {
  strip.clear();
  if (lightsOn) {
    for (int i = FRONT_FIRST; i <= FRONT_LAST; i++) {
      strip.setPixelColor(i, strip.Color(255, 255, 255));
    }
  }
  strip.show();
}

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();

  for (int i = 0; i < 6; i++) {
    if (MY_CONTROLLER[i] != 0x00) {
      addressIsSet = true;
    }
  }

  Serial.println();
  Serial.println("LEFT face button   = headlights, done properly");
  Serial.println("TOP face button    = the broken version, watch the count");
  Serial.println("BOTTOM face button = reset the counters");

  if (!addressIsSet) {
    Serial.println();
    Serial.println("MY_CONTROLLER has not been filled in. Run l3a_controller_check.");
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

  Serial.println("Waiting for the controller...");
}

void loop() {
  if (!addressIsSet) {
    delay(1000);
    return;
  }

  BP32.update();

  if (myController == nullptr || !myController->isConnected()) {
    wasConnected = false;
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    Serial.println("Controller connected.");
    showHeadlights();
  }

  // ---- The right way: fire once, on the edge ----
  if (justPressed(myController->x(), lightsButtonWasDown)) {
    lightsOn = !lightsOn;
    goodPressCount++;

    showHeadlights();

    Serial.print("LEFT button: headlights ");
    Serial.print(lightsOn ? "ON " : "OFF");
    Serial.print("   (changed ");
    Serial.print(goodPressCount);
    Serial.println(" times in total)");
  }

  // ---- The wrong way: fire on the level, every single pass ----
  // Deliberately broken. This is what happens without edge detection.
  bool brokenButton = myController->y();
  if (brokenButton) {
    brokenLightsOn = !brokenLightsOn;
    brokenFlipCount++;
  }

  // Report the damage once the button comes back up.
  if (!brokenButton && brokenButtonWasDown) {
    Serial.print("TOP button: one press flipped the value ");
    Serial.print(brokenFlipCount);
    Serial.println(" times. That is the bug.");
    brokenFlipCount = 0;
  }
  brokenButtonWasDown = brokenButton;

  // ---- Reset ----
  if (justPressed(myController->a(), resetButtonWasDown)) {
    goodPressCount = 0;
    brokenFlipCount = 0;
    Serial.println("Counters reset.");
  }
}
