/*
  e01a_learner_drive
  Pathfinder Explorers - Lesson 1: Meet Pathfinder

  This is the program your vehicle is running today. Don't worry about
  understanding it yet. By Lesson 10 you will have written most of it
  yourself.

  CONTROLS
    Left stick   Drive
    A            Flash the headlights (hold it), and buzz the controller
    Y            Lights on and off
    X            Party lights on and off

  THE ONE LINE TO LOOK AT TODAY
    Find SPEED_LIMIT just below. It is the reason your vehicle drives
    gently. When you pass your driving test, your teacher will let you
    raise it.

  SAFETY
    Drive on the floor, inside the arena. If anything goes wrong, let go
    of the stick. The vehicle stops by itself if the controller turns off.
*/

#include "explorer.h"

// ===== SETTINGS =====
const int SPEED_LIMIT = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100
const int WOBBLE_ZONE = 10;   // Stick numbers smaller than this count as zero

bool lightsOn  = true;
bool partyMode = false;

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  startController();
}

void loop() {
  if (controllerReady() == false) {
    return;      // No controller yet, so the motors stay stopped
  }

  // ----- Driving -----
  int forward = leftStickY();
  int turn    = leftStickX();

  if (abs(forward) < WOBBLE_ZONE) {
    forward = 0;
  }
  if (abs(turn) < WOBBLE_ZONE) {
    turn = 0;
  }

  drive(forward + turn, forward - turn);

  // ----- Buttons -----
  if (tappedY()) {
    lightsOn = !lightsOn;        // ! means "the opposite of"
  }
  if (tappedX()) {
    partyMode = !partyMode;
  }
  if (tappedA()) {
    rumble(300);
  }

  // ----- Lights -----
  lightsOff();

  if (partyMode) {
    for (int number = 0; number < 32; number++) {
      setLight(number, rainbowColor(number * 3 + millis() / 20));
    }
  } else if (lightsOn) {
    setFrontLights(mixColor(120, 120, 120));      // Headlights

    if (forward > 0) {
      setBackLights(mixColor(60, 0, 0));          // Tail lights
    } else if (forward < 0) {
      setBackLights(WHITE);                       // Backup lights
    } else {
      setBackLights(RED);                         // Brake lights
    }

    if (turn > 0 && blinkIsOn()) {
      setRightLights(ORANGE);                     // Turn signal
    }
    if (turn < 0 && blinkIsOn()) {
      setLeftLights(ORANGE);
    }
  }

  if (buttonA()) {
    setFrontLights(WHITE);                        // Flash the headlights
  }
}
