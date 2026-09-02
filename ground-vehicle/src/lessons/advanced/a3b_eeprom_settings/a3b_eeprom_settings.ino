/*
  a3b_eeprom_settings.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Stores settings that survive a power cycle. Change a value from the serial
  console, save it, pull the USB cable out, plug it back in, and the value is
  still there. Nothing moves.

  This is what makes Op Program 12 tunable at the track. Deadzones, scale
  factors and the paired controller address all live in EEPROM, so a vehicle
  keeps its personality without anybody opening the Arduino IDE.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none.

  COMMANDS (115200 baud)
  ----------------------
    show                     what is in RAM right now
    dump                     the raw bytes, as stored
    set deadzone <n>         0 to 511
    set scale <f>            0.2 to 1.2
    set name <text>          up to 15 characters
    save                     write RAM to EEPROM
    load                     read EEPROM back into RAM
    erase                    invalidate the stored copy
    defaults                 back to compiled-in values, without saving

  THERE IS NO EEPROM ON AN ESP32
  ------------------------------
  The EEPROM library is an emulation. It reserves a slice of the flash chip,
  keeps a copy of it in RAM, and only writes the flash when you call commit().
  Three things follow:

    - EEPROM.begin(size) must come first, before any get or put.
    - Nothing is actually stored until EEPROM.commit(). Forgetting it is the
      single most common cause of "my settings did not save".
    - Flash wears out - a hundred thousand erase cycles or so per sector. That
      is plenty for saving when a human asks, and nowhere near enough to
      commit() inside loop(). Never write on a schedule.

  ADDRESSES ARE MANUAL, AND EASY TO GET WRONG
  -------------------------------------------
  EEPROM is a flat array of bytes. You choose where each value lives, and
  nothing stops two values overlapping. An int is 4 bytes and a float is 4
  bytes on this chip, so the map below leaves 4 bytes per slot and the string
  gets 16. Op 12 lays its Config.h out the same way, one ADDR_ constant per
  value, which is what makes an overlap visible when you read the file.

  THE MAGIC BYTE
  --------------
  A brand new ESP32 has flash full of 0xFF. Read an int out of that and you get
  -1; read a float and you get something meaningless. So the first byte of the
  block is a marker written only after a successful save. If it is not there,
  the stored data is not ours and we use the compiled-in defaults instead.

  Even with the marker present, every value is range checked as it is loaded.
  A saved value from an older version of the program can be valid data and
  still be wrong for this one. Op 12 does exactly this in loadSettings().

  WHAT TO TRY
  -----------
  1. "set deadzone 100", "save", then unplug and replug the board. "show".
  2. Now "set deadzone 200" and DO NOT save. Reset the board. What happened?
  3. "erase", then reset. Where did the values come from this time?
  4. "dump" before and after a save, and find the magic byte.
  5. Comment out the EEPROM.commit() in saveSettings, save, and reset. This is
     the bug you will write one day - see it now instead.
*/

#include <EEPROM.h>

// ===================================================================
// THE STORAGE MAP
// ===================================================================

const int EEPROM_SIZE = 64;

const int ADDR_VALID    =  0;   // 1 byte
const int ADDR_DEADZONE =  4;   // 4 bytes, int
const int ADDR_SCALE    =  8;   // 4 bytes, float
const int ADDR_NAME     = 16;   // 16 bytes, text plus a terminator

const uint8_t VALID_MAGIC = 0xA5;   // Same marker Op 12 uses

const int NAME_LEN = 15;

// ===================================================================
// COMPILED-IN DEFAULTS
// ===================================================================

const int   DEFAULT_DEADZONE = 32;
const float DEFAULT_SCALE    = 1.0f;
const char  DEFAULT_NAME[]   = "pathfinder";

const int   DEADZONE_MIN = 0,    DEADZONE_MAX = 511;
const float SCALE_MIN    = 0.2f, SCALE_MAX    = 1.2f;

// ===================================================================
// LIVE VALUES
// ===================================================================

int   input_deadzone = DEFAULT_DEADZONE;
float scale_factor   = DEFAULT_SCALE;
char  vehicle_name[NAME_LEN + 1] = { 0 };

String input_line = "";

// ===================================================================
// SAVE AND LOAD
// ===================================================================

void saveSettings() {
  EEPROM.write(ADDR_VALID, VALID_MAGIC);
  EEPROM.put(ADDR_DEADZONE, input_deadzone);
  EEPROM.put(ADDR_SCALE, scale_factor);

  for (int i = 0; i <= NAME_LEN; i++) {
    EEPROM.write(ADDR_NAME + i, vehicle_name[i]);
  }

  // Without this line nothing reaches the flash at all.
  EEPROM.commit();

  Serial.println(F("Saved."));
}

