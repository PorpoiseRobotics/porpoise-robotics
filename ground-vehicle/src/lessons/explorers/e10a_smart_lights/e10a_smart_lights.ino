/*
  e10a_smart_lights
  Pathfinder Explorers - Lesson 10: Smart Lights

  WHAT IT DOES
    Your joystick driving from Lesson 9, plus lights that tell everyone
    what the vehicle is doing - just like a real car:
      Headlights      white, at the front, all the time
      Tail lights     dim red at the back while driving forward
      Brake lights    bright red at the back when stopped
      Backup lights   white at the back when reversing

  YOUR MISSION
    Level 1: Drive it (wheels up first). Do the back lights change at the
             right moments?
    Level 2: TURN SIGNALS. Find the turn signal lines near the bottom and
             remove the two slashes at the start of each one.
    Level 3: Make the turn signals BLINK. Change  if (turn > 0)  to
                 if (turn > 0 && blinkIsOn())
             and do the same for the left side. Then add a HEADLIGHT
             SWITCH: make tappedY() turn the headlights on and off.
             (Look at how lightsOn works in e01a_learner_drive.)
*/

#include "explorer.h"

// ===== SETTINGS =====
const int SPEED_LIMIT = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100
const int WOBBLE_ZONE = 10;   // Stick numbers smaller than this count as zero

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  startController();
}

void loop() {
  if (controllerReady() == false) {
    return;
  }

  // ===== DRIVING (from Lesson 9) =====
  int forward = leftStickY();
  int turn    = leftStickX();

  if (abs(forward) < WOBBLE_ZONE) {
    forward = 0;
  }
  if (abs(turn) < WOBBLE_ZONE) {
    turn = 0;
  }

  drive(forward + turn, forward - turn);

  // ===== SMART LIGHTS =====
  lightsOff();                              // Start a fresh picture each time

  setFrontLights(WHITE);                    // Headlights

  if (forward > 0) {
    setBackLights(mixColor(60, 0, 0));      // Driving forward: dim tail lights
  } else if (forward < 0) {
    setBackLights(WHITE);                   // Reversing: backup lights
  } else {
    setBackLights(RED);                     // Stopped: bright brake lights
  }

  // LEVEL 2: turn signals
  // if (turn > 0) {
  //   setRightLights(ORANGE);
  // }
  // if (turn < 0) {
  //   setLeftLights(ORANGE);
  // }
}
