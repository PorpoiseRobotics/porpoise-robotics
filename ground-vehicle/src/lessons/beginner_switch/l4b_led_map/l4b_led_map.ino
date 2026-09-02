/*
  l4b_led_map.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 4

  WHAT THIS PROGRAM DOES
  ----------------------
  Teaches you where every LED number physically is on the vehicle. First it
  walks a single white dot all the way round the loop, printing the number as
  it goes. Then it lights the front, the rear, the left side and the right side
  in turn. Then it shows the mirror trick the scanner uses.

  Nothing moves. Sit the vehicle in front of you and watch.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
       Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  2. Library: "Adafruit NeoPixel" by Adafruit

  HOW THE 32 LEDS ARE ARRANGED
  ----------------------------
  The data signal goes into LED 0 and each LED passes it on to the next, so
  they are numbered in the order the wire runs, not in any order that is
  convenient for us:

        LEDs  0..15   across the FRONT, left to right
        LEDs 16..31   across the REAR,  right to left

                        FRONT
              0 1 2 3 4 5 6 7 8 9 ... 15
        LEFT                                RIGHT
             31 30 29 ...        18 17 16
                        REAR

  Two things fall out of that, and both get used constantly:

    - The LEFT of the vehicle is LEDs 0-7 at the front AND 24-31 at the rear.
      The RIGHT is 8-15 at the front AND 16-23 at the rear.

    - The LED directly behind front LED p is always LED (31 - p).
      Front 0 pairs with rear 31. Front 15 pairs with rear 16. Front 3 pairs
      with rear 28. Check that on the vehicle - it is how the scanner keeps its
      front dot and rear dot lined up.

  WHAT TO TRY
  -----------
  1. Run the walk and follow the dot with your finger. Write the numbers on a
     sketch of the vehicle.
  2. During the mirror demonstration, check that the two lit LEDs really are
     opposite each other.
  3. Change the numbers in showSide() so that "front" lights only the middle
     four LEDs of the front bar. Which numbers are those?
  4. Add your own section that lights the four CORNERS of the vehicle.
*/

#include <Adafruit_NeoPixel.h>

const int LED_PIN    = 5;
const int LED_COUNT  = 32;
const int BRIGHTNESS = 60;

// Naming the boundaries keeps the rest of the code readable.
const int FRONT_FIRST = 0;    // Front left
const int FRONT_LAST  = 15;   // Front right
const int REAR_FIRST  = 16;   // Rear right
const int REAR_LAST   = 31;   // Rear left

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();

  Serial.println("LED map. Watch the vehicle, not the screen.");
}

/*
  Lights one run of LEDs, from "first" to "last" inclusive, and says what it
  is doing on the Serial Monitor.
*/
void showSide(const char *name, int first, int last, uint32_t colour) {
  strip.clear();
  for (int i = first; i <= last; i++) {
    strip.setPixelColor(i, colour);
  }
  strip.show();

  Serial.print(name);
  Serial.print("  = LEDs ");
  Serial.print(first);
  Serial.print(" to ");
  Serial.println(last);
}

void loop() {
  uint32_t white = strip.Color(255, 255, 255);
  uint32_t amber = strip.Color(255, 100, 0);
  uint32_t red   = strip.Color(255, 0, 0);
  uint32_t blue  = strip.Color(0, 0, 255);

  // ---- 1. Walk one dot all the way round ----
  Serial.println();
  Serial.println("--- walking the loop, one LED at a time ---");
  for (int i = 0; i < LED_COUNT; i++) {
    strip.clear();
    strip.setPixelColor(i, white);
    strip.show();

    Serial.print("LED ");
    Serial.print(i);
    Serial.println(i <= FRONT_LAST ? "   (front)" : "   (rear)");

    delay(300);
  }

  strip.clear();
  strip.show();
  delay(1000);

  // ---- 2. The four sides ----
  Serial.println();
  Serial.println("--- the four sides ---");

  showSide("FRONT ", FRONT_FIRST, FRONT_LAST, white);
  delay(2000);

  showSide("REAR  ", REAR_FIRST, REAR_LAST, red);
  delay(2000);

  // The left side is in two pieces, because the loop wraps round the back.
  strip.clear();
  for (int i = 0;  i <= 7;  i++) strip.setPixelColor(i, amber);
  for (int i = 24; i <= 31; i++) strip.setPixelColor(i, amber);
  strip.show();
  Serial.println("LEFT   = LEDs 0 to 7 (front) and 24 to 31 (rear)");
  delay(2000);

  strip.clear();
  for (int i = 8;  i <= 15; i++) strip.setPixelColor(i, amber);
  for (int i = 16; i <= 23; i++) strip.setPixelColor(i, amber);
  strip.show();
  Serial.println("RIGHT  = LEDs 8 to 15 (front) and 16 to 23 (rear)");
  delay(2000);

  // ---- 3. The mirror trick ----
  Serial.println();
  Serial.println("--- front LED p, and the rear LED opposite it: 31 - p ---");
  for (int p = FRONT_FIRST; p <= FRONT_LAST; p++) {
    strip.clear();
    strip.setPixelColor(p, white);              // Front
    strip.setPixelColor(REAR_LAST - p, blue);   // Directly behind it
    strip.show();

    Serial.print("front ");
    Serial.print(p);
    Serial.print("  <->  rear ");
    Serial.println(REAR_LAST - p);

    delay(400);
  }

  strip.clear();
  strip.show();
  delay(2000);
}
