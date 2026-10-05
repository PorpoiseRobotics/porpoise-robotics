/*
  e02b_bug_hunt
  Pathfinder Explorers - Lesson 2: Talking to Robots

  THIS PROGRAM HAS THREE BUGS. They are on purpose. It will not upload
  until you have fixed all three.

  HOW TO HUNT
    1. Click the CHECK MARK button (Verify). It checks your code without
       uploading it.
    2. Read the message at the bottom of the window. It tells you the LINE
       NUMBER, and it often guesses what you meant.
    3. Fix that one bug. Click the check mark again.
    4. When it says "Done compiling", upload it. Lights 0, 1 and 2 come on.

  HINTS
    Every instruction ends with a semicolon  ;
    Capital letters matter. setLight is not the same as setlight.
    Colors have to be spelled the way the computer knows them.
*/

#include "explorer.h"

void setup() {
  startRover();
  setLight(0, GREEN)
  setlight(1, YELLOW);
  setLight(2, BLEU);
}

void loop() {
  Serial.println("Bug hunt complete!");
  delay(2000);
}
