/*
  Console.ino - the serial command interface, the stored settings behind it,
  and the live diagnostics.

  Open the Serial Monitor at 115200 baud and type "help".

  Everything set here is written to EEPROM, so a vehicle keeps its tuning
  across a power cycle. That is what makes it possible to tune a vehicle at
  the track without a laptop full of toolchains.
*/

// ===================================================================
// STORED SETTINGS
// ===================================================================

void saveSettings() {
  EEPROM.put(ADDR_DEADZONE,      INPUT_DEADZONE);
  EEPROM.put(ADDR_DEADZONE_LS_X, INPUT_DEADZONE_LS_X);
  EEPROM.put(ADDR_DEADZONE_LS_Y, INPUT_DEADZONE_LS_Y);
  EEPROM.put(ADDR_DEADZONE_RS_X, INPUT_DEADZONE_RS_X);
  EEPROM.put(ADDR_DEADZONE_RS_Y, INPUT_DEADZONE_RS_Y);
  EEPROM.put(ADDR_LS_Y_SC, ls_y_sc);
  EEPROM.put(ADDR_LS_X_SC, ls_x_sc);
  EEPROM.put(ADDR_RS_Y_SC, rs_y_sc);
  EEPROM.put(ADDR_RS_X_SC, rs_x_sc);
  EEPROM.commit();
}

/*
  Reads everything back, keeping the compiled-in default for any value that is
  out of range. A brand new ESP32 has EEPROM full of 0xFF, which reads back as
  -1 for an int and as nonsense for a float, so every value is range checked
  rather than trusted.
*/
void loadSettings() {
  int   saved_dz, saved_lsx_dz, saved_lsy_dz, saved_rsx_dz, saved_rsy_dz;
  float saved_lsy_sc, saved_lsx_sc, saved_rsy_sc, saved_rsx_sc;

  EEPROM.get(ADDR_DEADZONE,      saved_dz);
  EEPROM.get(ADDR_DEADZONE_LS_X, saved_lsx_dz);
  EEPROM.get(ADDR_DEADZONE_LS_Y, saved_lsy_dz);
  EEPROM.get(ADDR_DEADZONE_RS_X, saved_rsx_dz);
  EEPROM.get(ADDR_DEADZONE_RS_Y, saved_rsy_dz);
  EEPROM.get(ADDR_LS_Y_SC, saved_lsy_sc);
  EEPROM.get(ADDR_LS_X_SC, saved_lsx_sc);
  EEPROM.get(ADDR_RS_Y_SC, saved_rsy_sc);
  EEPROM.get(ADDR_RS_X_SC, saved_rsx_sc);

  if (saved_dz >= 0 && saved_dz <= DEADZONE_MAX) INPUT_DEADZONE = saved_dz;

  if (saved_lsx_dz >= DEADZONE_MIN && saved_lsx_dz <= DEADZONE_MAX) INPUT_DEADZONE_LS_X = saved_lsx_dz;
  if (saved_lsy_dz >= DEADZONE_MIN && saved_lsy_dz <= DEADZONE_MAX) INPUT_DEADZONE_LS_Y = saved_lsy_dz;
  if (saved_rsx_dz >= DEADZONE_MIN && saved_rsx_dz <= DEADZONE_MAX) INPUT_DEADZONE_RS_X = saved_rsx_dz;
  if (saved_rsy_dz >= DEADZONE_MIN && saved_rsy_dz <= DEADZONE_MAX) INPUT_DEADZONE_RS_Y = saved_rsy_dz;

  ls_y_sc = (saved_lsy_sc >= SCALE_MIN && saved_lsy_sc <= SCALE_MAX) ? saved_lsy_sc : 1.0f;
  ls_x_sc = (saved_lsx_sc >= SCALE_MIN && saved_lsx_sc <= SCALE_MAX) ? saved_lsx_sc : 1.0f;
  rs_y_sc = (saved_rsy_sc >= SCALE_MIN && saved_rsy_sc <= SCALE_MAX) ? saved_rsy_sc : 1.0f;
  rs_x_sc = (saved_rsx_sc >= SCALE_MIN && saved_rsx_sc <= SCALE_MAX) ? saved_rsx_sc : 1.0f;

  // Always start in NORMAL mode, whatever the saved scales say.
  throttle_mode  = MODE_NORMAL;
  active_ls_y_sc = NORMAL_MODE_Y_SCALE;
  active_ls_x_sc = NORMAL_MODE_X_SCALE;
}

