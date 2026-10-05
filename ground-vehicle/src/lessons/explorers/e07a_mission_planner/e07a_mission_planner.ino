/*
  e07a_mission_planner
  Pathfinder Explorers - Lesson 7: Mission Planner

  THE MISSION
    Your rover is on Mars. The radio signal from Earth takes MINUTES to
    arrive, so nobody can steer it live. It has to follow a plan.

    Drive from the START base to the SAMPLE zone, collect the sample (flash
    the lights), and come home again - without touching the crater.

  HOW TO PLAN IT
    1. Draw your route on the grid in your Mission Log first.
    2. Turn each piece of the route into one move, below.
    3. Test it. Fix it. Test it again. Real mission teams do this too.

  YOUR MISSION
    Level 1: Reach the SAMPLE zone and run collectSample().
    Level 2: Drive back home to START.
    Level 3: Invent a move of your own at the bottom - a zigzag, a victory
             spin, a "look around". Use it in your mission at least twice.

  HOW TO RUN IT
    Same as Lesson 6: upload wheels up, unplug, switch off, put it on
    START, switch on, step back.
*/

#include "explorer.h"

// ===== SETTINGS =====
const int SPEED_LIMIT  = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100
const int DRIVE_SPEED  = 60;   // How fast, in percent of top speed
const int QUARTER_TURN = 500;  // Copy your tuned TURN_TIME from Lesson 6!

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  setDriveSpeed(DRIVE_SPEED);
  countdown(5);

  // ===== YOUR MISSION PLAN: one move per line =====
  forward(1000);
  turnRight();
  forward(800);

  collectSample();

  // LEVEL 2: now drive back home to START

  // =================================================
  setAllLights(GREEN);    // Mission complete!
}

void loop() {
}

// ===== YOUR OWN MOVES =====
// A function is a new move that you invent. Write it once down here, then
// use it in your plan as many times as you like.

void turnRight() {
  spinRight(QUARTER_TURN);
}

void turnLeft() {
  spinLeft(QUARTER_TURN);
}

// Flash the lights purple three times: sample collected!
void collectSample() {
  for (int flash = 0; flash < 3; flash++) {
    setAllLights(PURPLE);
    delay(300);
    lightsOff();
    delay(300);
  }
}

// LEVEL 3: invent your own move here.
