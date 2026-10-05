/*
  e08a_button_lab
  Pathfinder Explorers - Lesson 8: Wireless Control

  WHAT IT DOES
    Your controller talks to the vehicle by radio. This program makes the
    buttons do things:
      Hold A    front lights green
      Hold B    back lights red
      Tap X     the controller buzzes
      Tap Y     a message on the Serial Monitor

    No driving yet - the wheels never move in this program.

  YOUR MISSION
    Level 1: Try every button. Then change the colors.
    Level 2: Find tappedY() below and change it to buttonY(). Open the
             Serial Monitor and HOLD Y. What is different? Why?
             (Change it back afterward.)
    Level 3: Make the D-pad work. dpadLeft() and dpadRight() could light
             up one side - like a turn signal. Then invent a SECRET COMBO:
             something special that only happens when A AND B are both
             held down. Two ampersands mean AND:
                 if (buttonA() && buttonB()) {
                 }
*/

#include "explorer.h"

void setup() {
  startRover();
  startController();
}

void loop() {
  if (controllerReady() == false) {
    return;     // No controller yet. The lights blink green while it looks.
  }

  if (buttonA()) {
    setFrontLights(GREEN);     // WHILE A is held down...
  } else {
    setFrontLights(OFF);       // ...and whenever it is not
  }

  if (buttonB()) {
    setBackLights(RED);
  } else {
    setBackLights(OFF);
  }

  if (tappedX()) {
    rumble(200);               // One buzz for each tap
  }

  if (tappedY()) {
    Serial.println("Y was pressed!");
  }

  // LEVEL 3: the D-pad and your secret combo go here
}
