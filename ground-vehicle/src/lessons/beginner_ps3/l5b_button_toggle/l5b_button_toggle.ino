/*
  l5b_button_toggle.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 5

  WHAT THIS PROGRAM DOES
  ----------------------
  Turns the headlights on and off with the SQUARE button, so that one press
  gives you exactly one change. Nothing moves.

  It also shows you, on the Serial Monitor, what goes wrong if you do it the
  obvious way instead - because the obvious way does not work, and finding out
  why is the whole point of this program.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  2. Libraries: "PS3 Controller Host" by Jeffrey van Pernis
                "Adafruit NeoPixel" by Adafruit

  CONTROLS
  --------
    SQUARE      Headlights on / off  (done properly)
    TRIANGLE    The broken version, so you can see the difference
    CROSS       Reset the counters

  WHY THE OBVIOUS WAY FAILS
  -------------------------
  From Lesson 5a you know loop() runs tens of thousands of times a second. A
  human finger holds a button down for something like 100 milliseconds, which
  is thousands of trips round the loop. So this:

        if (Ps3.data.button.square) {
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
  function pathfinder_ps3.ino uses for every one of its buttons.

  WHAT TO TRY
  -----------
  1. Press SQUARE a few times. One press, one change, every time.
  2. Now press TRIANGLE once and look at the Serial Monitor. How many times did
     one press register?
  3. Hold SQUARE down for five seconds. Does anything extra happen? Should it?
  4. Change justPressed so it fires on RELEASE instead of on press. Which feels
     better to use?
*/

#include <Ps3Controller.h>
#include <Adafruit_NeoPixel.h>

const char *PS3_MAC_ADDRESS = "02:02:03:04:05:08";

const int LED_PIN     = 5;
const int LED_COUNT   = 32;
const int BRIGHTNESS  = 60;
const int FRONT_FIRST = 0;
const int FRONT_LAST  = 15;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool lightsOn      = false;   // What SQUARE controls
bool brokenLightsOn = false;  // What TRIANGLE controls, badly

bool squareWasDown = false;   // Remembers SQUARE from last time round loop()
bool crossWasDown  = false;

long goodPressCount   = 0;    // How many times the good version changed
long brokenFlipCount  = 0;    // How many times the broken version changed

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

  Ps3.begin(PS3_MAC_ADDRESS);

  Serial.println();
  Serial.println("SQUARE   = headlights, done properly");
  Serial.println("TRIANGLE = the broken version, watch the count");
  Serial.println("CROSS    = reset the counters");
  Serial.println("Waiting for the controller...");
}

void loop() {
  if (!Ps3.isConnected()) {
    wasConnected = false;
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    Ps3.setPlayer(1);
    Serial.println("Controller connected.");
    showHeadlights();
  }

  // ---- The right way: fire once, on the edge ----
  if (justPressed(Ps3.data.button.square, squareWasDown)) {
    lightsOn = !lightsOn;
    goodPressCount++;

    showHeadlights();

    Serial.print("SQUARE: headlights ");
    Serial.print(lightsOn ? "ON " : "OFF");
    Serial.print("   (changed ");
    Serial.print(goodPressCount);
    Serial.println(" times in total)");
  }

  // ---- The wrong way: fire on the level, every single pass ----
  // Deliberately broken. This is what happens without edge detection.
  if (Ps3.data.button.triangle) {
    brokenLightsOn = !brokenLightsOn;
    brokenFlipCount++;
  }

  // Report the damage once the button comes back up.
  static bool triangleWasDown = false;
  if (!Ps3.data.button.triangle && triangleWasDown) {
    Serial.print("TRIANGLE: one press flipped the value ");
    Serial.print(brokenFlipCount);
    Serial.println(" times. That is the bug.");
    brokenFlipCount = 0;
  }
  triangleWasDown = Ps3.data.button.triangle;

  // ---- Reset ----
  if (justPressed(Ps3.data.button.cross, crossWasDown)) {
    goodPressCount = 0;
    brokenFlipCount = 0;
    Serial.println("Counters reset.");
  }
}
