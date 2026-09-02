/*
  l1b_serial_monitor.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 1

  WHAT THIS PROGRAM DOES
  ----------------------
  Sends words and numbers back up the USB cable to your computer, where you can
  read them in the Arduino IDE Serial Monitor. Nothing on the vehicle moves.

  This is how you find out what a robot is THINKING. When the vehicle does
  something you did not expect, printing the numbers it is working from is
  almost always the fastest way to find out why. You will use the Serial
  Monitor in every lesson from here on.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32" by Espressif Systems, VERSION 3.0.7
    Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  Libraries: none.

  HOW TO SEE THE OUTPUT
  ---------------------
  Upload the program, then open Tools > Serial Monitor (or click the magnifying
  glass at the top right). At the bottom of the Serial Monitor window there is
  a baud rate box. It MUST say 115200, because that is the number in
  Serial.begin() below. If you see rows of nonsense characters, that box is
  the first thing to check.

  WHAT TO TRY
  -----------
  1. Upload and open the Serial Monitor. Press the EN (reset) button on the
     ESP32 and watch the greeting print again from the start.
  2. Change the greeting to your own name and upload again.
  3. Change the two numbers in the arithmetic section and predict the answers
     BEFORE you upload. Were you right about 7 / 2?
  4. Add a line of your own that prints something every time round the loop.
*/

// A variable is a named box that holds a value which is allowed to change.
// The word in front of the name says what KIND of value fits in the box.
int   loopCount    = 0;      // int    - a whole number
float batteryVolts = 16.8;   // float  - a number with a decimal point
bool  headlightsOn = true;   // bool   - only ever true or false

void setup() {
  // Open the serial connection at 115200 bits per second. Both ends have to
  // agree on this number or the letters arrive scrambled.
  Serial.begin(115200);
  delay(500);   // Give the USB connection a moment to wake up

  Serial.println();
  Serial.println("=== Pathfinder Lesson 1 ===");
  Serial.println("Hello from the ESP32!");
  Serial.println();

  // println() prints and then moves to a new line. print() stays on the line,
  // which lets you build up one line out of several pieces.
  Serial.print("Battery: ");
  Serial.print(batteryVolts);
  Serial.println(" volts");

  Serial.print("Headlights on? ");
  Serial.println(headlightsOn);   // A bool prints as 1 for true, 0 for false
  Serial.println();

  // --- Arithmetic, and one trap worth meeting early ---
  Serial.println("--- Maths ---");
  Serial.print("7 + 2 = ");  Serial.println(7 + 2);
  Serial.print("7 - 2 = ");  Serial.println(7 - 2);
  Serial.print("7 * 2 = ");  Serial.println(7 * 2);

  // Watch this one. Both 7 and 2 are whole numbers, so the ESP32 does whole
  // number division and throws the remainder away. It does NOT round.
  Serial.print("7 / 2 = ");  Serial.println(7 / 2);

  // Writing one of them with a decimal point asks for decimal division.
  Serial.print("7.0 / 2 = ");  Serial.println(7.0 / 2);

  // The % operator gives you the remainder that the first division threw away.
  Serial.print("7 % 2 = ");  Serial.println(7 % 2);
  Serial.println();

  Serial.println("Now watch the loop count. One line every second.");
}

void loop() {
  loopCount = loopCount + 1;   // Add one to whatever is in the box already

  Serial.print("Loop number ");
  Serial.print(loopCount);
  Serial.print("   -   the board has been on for ");
  Serial.print(millis() / 1000);   // millis() counts milliseconds since power-up
  Serial.println(" seconds");

  delay(1000);
}
