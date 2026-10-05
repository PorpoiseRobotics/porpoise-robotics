/*
  e05a_motor_lab
  Pathfinder Explorers - Lesson 5: Make It Move

  WHAT IT DOES
    Runs the wheels through a test, over and over: left side, right side,
    both forward, both backward, a spin, then a rest.

    drive(leftSpeed, rightSpeed)
    Speeds go from -100 to 100. Plus is forward, minus is backward.

  SAFETY - WHEELS UP FOR THIS WHOLE LESSON
    The vehicle stays on its stand, wheels in the air, the entire time.
    Hands, hair and cables away from the wheels. The Crew Chief keeps a
    finger near the power switch.

  YOUR MISSION
    Level 1: PREDICT each step before it happens. Were you right?
    Level 2: Find the WAKE-UP SPEED - the smallest number that still makes
             the wheels turn. Change the 50s in the first step to 30, then
             20, then 10... Write your answer in your Mission Log.
    Level 3: Write a WHEEL DANCE. Change the steps in loop() into your own
             routine. Make one side go forward while the other goes back.
*/

#include "explorer.h"

const int SPEED_LIMIT = 50;   // LEARNER 50  -  LICENSED 75  -  EXPERT 100

void setup() {
  startRover();
  setSpeedLimit(SPEED_LIMIT);
  Serial.println("WHEELS UP? The wheels start turning after the countdown.");
  countdown(5);
}

void loop() {
  Serial.println("Left side forward");
  drive(50, 0);
  delay(2000);

  Serial.println("Right side forward");
  drive(0, 50);
  delay(2000);

  Serial.println("Both sides forward");
  drive(50, 50);
  delay(2000);

  Serial.println("Both sides BACKWARD");
  drive(-50, -50);
  delay(2000);

  Serial.println("Spin: left forward, right backward");
  drive(50, -50);
  delay(2000);

  Serial.println("Stop and rest");
  stopMotors();
  delay(3000);
}
