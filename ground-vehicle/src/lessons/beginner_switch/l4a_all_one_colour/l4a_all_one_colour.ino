/*
  l4a_all_one_colour.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 4

  WHAT THIS PROGRAM DOES
  ----------------------
  Sets all 32 LEDs to the same colour, one colour after another, using a for
  loop. Nothing moves. It also prints how much current that colour is drawing
  from the battery, which turns out to matter a great deal.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
       Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  2. Library: "Adafruit NeoPixel" by Adafruit

  THE POWER BUDGET - READ THIS BEFORE YOU TURN THE BRIGHTNESS UP
  --------------------------------------------------------------
  Each NeoPixel holds three LEDs: one red, one green, one blue. Each of those
  draws about 20 mA at full brightness, so one pixel showing full white draws
  about 60 mA. Our vehicle has 32 of them:

        32 pixels x 60 mA = 1920 mA, which is nearly 2 amps
        1920 mA x 5 V     = 9600 mW, or 9.6 watts

  That is a lot of power to pull through a small board, and it makes real heat.
  Two things keep it sane:
    - setBrightness() scales EVERY pixel down before it is sent. At 60 out of
      255 you are drawing roughly a quarter of the worst case.
    - Coloured light is cheaper than white. Pure red only lights one of the
      three LEDs in each pixel, so it costs about a third of what white costs.

  This is why the vehicle programs run the LEDs at about 120 rather than 255,
  and why full white across all 32 is used for a moment and not held.

  WHAT TO TRY
  -----------
  1. Watch the estimated current print for each colour. Which colour is
     cheapest? Which is most expensive? Why?
  2. Change BRIGHTNESS to 255 and put your hand near the LED bars after a
     minute of white. Then put it back to 60.
  3. Change the for loop to  i = i + 2  and see what happens. Why?
  4. Use strip.fill() instead of the loop. Look up what arguments it takes.
     Does it do the same thing in fewer lines?
*/

#include <Adafruit_NeoPixel.h>

const int LED_PIN    = 5;
const int LED_COUNT  = 32;
const int BRIGHTNESS = 60;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();

  Serial.println("All 32 LEDs, one colour at a time.");
}

/*
  Sets every LED on the strip to one colour and works out roughly how much
  current that costs.

  Each of the three colours inside a pixel draws about 20 mA flat out, and
  setBrightness scales the value we asked for before it reaches the LED, so:

        current for one colour channel = 20 mA x (value/255) x (brightness/255)

  This is an estimate, not a measurement. The advanced vehicles carry a real
  current sensor - you will meet it if you move on to Op Program 12.
*/
void fillAll(const char *name, int red, int green, int blue) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(red, green, blue));
  }
  strip.show();

  long total = (long)(red + green + blue);
  long milliamps = total * 20L * LED_COUNT * BRIGHTNESS / (255L * 255L);

  Serial.print(name);
  Serial.print("\t rgb(");
  Serial.print(red);   Serial.print(", ");
  Serial.print(green); Serial.print(", ");
  Serial.print(blue);  Serial.print(")\t about ");
  Serial.print(milliamps);
  Serial.println(" mA");
}

void loop() {
  fillAll("red    ", 255,   0,   0);
  delay(1500);

  fillAll("green  ",   0, 255,   0);
  delay(1500);

  fillAll("blue   ",   0,   0, 255);
  delay(1500);

  fillAll("yellow ", 255, 255,   0);
  delay(1500);

  fillAll("cyan   ",   0, 255, 255);
  delay(1500);

  fillAll("magenta", 255,   0, 255);
  delay(1500);

  fillAll("white  ", 255, 255, 255);
  delay(1500);

  fillAll("off    ",   0,   0,   0);
  delay(2000);
}
