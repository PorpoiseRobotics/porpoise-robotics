/*
  l1c_first_pixel.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 1

  WHAT THIS PROGRAM DOES
  ----------------------
  Lights up ONE of the 32 LEDs on the vehicle and cycles it through red, green,
  blue and white. Nothing moves - no motors, no servos - so the vehicle is safe
  on the bench.

  This is your first program that uses a LIBRARY. A library is code somebody
  else already wrote and tested, which your program can call instead of you
  having to work out the timing of the LED signal yourself. Adafruit wrote this
  one. The compiler is the tool that welds your code and the library together
  into the machine language the ESP32 actually runs.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  2. Library: "Adafruit NeoPixel" by Adafruit
       Tools > Manage Libraries, search for it, click Install.

  WHAT TO TRY
  -----------
  1. Upload it and find which physical LED lights up. Where is number 0?
  2. Change WHICH_LED to 15, then to 16, then to 31. Now draw the vehicle on
     paper and mark where LED 0, 8, 15, 16, 24 and 31 are. You will need that
     map in Lesson 4.
  3. Change BRIGHTNESS from 60 to 255. Do not stare at it from close up.
  4. Make up a colour of your own with strip.Color(red, green, blue), where
     each number is 0 to 255. What do you get from (255, 255, 0)? From
     (128, 0, 128)?
*/

#include <Adafruit_NeoPixel.h>

const int LED_PIN    = 5;    // All 32 LEDs are wired to GPIO 5 on this vehicle
const int LED_COUNT  = 32;   // Two bars of 16
const int WHICH_LED  = 0;    // The one LED this program lights. 0 is the first.
const int BRIGHTNESS = 60;   // Master brightness, 0 (off) to 255 (blinding)

// This line creates the object we talk to the LED strip through. NEO_GRB says
// these LEDs expect their colour data green first, and NEO_KHZ800 is the speed
// the data is clocked out at. Both are right for the WS2812B LEDs we use.
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  strip.begin();                    // Wake the strip up. Always required.
  strip.setBrightness(BRIGHTNESS);
  strip.clear();                    // Set every LED to off, in memory
  strip.show();                     // Send it to the hardware. See the note below.

  Serial.print("Lighting LED number ");
  Serial.println(WHICH_LED);
}

void loop() {
  // setPixelColor() only changes the colour stored in the ESP32 memory.
  // NOTHING happens on the vehicle until show() pushes that memory out to the
  // LEDs. Forgetting show() is the single most common NeoPixel mistake.

  strip.setPixelColor(WHICH_LED, strip.Color(255, 0, 0));   // Red
  strip.show();
  delay(1000);

  strip.setPixelColor(WHICH_LED, strip.Color(0, 255, 0));   // Green
  strip.show();
  delay(1000);

  strip.setPixelColor(WHICH_LED, strip.Color(0, 0, 255));   // Blue
  strip.show();
  delay(1000);

  // All three colours at once makes white. That is how your television works.
  strip.setPixelColor(WHICH_LED, strip.Color(255, 255, 255));
  strip.show();
  delay(1000);

  strip.clear();     // Everything off...
  strip.show();      // ...and pushed out to the hardware
  delay(1000);
}