// ===================================================================
// COMMANDS
// ===================================================================

void printHelp() {
  Serial.println(F("\nAvailable commands:"));
  Serial.println(F("  help      - show this help"));
  Serial.println(F("  pair      - forget this controller and pair a new one"));
  Serial.println(F("  forget    - same as pair"));
  Serial.println(F("  diag      - toggle live diagnostics"));
  Serial.println(F("  dynamics  - show the current input settings"));
  Serial.println(F("  dynamics [param value]... - change them"));
  Serial.println(F("      params: ls_y_dz, ls_y_sc, ls_x_dz, ls_x_sc,"));
  Serial.println(F("              rs_y_dz, rs_y_sc, rs_x_dz, rs_x_sc"));
  Serial.printf ("      dz: %d to %d (%d means use the global deadzone)\n",
                 DEADZONE_MIN, DEADZONE_MAX, DEADZONE_MIN);
  Serial.printf ("      sc: %.1f to %.1f\n", SCALE_MIN, SCALE_MAX);
  Serial.println(F("      example: dynamics ls_y_dz 30 ls_x_sc 1.1"));
}

void printDynamics() {
  Serial.println(F("Current input settings:"));
  Serial.printf("  Global deadzone: %d\n", INPUT_DEADZONE);
  Serial.printf("  LS_Y: dz=%d sc=%.2f\n", INPUT_DEADZONE_LS_Y, ls_y_sc);
  Serial.printf("  LS_X: dz=%d sc=%.2f\n", INPUT_DEADZONE_LS_X, ls_x_sc);
  Serial.printf("  RS_Y: dz=%d sc=%.2f\n", INPUT_DEADZONE_RS_Y, rs_y_sc);
  Serial.printf("  RS_X: dz=%d sc=%.2f\n", INPUT_DEADZONE_RS_X, rs_x_sc);
  Serial.printf("  Throttle mode: %s\n", throttle_mode == MODE_FAST ? "FAST" : "NORMAL");
}

// Applies one "name value" pair. Returns true if it was understood and valid.
bool applyDynamicsSetting(const String &key, const String &valStr) {
  // Deadzones are whole numbers.
  int  *dz_target = nullptr;
  if      (key == "ls_y_dz") dz_target = &INPUT_DEADZONE_LS_Y;
  else if (key == "ls_x_dz") dz_target = &INPUT_DEADZONE_LS_X;
  else if (key == "rs_y_dz") dz_target = &INPUT_DEADZONE_RS_Y;
  else if (key == "rs_x_dz") dz_target = &INPUT_DEADZONE_RS_X;

  if (dz_target != nullptr) {
    int v = valStr.toInt();
    if (v < DEADZONE_MIN || v > DEADZONE_MAX) {
      Serial.printf("  %s must be between %d and %d\n", key.c_str(), DEADZONE_MIN, DEADZONE_MAX);
      return false;
    }
    *dz_target = v;
    return true;
  }

  // Scale factors are decimals.
  float *sc_target = nullptr;
  if      (key == "ls_y_sc") sc_target = &ls_y_sc;
  else if (key == "ls_x_sc") sc_target = &ls_x_sc;
  else if (key == "rs_y_sc") sc_target = &rs_y_sc;
  else if (key == "rs_x_sc") sc_target = &rs_x_sc;

  if (sc_target != nullptr) {
    float v = valStr.toFloat();
    if (v < SCALE_MIN || v > SCALE_MAX) {
      Serial.printf("  %s must be between %.1f and %.1f\n", key.c_str(), SCALE_MIN, SCALE_MAX);
      return false;
    }
    *sc_target = v;
    // A scale change should be felt straight away in FAST mode.
    if (throttle_mode == MODE_FAST) {
      active_ls_y_sc = ls_y_sc;
      active_ls_x_sc = ls_x_sc;
    }
    return true;
  }

  Serial.printf("  Unknown setting: %s\n", key.c_str());
  return false;
}

