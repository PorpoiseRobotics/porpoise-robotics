/*
  e03b_light_map
  Pathfinder Explorers - Lesson 3: Color Lab

  WHAT IT DOES
    Lights up the 32 lights ONE AT A TIME, in order, and prints each light's
    number on the Serial Monitor.

  YOUR MISSION
    Open the Serial Monitor (the magnifying glass, top right). Watch the
    vehicle and the numbers together, and fill in the light map in your
    Mission Log. Where is light 0? Where is 15? Where is 16? Where is 31?

    Too fast? Make WAIT bigger. Too slow? Make it smaller.

  Don't worry about HOW this program counts from 0 to 31. That is the
  next lesson!
*/

#include "explorer.h"

const int WAIT = 1500;    // How long each light stays on, in milliseconds

void setup() {
  startRover();
}

void loop() {
  for (int number = 0; number < 32; number++) {
    lightsOff();
    setLight(number, WHITE);
    Serial.print("Light number ");
    Serial.println(number);
    delay(WAIT);
  }
}
