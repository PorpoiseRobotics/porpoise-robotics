/*
  Config.h - a1a_tabs_and_config

  Every tunable number and every custom type lives here, so the rest of the
  program contains logic and nothing else. If you want to change how this
  sketch behaves, this is the only file you need.

  This is a real header file rather than another tab because the enum below has
  to be defined before anything uses it. Arduino writes function prototypes for
  you and puts them at the top of the sketch, which would land ABOVE a type
  declared in a tab; putting the types in a header that the main sketch
  includes first avoids that trap entirely.

  Pathfinder_Op_Program12 has a Config.h that opens with the same paragraph,
  for the same reason.
*/

#ifndef CONFIG_H
#define CONFIG_H

// The include guard above, and the #endif at the bottom, stop this file being
// pasted in twice if two tabs both include it. Every header needs one.

#include <Arduino.h>

// ===================================================================
// PINS
// ===================================================================

const int LED_PIN = 2;    // The small blue LED on the ESP32 module itself

// ===================================================================
// TIMING
// ===================================================================

const unsigned long BLINK_SLOW_MS       = 1000;
const unsigned long BLINK_FAST_MS       = 150;
const unsigned long MODE_SWITCH_MS      = 5000;   // How long each mode lasts
const unsigned long REPORT_INTERVAL_MS  = 1000;

// ===================================================================
// TYPES
// ===================================================================

// This enum is the whole reason Config.h exists. Try moving it into
// Blinker.ino and compiling - the error you get is worth seeing once.
enum BlinkMode { BLINK_SLOW, BLINK_FAST };

#endif  // CONFIG_H
