/*
  l4c_patterns.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 4

  WHAT THIS PROGRAM DOES
  ----------------------
  Runs four light patterns in turn: a colour wipe, a theatre chase, a rainbow,
  and the KITT scanner from the full vehicle program. Nothing moves.

  Each pattern is written as its own FUNCTION. A function is a piece of the
  program with a name, written once and called as many times as you like. Look
  at how short loop() is compared with what actually happens on the vehicle -
  that is what functions buy you.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
       Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  2. Library: "Adafruit NeoPixel" by Adafruit

  WHAT TO TRY
  -----------
  1. Change the wait argument on colorWipe from 30 to 5 and to 150.
  2. In theaterChase, change the 3 to a 4 in both places. What changes?
  3. In the scanner, change SCANNER_TAIL from 3 to 0, then to 8.
  4. Change the scanner colour to blue and you have a Cylon instead of KITT.
  5. Write a pattern of your own and call it from loop(). Start by copying
     colorWipe and making it run backwards.
*/

#include <Adafruit_NeoPixel.h>

const int LED_PIN     = 5;
const int LED_COUNT   = 32;
const int BRIGHTNESS  = 60;

const int FRONT_FIRST = 0;
const int FRONT_LAST  = 15;
const int REAR_LAST   = 31;

const int SCANNER_TAIL = 3;   // How many fading LEDs trail behind the bright one

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();
}

/*
  Fills the strip one LED at a time, in order, so the colour appears to sweep
  along it. The strip is NOT cleared first, so each wipe paints over the last.
*/
void colorWipe(uint32_t colour, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, colour);
    strip.show();
    delay(wait);
  }
}

/*
  A theatre marquee: every third LED is lit, and the pattern shuffles along by
  one each frame so the lights appear to chase each other.

  Three loops, one inside another. The "b" loop is which of the three
  positions is lit this frame, and the "c" loop steps along the strip in
  threes lighting them.
*/
void theaterChase(uint32_t colour, int wait) {
  for (int a = 0; a < 10; a++) {           // Repeat the whole effect 10 times
    for (int b = 0; b < 3; b++) {          // Three frames per cycle
      strip.clear();
      for (int c = b; c < strip.numPixels(); c += 3) {
        strip.setPixelColor(c, colour);
      }
      strip.show();
      delay(wait);
    }
  }
}

/*
  A rainbow flowing round the whole strip.

  Instead of red, green and blue, this uses HUE: one number that goes all the
  way round the colour wheel. The full circle is 65536 steps. Each LED is given
  a hue a little further round the wheel than the one before, which spreads a
  whole rainbow along the strip, and then the starting point creeps forward so
  the rainbow appears to move.

  gamma32() corrects for the fact that human eyes do not see brightness in a
  straight line, which makes the colours look truer.
*/
void rainbow(int wait) {
  for (long firstPixelHue = 0; firstPixelHue < 65536L; firstPixelHue += 256) {
    for (int i = 0; i < strip.numPixels(); i++) {
      long pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}

/*
  The KITT scanner, as seen in Knight Rider - or a Cylon, if you prefer
  Battlestar Galactica.

  One bright dot slides across the front and back again, with a short fading
  tail behind it. The rear LED opposite the front one is drawn at
  REAR_LAST - position, so the front and rear dots stay side by side.

  The ">> tail" is a right shift, which halves a number. So the tail LEDs get
  255, then 127, then 63, then 31 - each one half as bright as the last.
*/
void scanner(int red, int green, int blue, int wait, int sweeps) {
  for (int sweep = 0; sweep < sweeps; sweep++) {
    // One sweep out...
    for (int position = FRONT_FIRST; position <= FRONT_LAST; position++) {
      drawScannerFrame(position, 1, red, green, blue);
      delay(wait);
    }
    // ...and one sweep back.
    for (int position = FRONT_LAST; position >= FRONT_FIRST; position--) {
      drawScannerFrame(position, -1, red, green, blue);
      delay(wait);
    }
  }
}

/*
  Draws one frame of the scanner: the bright dot at "position", plus its tail
  trailing behind in whichever direction it came from.
*/
void drawScannerFrame(int position, int step, int red, int green, int blue) {
  strip.clear();

  for (int tail = 0; tail <= SCANNER_TAIL; tail++) {
    int p = position - (step * tail);
    if (p < FRONT_FIRST || p > FRONT_LAST) {
      continue;   // This part of the tail has slid off the end, so skip it
    }

    uint32_t colour = strip.Color(red >> tail, green >> tail, blue >> tail);
    strip.setPixelColor(p, colour);              // Front of the vehicle
    strip.setPixelColor(REAR_LAST - p, colour);  // Matching LED at the rear
  }

  strip.show();
}

void loop() {
  Serial.println("colour wipe");
  colorWipe(strip.Color(255, 0, 0), 30);   // Red
  colorWipe(strip.Color(0, 255, 0), 30);   // Green
  colorWipe(strip.Color(0, 0, 255), 30);   // Blue
  colorWipe(strip.Color(0, 0, 0),   30);   // Off again

  Serial.println("theatre chase");
  theaterChase(strip.Color(127, 127, 127), 60);   // White, half brightness
  theaterChase(strip.Color(127, 0, 0),     60);   // Red
  strip.clear();
  strip.show();
  delay(500);

  Serial.println("rainbow");
  rainbow(10);
  strip.clear();
  strip.show();
  delay(500);

  Serial.println("KITT scanner");
  scanner(255, 0, 0, 40, 3);   // Red. Try 0, 0, 255 for a Cylon.
  strip.clear();
  strip.show();
  delay(1500);
}