/*
  Handles "dynamics" with no arguments (show everything) or with any number of
  name/value pairs after it.
*/
void handleDynamicsCommand(const String &args) {
  String remaining = args;
  remaining.trim();

  if (remaining.length() == 0) {
    printDynamics();
    return;
  }

  bool changed = false;

  while (remaining.length() > 0) {
    int split = remaining.indexOf(' ');
    if (split == -1) {
      Serial.printf("  %s has no value\n", remaining.c_str());
      break;
    }

    String key = remaining.substring(0, split);
    remaining = remaining.substring(split + 1);
    remaining.trim();
    if (remaining.length() == 0) {
      Serial.printf("  %s has no value\n", key.c_str());
      break;
    }

    split = remaining.indexOf(' ');
    String valStr = (split == -1) ? remaining : remaining.substring(0, split);
    remaining     = (split == -1) ? ""        : remaining.substring(split + 1);
    remaining.trim();

    if (applyDynamicsSetting(key, valStr)) changed = true;
  }

  if (changed) {
    saveSettings();
    Serial.println(F("Settings updated and saved."));
  } else {
    Serial.println(F("No changes applied."));
  }
}

void handleSerialCommands() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();
  if (cmd.length() == 0) return;

  if (cmd == "help") {
    printHelp();
  } else if (cmd == "pair" || cmd == "forget") {
    enterPairingMode();
  } else if (cmd == "diag") {
    diag_enabled = !diag_enabled;
    Serial.println(diag_enabled ? F("Diagnostics ON") : F("Diagnostics OFF"));
  } else if (cmd == "dynamics") {
    printDynamics();
  } else if (cmd.startsWith("dynamics ")) {
    handleDynamicsCommand(cmd.substring(9));
  } else {
    Serial.printf("Unknown command: %s. Type 'help'.\n", cmd.c_str());
  }
}

// ===================================================================
// LIVE DIAGNOSTICS
// ===================================================================

/*
  Prints button codes the moment they change, and raw stick values at 4 Hz
  while any axis is off centre. Useful for working out what a third-party
  controller actually sends, and for spotting a stick that no longer centres.
*/
void report_controller_state(ControllerPtr gp, unsigned long now) {
  uint32_t buttons = gp->buttons();
  uint8_t  misc    = gp->miscButtons();
  uint8_t  dpad    = gp->dpad();

  if (buttons != prev_reported_buttons || misc != prev_reported_misc || dpad != prev_reported_dpad) {
    Serial.printf("BUTTONS: 0x%04X | MISC: 0x%02X | DPAD: 0x%02X\n", buttons, misc, dpad);
    prev_reported_buttons = buttons;
    prev_reported_misc    = misc;
    prev_reported_dpad    = dpad;
  }

  if (now - last_joy_report_time < JOY_REPORT_INTERVAL_MS) return;
  last_joy_report_time = now;

  int lx = gp->axisX(),  ly = gp->axisY();
  int rx = gp->axisRX(), ry = gp->axisRY();
  int trigL = gp->brake(), trigR = gp->throttle();

  bool active = abs(ly) >= get_effective_deadzone(INPUT_DEADZONE_LS_Y) ||
                abs(lx) >= get_effective_deadzone(INPUT_DEADZONE_LS_X) ||
                abs(ry) >= get_effective_deadzone(INPUT_DEADZONE_RS_Y) ||
                abs(rx) >= get_effective_deadzone(INPUT_DEADZONE_RS_X) ||
                trigL >= 50 || trigR >= 50;

  if (active) {
    Serial.printf("JOY RAW: LX=%4d LY=%4d | RX=%4d RY=%4d | L2=%4d R2=%4d\n",
                  lx, ly, rx, ry, trigL, trigR);
  }
}
