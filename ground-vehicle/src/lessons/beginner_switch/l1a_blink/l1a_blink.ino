/*
  l1a_blink.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 1

  WHAT THIS PROGRAM DOES
  ----------------------
  Blinks the small blue LED that is built into the ESP32 module itself: one
  second on, one second off, forever. It does not touch the motors, the servos
  or the 32 LEDs around the vehicle, so it is completely safe to run with the
  vehicle sitting on the bench.

  This is the smallest useful program there is. If this uploads and blinks,
  then your computer, your USB cable, your board package and your board are all
  working, and everything else in the course is built on top of that.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    File > Preferences > "Additional boards manager URLs", add this line:
      https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
    Then Tools > Board > Boards Manager, search "bluepad32", and install it.
    Finally pick Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".

    WATCH OUT: you now have TWO "ESP32 Dev Module" entries in the board
    list, one under "esp32" and one under "esp32_bluepad32". Everything in
    the Switch track needs the esp32_bluepad32 one. If you get a pile of
    errors, check this first.
  Libraries: none.

  These are the same settings pathfinder_nintendoswitch.ino uses, so once you
  have set them up for this sketch you are set up for the whole Switch track.

  THE THREE PARTS OF EVERY ARDUINO PROGRAM
  ----------------------------------------
  1. The top of the file - libraries you want to use, and your constants and
     variables. This is read before anything else runs.
  2. setup()  - runs ONCE, the moment the board powers up or is reset.
  3. loop()   - runs over and over again, forever, until the power goes off.

  Every program in this course has that shape, including the 500-line one you
  will be driving by Lesson 5.

  WHAT TO TRY
  -----------
  1. Upload it as it is and watch the blue LED.
  2. Change BLINK_MS to 100 and upload again. What happens?
  3. Change it to 2000. Which number makes the LED blink FASTER - a bigger one
     or a smaller one? Why?
  4. Delete one of the two delay() lines and upload. Explain what you see.
     (Hint: the LED is still blinking. How fast?)
*/

// GPIO 2 is wired to the small blue LED on the ESP32 module. "GPIO" stands for
// General Purpose Input/Output: a pin the program can switch on and off.
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
