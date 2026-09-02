/*
  l5a_millis_not_delay.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 5

  WHAT THIS PROGRAM DOES
  ----------------------
  Blinks two things at two different speeds AT THE SAME TIME, and counts how
  many times loop() runs while it does it. Nothing moves.

  This is the idea that separates a toy program from a real one, and it is the
  reason the vehicle can drive, flash a turn signal and run a scanner all at
  once without any of them getting in each other's way.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  2. Library: "Adafruit NeoPixel" by Adafruit

  THE PROBLEM WITH delay()
  ------------------------
  delay(1000) does not mean "come back in a second". It means "stand here and
  do absolutely nothing for a second". While the ESP32 is inside a delay() it
  is not reading the controller, not checking whether the vehicle has hit
  something, and not updating any other light. Everything stops.

  In Lesson 2 that was fine, because blinking was all we wanted. As soon as two
  things have to happen at once, delay() falls apart. You cannot blink a turn
  signal every 300 ms and a scanner every 40 ms with delay(), because the two
  numbers get in each other's way.

  THE ANSWER: WATCH THE CLOCK INSTEAD OF STOPPING
  -----------------------------------------------
  millis() returns how many milliseconds the board has been switched on for.
  It costs nothing to read. So instead of stopping, we ask each time round the
  loop: "has enough time gone by yet?"

        if (millis() - lastTime >= INTERVAL) {
          lastTime = millis();
          ... do the thing ...
        }

  loop() keeps running at full speed, and each job fires on its own schedule.
  This pattern appears everywhere in pathfinder_ps3.ino - the scanner, the
  waiting-for-a-controller blink, all of it.

  WATCH THE LOOP COUNTER. It tells you how many times per second the program
  gets round loop(). That is how much attention the vehicle has spare.

  WHAT TO TRY
  -----------
  1. Upload and watch. The two lights blink at different rates, and the loop
     counter is enormous - tens of thousands per second.
  2. Now uncomment the delay(1000) line at the bottom of loop(). Upload again.
     Both lights break, and the loop counter falls to 1. Comment it back out.
  3. Change SLOW_MS and FAST_MS. Can you get the two lights to line up every
     third blink?
  4. Add a THIRD light on its own timer without touching the other two.
*/

#include <Adafruit_NeoPixel.h>

const int LED_PIN    = 5;
const int LED_COUNT  = 32;
const int BRIGHTNESS = 60;

const int SLOW_LED = 0;      // Front left, blinks slowly
const int FAST_LED = 15;     // Front right, blinks quickly

const unsigned long SLOW_MS   = 1000;   // Slow light: once a second
const unsigned long FAST_MS   = 150;    // Fast light: about seven times a second
const unsigned long REPORT_MS = 1000;   // How often to print the loop count

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Each job needs its own "when did I last run" variable. "unsigned long" is a
// big whole number that cannot go negative - the right type for millis().
unsigned long slowLastChange   = 0;
unsigned long fastLastChange   = 0;
unsigned long reportLastChange = 0;

bool slowIsOn = false;
bool fastIsOn = false;

long loopCount = 0;

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();

  Serial.println("Two lights, two speeds, one loop.");
  Serial.println("Watch how many times per second loop() runs.");
}

void loop() {
  loopCount++;

  unsigned long now = millis();

  // ---- Job 1: the slow light ----
  if (now - slowLastChange >= SLOW_MS) {
    slowLastChange = now;
    slowIsOn = !slowIsOn;            // "!" flips true to false and back
    strip.setPixelColor(SLOW_LED, slowIsOn ? strip.Color(255, 0, 0)
                                           : strip.Color(0, 0, 0));
    strip.show();
  }

  // ---- Job 2: the fast light ----
  if (now - fastLastChange >= FAST_MS) {
    fastLastChange = now;
    fastIsOn = !fastIsOn;
    strip.setPixelColor(FAST_LED, fastIsOn ? strip.Color(0, 0, 255)
                                           : strip.Color(0, 0, 0));
    strip.show();
  }

  // ---- Job 3: report how busy we are ----
  if (now - reportLastChange >= REPORT_MS) {
    reportLastChange = now;
    Serial.print("loop() ran ");
    Serial.print(loopCount);
    Serial.println(" times in the last second");
    loopCount = 0;
  }

  // ---- Uncomment the next line to see what delay() does to all of this ----
  // delay(1000);
}