/*
  Reads everything back, keeping the compiled-in default for anything that is
  missing or out of range. Never trust what comes out of storage.
*/
void loadSettings() {
  if (EEPROM.read(ADDR_VALID) != VALID_MAGIC) {
    Serial.println(F("No valid saved settings. Using defaults."));
    useDefaults();
    return;
  }

  int   saved_deadzone;
  float saved_scale;
  EEPROM.get(ADDR_DEADZONE, saved_deadzone);
  EEPROM.get(ADDR_SCALE, saved_scale);

  if (saved_deadzone >= DEADZONE_MIN && saved_deadzone <= DEADZONE_MAX) {
    input_deadzone = saved_deadzone;
  } else {
    Serial.println(F("Stored deadzone out of range, keeping the default."));
  }

  if (saved_scale >= SCALE_MIN && saved_scale <= SCALE_MAX) {
    scale_factor = saved_scale;
  } else {
    Serial.println(F("Stored scale out of range, keeping the default."));
  }

  for (int i = 0; i <= NAME_LEN; i++) {
    vehicle_name[i] = (char)EEPROM.read(ADDR_NAME + i);
  }
  vehicle_name[NAME_LEN] = '\0';   // Never trust stored text to be terminated

  Serial.println(F("Loaded from EEPROM."));
}

void useDefaults() {
  input_deadzone = DEFAULT_DEADZONE;
  scale_factor   = DEFAULT_SCALE;
  strncpy(vehicle_name, DEFAULT_NAME, NAME_LEN);
  vehicle_name[NAME_LEN] = '\0';
}

void eraseSettings() {
  EEPROM.write(ADDR_VALID, 0x00);
  EEPROM.commit();
  Serial.println(F("Marker cleared. The next boot will use defaults."));
}

// ===================================================================
// OUTPUT
// ===================================================================

void showSettings() {
  Serial.printf("deadzone = %d\n", input_deadzone);
  Serial.printf("scale    = %.2f\n", scale_factor);
  Serial.printf("name     = %s\n", vehicle_name);
}

void dumpEeprom() {
  Serial.println(F("addr  bytes"));
  for (int row = 0; row < EEPROM_SIZE; row += 8) {
    Serial.printf("%3d   ", row);
    for (int i = 0; i < 8; i++) {
      Serial.printf("%02X ", EEPROM.read(row + i));
    }
    Serial.println();
  }
}

// ===================================================================
// SETUP AND LOOP
// ===================================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  useDefaults();

  // Must come before any get or put.
  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  Serial.println();
  Serial.println(F("=== EEPROM settings ==="));
  showSettings();
  Serial.println(F("Type 'help'."));
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (input_line.length() > 0) {
        runCommand(input_line);
        input_line = "";
      }
    } else if (c >= 32 && c < 127 && input_line.length() < 60) {
      input_line += c;
    }
  }
}

void runCommand(String line) {
  line.trim();
  String lower = line;
  lower.toLowerCase();

  if (lower == "help") {
    Serial.println(F("  show                 values in RAM"));
    Serial.println(F("  dump                 raw EEPROM bytes"));
    Serial.println(F("  set deadzone <n>     0 to 511"));
    Serial.println(F("  set scale <f>        0.2 to 1.2"));
    Serial.println(F("  set name <text>      up to 15 characters"));
    Serial.println(F("  save / load / erase / defaults"));

  } else if (lower == "show") {
    showSettings();

  } else if (lower == "dump") {
    dumpEeprom();

  } else if (lower == "save") {
    saveSettings();

  } else if (lower == "load") {
    loadSettings();
    showSettings();

  } else if (lower == "erase") {
    eraseSettings();

  } else if (lower == "defaults") {
    useDefaults();
    Serial.println(F("Back to compiled-in defaults. Not saved yet."));
    showSettings();

  } else if (lower.startsWith("set deadzone ")) {
    long value = lower.substring(13).toInt();
    if (value >= DEADZONE_MIN && value <= DEADZONE_MAX) {
      input_deadzone = (int)value;
      Serial.printf("deadzone = %d  (not saved yet)\n", input_deadzone);
    } else {
      Serial.printf("deadzone must be %d to %d\n", DEADZONE_MIN, DEADZONE_MAX);
    }

  } else if (lower.startsWith("set scale ")) {
    float value = lower.substring(10).toFloat();
    if (value >= SCALE_MIN && value <= SCALE_MAX) {
      scale_factor = value;
      Serial.printf("scale = %.2f  (not saved yet)\n", scale_factor);
    } else {
      Serial.printf("scale must be %.1f to %.1f\n", SCALE_MIN, SCALE_MAX);
    }

  } else if (lower.startsWith("set name ")) {
    String text = line.substring(9);
    text.trim();
    if (text.length() == 0 || text.length() > NAME_LEN) {
      Serial.printf("name must be 1 to %d characters\n", NAME_LEN);
      return;
    }
    strncpy(vehicle_name, text.c_str(), NAME_LEN);
    vehicle_name[NAME_LEN] = '\0';
    Serial.printf("name = %s  (not saved yet)\n", vehicle_name);

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
