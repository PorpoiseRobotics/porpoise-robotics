/*
  e03a_color_lab
  Pathfinder Explorers - Lesson 3: Color Lab

  WHAT IT DOES
    Mixes red, green and blue light and shows the results on lights 0 to 6.

    mixColor(red, green, blue)
    Each number goes from 0 (none of that color) to 255 (all of it).

  YOUR MISSION
    Level 1: PREDICT first, then look. What color are lights 3, 4 and 5?
             Write your guesses in your Mission Log before you upload.
    Level 2: Mix your TEAM COLOR in teamColor below. Then remove the two
             slashes in front of setBackLights so the whole back glows.
    Level 3: Make a flag or a rainbow across the front lights, 0 to 15.
             setLights(first, last, color) paints a whole row at once.

  REMEMBER
    Wheels up! And don't stare right into the lights.
*/

#include "explorer.h"

// ===== YOUR COLORS =====
Color myColor   = mixColor(255, 0, 255);   // Red + blue. What do you get?
Color teamColor = mixColor(0, 0, 0);       // LEVEL 2: mix your team color here

void setup() {
  startRover();

  setLight(0, mixColor(255, 0, 0));       // Red only
  setLight(1, mixColor(0, 255, 0));       // Green only
  setLight(2, mixColor(0, 0, 255));       // Blue only
  setLight(3, mixColor(255, 255, 0));     // Red + green = ?
  setLight(4, mixColor(0, 255, 255));     // Green + blue = ?
  setLight(5, mixColor(255, 255, 255));   // All three = ?
  setLight(6, myColor);

  // setBackLights(teamColor);
}

void loop() {
  // Nothing to repeat. The colors stay on.
}
