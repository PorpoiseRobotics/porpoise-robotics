/*
  e12a_mission_day
  Pathfinder Explorers - Lesson 12: Mission Day

  THE BACKUP PROGRAM
    Everything the course taught, working, in one program. Use your OWN
    program from Lesson 11 on Mission Day if it works. If it doesn't,
    upload this one - no shame in it. Real mission teams always carry a
    backup.

  CONTROLS
    Left stick   Drive
    Y            Headlights on and off
    A            Flash the headlights (hold it)
    ZL           Crawl mode: half speed while you hold it, for parking
    X            AUTOPILOT: drives the route you planned, all by itself
    B            While autopilot is running: STOP. B is for Brake.

  THE AUTOPILOT ROUTE
    On Mission Day there is a NO-SIGNAL ZONE. Your vehicle has to cross it
    on autopilot. Measure the zone, then fill in the route in runAutopilot()
    at the bottom. Test it, fix it, test it again.
*/

#include "explorer.h"

// ===== SETTINGS =====
const int SPEED_LIMIT  = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100
const int WOBBLE_ZONE  = 10;   // Stick numbers smaller than this count as zero
const int DRIVE_SPEED  = 60;   // Autopilot speed, in percent of top speed
const int QUARTER_TURN = 500;  // Your tuned quarter turn from Lesson 6

bool headlightsOn = true;

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  setDriveSpeed(DRIVE_SPEED);
  startController();
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

  if (buttonZL()) {
    forward = forward / 2;           // Crawl mode
    turn = turn / 2;
  }

  drive(forward + turn, forward - turn);

  // ===== BUTTONS =====
  if (tappedY()) {
    headlightsOn = !headlightsOn;
  }

  if (tappedX()) {
    runAutopilot();
  }

  // ===== SMART LIGHTS =====
  lightsOff();

  if (headlightsOn) {
    setFrontLights(mixColor(120, 120, 120));
  }
  if (buttonA()) {
    setFrontLights(WHITE);           // Headlight flash
  }

  if (forward > 0) {
    setBackLights(mixColor(60, 0, 0));
  } else if (forward < 0) {
    setBackLights(WHITE);
  } else {
    setBackLights(RED);
  }

  if (turn > 0 && blinkIsOn()) {
    setRightLights(ORANGE);
  }
  if (turn < 0 && blinkIsOn()) {
    setLeftLights(ORANGE);
  }
}

// ===== THE AUTOPILOT ROUTE =====
// Crossing the no-signal zone. Change these moves to fit the course.
void runAutopilot() {
  Serial.println("AUTOPILOT ON. Hold B to stop.");
  setAllLights(CYAN);
  pause(1000);

  forward(1500);
  spinLeft(QUARTER_TURN);
  forward(1000);
  spinRight(QUARTER_TURN);
  forward(1000);

  stopMotors();
  Serial.println("Autopilot finished. You have control.");
}
