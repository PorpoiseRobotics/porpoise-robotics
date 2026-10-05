/*
  e09a_joystick_drive
  Pathfinder Explorers - Lesson 9: Joystick Driving

  WHAT IT DOES
    Drives the vehicle with the LEFT stick, using YOUR code. It also prints
    the stick numbers on the Serial Monitor, so you can see what the
    controller is really sending.

  YOUR MISSION
    Level 1 (wheels up): Open the Serial Monitor. Push the stick around and
             watch the numbers. Now LET GO. Is it exactly 0? Write down
             what you see.
    Level 2: Set WOBBLE_ZONE to 10 and upload. Let go of the stick again.
             Then try 60. What goes wrong when it is too big?
    Level 3 (floor): Drive the license course. Too twitchy when you turn?
             Remove the two slashes in front of  turn = turn / 2;

  SAFETY
    Wheels up until your numbers make sense. Then floor, inside the arena.
*/

#include "explorer.h"

// ===== SETTINGS =====
const int SPEED_LIMIT = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100
const int WOBBLE_ZONE = 0;    // Stick numbers smaller than this count as zero

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  startController();
}

void loop() {
  if (controllerReady() == false) {
    return;
  }

  int forward = leftStickY();    // Up is +100, down is -100
  int turn    = leftStickX();    // Right is +100, left is -100

  // The wobble zone: a stick you let go of is not always exactly 0
  if (abs(forward) < WOBBLE_ZONE) {
    forward = 0;
  }
  if (abs(turn) < WOBBLE_ZONE) {
    turn = 0;
  }

  // turn = turn / 2;            // LEVEL 3: gentler steering

  // Tank steering: to turn right, the left side goes faster than the right
  int leftSide  = forward + turn;
  int rightSide = forward - turn;
  drive(leftSide, rightSide);

  if (timeToPrint()) {
    Serial.print("forward ");
    Serial.print(forward);
    Serial.print("   turn ");
    Serial.print(turn);
    Serial.print("   left side ");
    Serial.print(leftSide);
    Serial.print("   right side ");
    Serial.println(rightSide);
  }
}
