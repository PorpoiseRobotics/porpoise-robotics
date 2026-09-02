/*
  l1a_blink.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 1

  WHAT THIS PROGRAM DOES
  ----------------------
  Blinks an LED you have wired yourself: one second on, one second off,
  forever. It does not touch the motors, the servos or the 32 LEDs around the
  vehicle, so it is completely safe with the vehicle on the bench.

  This is the smallest useful program there is. If this uploads and blinks,
  then your computer, your USB cable, your board package, your board AND your
  wiring are all working, and everything else in the course is built on top of
  that.

  WHAT YOU HAVE TO BUILD FIRST
  ----------------------------
  The ESP32 module on this vehicle has no LED of its own that we can use, so
  you are going to give it one. On a breadboard:

      GPIO 2  ---->  220 ohm resistor  ---->  LED long leg (anode, +)
                                              LED short leg (cathode, -)
                                                     |
                                                    GND on the ESP32

  Three things matter, and all three catch people out:

    1. THE LED IS DIRECTIONAL. The long leg is positive. Put it in backwards
       and nothing happens - no damage, but no light either. Turn it round.

    2. THE RESISTOR IS NOT OPTIONAL. An LED will happily pull more current
       than it can survive. The resistor is what limits that current, and
       without it you get one bright flash and a dead LED.

    3. THE RESISTOR CAN GO ON EITHER SIDE of the LED. Current is the same all
       the way round a series circuit, so it does not matter whether the
       resistor comes before or after. What matters is that it is in the loop.

  WHY 220 OHMS
  ------------
  A GPIO pin drives 3.3 V. A green LED drops about 2.0 V across itself and
  wants roughly 20 mA. Ohm's law gives the resistor you need:

      R = (supply - LED drop) / current
        = (3.3 - 2.0) / 0.020
        = 65 ohms

  220 ohms is the value in the kit and it is a fine choice: it gives
  (3.3 - 2.0) / 220 = about 6 mA, which is dimmer than flat out but perfectly
  visible and comfortably safe for both the LED and the pin. An ESP32 pin
  should not be asked for more than about 20 mA, so erring high is the right
  way to err.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32" by Espressif Systems, VERSION 3.0.7
    File > Preferences > "Additional boards manager URLs", add this line:
      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
    Then Tools > Board > Boards Manager, search "esp32", choose version 3.0.7.
    Finally pick Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  Libraries: none.

  These are the same settings pathfinder_ps3.ino uses, so once you have set
  them up for this sketch you are set up for the whole PS3 track.

  THE THREE PARTS OF EVERY ARDUINO PROGRAM
  ----------------------------------------
  1. INITIALIZATION - the top of the file. Libraries you want to use, and your
     constants and variables. Read before anything else runs.
  2. SETUP - setup() runs ONCE, the moment the board powers up or is reset.
  3. LOOP  - loop() runs over and over, forever, until the power goes off.

  Every program in this course has that shape, including the 500-line one you
  will be driving by Lesson 5.

  WHAT TO TRY
  -----------
  1. Upload it as it is and watch your LED.
  2. Change BLINK_MS to 100 and upload again. What happens?
  3. Change it to 2000. Which number makes the LED blink FASTER - a bigger one
     or a smaller one? Why?
  4. Delete one of the two delay() lines and upload. Explain what you see.
     (Hint: the LED is still blinking. How fast?)
  5. Swap the LED round in the breadboard. What happens, and why?
  6. Work out what resistor you would need for a RED LED, which drops about
     1.8 V instead of 2.0 V.
*/

// GPIO 2 is the pin we drive. "GPIO" stands for General Purpose Input/Output:
// a pin the program can switch on and off.
//
// Any free GPIO would do. If you want to move your LED, change this number
// and move the wire - but stay off the pins the vehicle already uses for its
// motors, its LEDs and its servos. Lesson 1 has the table.
const int LED_PIN = 2;

// How long to wait, in milliseconds. 1000 milliseconds is one second.
const int BLINK_MS = 1000;

void setup() {
  // Tell the ESP32 that we intend to WRITE to this pin rather than read from
  // it. A pin has to be set to OUTPUT before digitalWrite() will do anything.
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);   // HIGH puts 3.3 volts on the pin: LED on
  delay(BLINK_MS);               // Do nothing at all for BLINK_MS milliseconds

  digitalWrite(LED_PIN, LOW);    // LOW puts 0 volts on the pin: LED off
  delay(BLINK_MS);

  // ...and then loop() starts again from the top, forever.
}
