/*
  e11a_my_upgrade
  Pathfinder Explorers - Lesson 11: Design Challenge

  WHAT IT DOES
    Everything you have built so far, in one program: joystick driving,
    smart lights with blinking turn signals, and a headlight switch on Y.

    Now it is YOUR turn to upgrade it. Your team picks an upgrade, plans
    it in the Mission Log, builds it, tests it, and makes it better.

  UPGRADE IDEAS
    Party mode       a button that turns on rainbow lights
    Hazard lights    a button that blinks BOTH sides, like a stopped car
    Crawl mode       hold ZL to drive at half speed, for careful parking
    Headlight flash  hold A to flash the headlights bright white
    Team light show  your own pattern when the rover wakes up, in setup()
    Victory dance    an autopilot move on a button - spins, wiggles, lights
    Your own idea    ask your teacher

  WHERE TO BUILD
    Look for the three UPGRADE ZONES below. Most upgrades need a
    true/false switch at the top, a few lines in the BUTTONS zone, and a
    few lines in the LIGHTS zone.

  REMEMBER
    Change ONE thing, then test it. Wheels up first, every time.
*/

#include "explorer.h"

// ===== SETTINGS =====
const int SPEED_LIMIT = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100
const int WOBBLE_ZONE = 10;   // Stick numbers smaller than this count as zero

bool headlightsOn = true;

// ===== UPGRADE ZONE 1: your true/false switches go here =====
// For example:   bool partyMode = false;

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  startController();

  // A team light show here plays once, when the rover wakes up.
}

void loop() {
  if (controllerReady() == false) {
    return;
  }

  // ===== DRIVING =====
  int forward = leftStickY();
  int turn    = leftStickX();

  if (abs(forward) < WOBBLE_ZONE) {
    forward = 0;
  }
  if (abs(turn) < WOBBLE_ZONE) {
    turn = 0;
  }

  drive(forward + turn, forward - turn);

  // ===== BUTTONS =====
  if (tappedY()) {
    headlightsOn = !headlightsOn;          // ! means "the opposite of"
  }

  // ===== UPGRADE ZONE 2: your buttons go here =====
  // For example:   if (tappedX()) {
  //                  partyMode = !partyMode;
  //                }

  // ===== SMART LIGHTS =====
  lightsOff();

  if (headlightsOn) {
    setFrontLights(WHITE);
  }

  if (forward > 0) {
    setBackLights(mixColor(60, 0, 0));     // Tail lights
  } else if (forward < 0) {
    setBackLights(WHITE);                  // Backup lights
  } else {
    setBackLights(RED);                    // Brake lights
  }

  if (turn > 0 && blinkIsOn()) {
    setRightLights(ORANGE);                // Turn signals
  }
  if (turn < 0 && blinkIsOn()) {
    setLeftLights(ORANGE);
  }

  // ===== UPGRADE ZONE 3: your lights go here =====
  // Lights drawn down here go on top of everything above.
  // For example:   if (partyMode) {
  //                  for (int number = 0; number < 32; number++) {
  //                    setLight(number, randomColor());
  //                  }
  //                }
}
