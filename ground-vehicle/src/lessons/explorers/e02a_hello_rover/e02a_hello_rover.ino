/*
  e02a_hello_rover
  Pathfinder Explorers - Lesson 2: Talking to Robots

  WHAT IT DOES
    Turns on ONE light, and says hello on the Serial Monitor every 2 seconds.

  YOUR MISSION
    Level 1: Change the light number from 0 to 5. Which light comes on?
    Level 2: Change RED to another color: BLUE, GREEN, PURPLE, PINK...
             Then change the message so it says your team's name.
    Level 3: Make the light BLINK. In loop(), turn it on, wait, turn it
             off, wait.

  REMEMBER
    Wheels up! The vehicle sits on its stand whenever the cable is in.
*/

#include "explorer.h"

void setup() {
  // Everything in here happens ONCE, when the rover wakes up.
  startRover();
  setLight(0, RED);
}

void loop() {
  // Everything in here happens OVER AND OVER, forever.
  Serial.println("Hello! I am a Pathfinder rover.");
  delay(2000);     // Wait 2000 milliseconds. That is 2 seconds.
}
