/*
  e04a_light_show
  Pathfinder Explorers - Lesson 4: Loops and Light Shows

  WHAT IT DOES
    Plays three light patterns, over and over: a chase that runs around the
    vehicle, police lights, and a scanner that sweeps like a robot's eye.

  YOUR MISSION
    Level 1: In loop(), change the colors. Change the order. Play a pattern
             twice. Make SPEED smaller to go faster, bigger to go slower.
    Level 2: In chase(), delete the line  lightsOff();  and upload. What
             happens now? Why? Then make the chase run BACKWARD, from 31
             down to 0.
    Level 3: Invent your own pattern. Draw it in your Mission Log first,
             then write it as a new for loop at the bottom of loop().

  REMEMBER
    Wheels up!
*/

#include "explorer.h"

const int SPEED = 60;    // Milliseconds between steps. Smaller is faster.

void setup() {
  startRover();
}

void loop() {
  chase(BLUE);
  chase(GREEN);
  policeLights(6);
  scanner(RED, 3);
}

// One light runs all the way around the vehicle.
void chase(Color color) {
  for (int number = 0; number < 32; number++) {
    lightsOff();
    setLight(number, color);
    delay(SPEED);
  }
}

// Left side red, then right side blue. "times" says how many flashes.
void policeLights(int times) {
  for (int count = 0; count < times; count++) {
    lightsOff();
    setLeftLights(RED);
    delay(200);
    lightsOff();
    setRightLights(BLUE);
    delay(200);
  }
}

// A dot sweeps across the front and back together, and bounces.
void scanner(Color color, int sweeps) {
  for (int sweep = 0; sweep < sweeps; sweep++) {
    for (int number = 0; number < 16; number++) {      // Left to right
      lightsOff();
      setLight(number, color);
      setLight(behind(number), color);
      delay(SPEED);
    }
    for (int number = 15; number >= 0; number--) {     // And back again
      lightsOff();
      setLight(number, color);
      setLight(behind(number), color);
      delay(SPEED);
    }
  }
}
