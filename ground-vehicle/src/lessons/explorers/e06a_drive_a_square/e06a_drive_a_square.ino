/*
  e06a_drive_a_square
  Pathfinder Explorers - Lesson 6: Drive by Code

  WHAT IT DOES
    Drives a square ALL BY ITSELF - no controller. Forward, quarter turn,
    four times. Then it stops and turns its lights green.

    This is called AUTOPILOT. The vehicle can't see where it is going. It
    just does what you told it, for exactly as long as you told it.

  HOW TO RUN IT
    1. Upload it with the wheels up. The wheels will turn on the stand.
       That's fine - it is practicing.
    2. Unplug the cable. Switch the vehicle OFF.
    3. Put it on the START mark in the arena, pointing along the tape.
    4. Switch it ON and step back. You have 5 seconds.
    5. To run it again: switch OFF, put it back on START, switch ON.

  YOUR MISSION
    Level 1: Does it come back to START? Probably not! Change TURN_TIME
             until each corner is a quarter turn. Change it by 50 at a time.
    Level 2: Make the square bigger or smaller. Which number do you change?
    Level 3: Your setup() repeats the same two lines four times. Use a
             for loop instead, like in Lesson 4. Then drive a TRIANGLE.
*/

#include "explorer.h"

// ===== SETTINGS: TUNE THESE =====
const int SPEED_LIMIT = 50;    // LEARNER 50  -  LICENSED 75  -  EXPERT 100
const int DRIVE_SPEED = 60;    // How fast, in percent of top speed
const int SIDE_TIME   = 1500;  // How long to drive each side, in milliseconds
const int TURN_TIME   = 500;   // How long a quarter turn takes. TUNE ME!

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  setDriveSpeed(DRIVE_SPEED);

  countdown(5);                // 5 seconds to step back

  // The mission happens ONCE, so it goes in setup().
  forward(SIDE_TIME);
  spinRight(TURN_TIME);
  forward(SIDE_TIME);
  spinRight(TURN_TIME);
  forward(SIDE_TIME);
  spinRight(TURN_TIME);
  forward(SIDE_TIME);
  spinRight(TURN_TIME);

  setAllLights(GREEN);         // Mission complete!
}

void loop() {
  // Nothing to repeat. Switch off, back to START, switch on to run again.
}
