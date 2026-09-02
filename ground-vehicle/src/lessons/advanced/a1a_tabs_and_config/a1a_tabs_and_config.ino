/*
  a1a_tabs_and_config.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 1

  WHAT THIS PROGRAM DOES
  ----------------------
  Almost nothing, on purpose. It blinks the on-board LED and prints a line
  every second. What matters is its SHAPE: it is split across three files, the
  same way Pathfinder_Op_Program12 is split across eight.

        a1a_tabs_and_config.ino   setup(), loop(), and the shared variables
        Config.h                  every tunable number and every custom type
        Blinker.ino               one subsystem, in its own tab

  Nothing moves. Safe on the bench.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    File > Preferences > "Additional boards manager URLs", add:
      https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
    Then Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none.

  These are the same settings Pathfinder_Op_Program12 uses.

  HOW ARDUINO TABS ACTUALLY WORK
  ------------------------------
  Every .ino file in the sketch folder shows up as a tab in the IDE. Before
  compiling, the IDE concatenates them - the one named after the folder first,
  then the rest in alphabetical order - into a single translation unit, and
  generates function prototypes for you at the top.

  Two consequences you have to know:

    - Functions and globals are shared across tabs with no header needed. That
      is why Blinker.ino can call Serial and read BLINK_MS without including
      anything.

    - TYPES are not. The IDE puts its generated prototypes ABOVE your code, so
      a prototype that mentions an enum or struct you declared in a tab will
      not compile - the type is not defined yet at that point.

  That is exactly why Config.h is a real header file and not a fourth tab. A
  header is #included by the main sketch, so anything in it is defined before
  the generated prototypes appear. Op Program 12 does the same thing for the
  same reason, and it says so at the top of its own Config.h.

  WHY BOTHER SPLITTING A PROGRAM UP
  ---------------------------------
  Pathfinder_Op_Program11.2 was one file of 1292 lines. Finding the lighting
  code in it meant scrolling. Op 12 is the same program split by subsystem, so
  "where does pairing happen" has a one-word answer: Bluetooth.

  The rule that makes it work: each tab owns one subsystem, and every number
  you might want to change lives in Config.h rather than being buried in the
  middle of the logic.

  WHAT TO TRY
  -----------
  1. Look at the tab bar at the top of the IDE. Click between the three files.
  2. Change BLINK_MS in Config.h. Notice you did not touch either .ino file.
  3. Add a fourth file called Reporter.ino with a function in it, and call that
     function from loop(). Notice that you did not have to declare it anywhere.
  4. Now move the BlinkMode enum out of Config.h and into Blinker.ino, and try
     to compile. Read the error. Put it back.
  5. Open Pathfinder_Op_Program12 and list what each of its eight files owns.
*/

#include "Config.h"

// ===================================================================
// SHARED STATE
// ===================================================================
// Globals declared here are visible from every tab in this sketch.

BlinkMode blink_mode = BLINK_SLOW;
unsigned long last_report = 0;

// ===================================================================
// SETUP
// ===================================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  setup_blinker();          // Lives in Blinker.ino. No declaration needed.

  Serial.println();
  Serial.println(F("=== Tabs and Config ==="));
  Serial.println(F("Three files, one program."));

  // F() keeps a string literal in flash instead of copying it into RAM at
  // boot. On a program this small it makes no difference. On Op 12, with its
  // Bluetooth stack already using most of the RAM, it makes a real one, which
  // is why you will see F() all over that program and should get in the habit.
}

// ===================================================================
// LOOP
// ===================================================================

void loop() {
  unsigned long now = millis();

  update_blinker(now);      // Also lives in Blinker.ino

  if (now - last_report >= REPORT_INTERVAL_MS) {
    last_report = now;
    Serial.printf("up %lu s, mode %s\n",
                  now / 1000,
                  blink_mode == BLINK_SLOW ? "SLOW" : "FAST");
  }
}
