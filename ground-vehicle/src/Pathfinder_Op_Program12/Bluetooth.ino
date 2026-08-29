/*
  Bluetooth.ino - controller connection, and the pairing workflow that locks
  one vehicle to one controller.

  HOW THE LOCK WORKS
  Bluepad32 keeps an "allowlist": if an address is on it, that controller may
  connect, and if it is not, the connection is refused before it is accepted.
  This program stores the paired controller's address in EEPROM and puts it
  back on the allowlist at every boot, so the lock survives a power cycle.

  11.2 added addresses to the allowlist at runtime only. If those entries did
  not survive a reboot, a vehicle could come back up with an empty allowlist
  and new connections disabled, and then refuse its own controller. Writing
  the address down ourselves removes the question.
*/

// ===================================================================
// ADDRESS HELPERS
// ===================================================================

/*
  Prints a Bluetooth address the normal way, most significant byte first.
  11.2 printed these backwards, counting down from byte 5 to byte 0.
*/
void printAddress(const uint8_t *address) {
  for (int i = 0; i < 6; i++) {
    if (address[i] < 0x10) Serial.print('0');
    Serial.print(address[i], HEX);
    if (i < 5) Serial.print(':');
  }
}

void savePairedAddress(const uint8_t *address) {
  memcpy(paired_addr, address, 6);
  paired_addr_valid = true;

  EEPROM.write(ADDR_PAIRED_VALID, PAIRED_VALID_MAGIC);
  for (int i = 0; i < 6; i++) {
    EEPROM.write(ADDR_PAIRED_ADDR + i, address[i]);
  }
  EEPROM.commit();
}

void loadPairedAddress() {
  paired_addr_valid = (EEPROM.read(ADDR_PAIRED_VALID) == PAIRED_VALID_MAGIC);
  if (!paired_addr_valid) return;

  for (int i = 0; i < 6; i++) {
    paired_addr[i] = EEPROM.read(ADDR_PAIRED_ADDR + i);
  }
}

void clearPairedAddress() {
  paired_addr_valid = false;
  memset(paired_addr, 0, 6);
  EEPROM.write(ADDR_PAIRED_VALID, 0x00);
  EEPROM.commit();
}

// ===================================================================
// CONNECTION CALLBACKS
// ===================================================================

/*
  Bluepad32 calls this when a controller connects. It runs from BP32.update(),
  so it is safe to print and to touch program state from in here.
*/
void onConnectedController(ControllerPtr ctl) {
  ControllerProperties props = ctl->getProperties();

  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] != nullptr) continue;

    myControllers[i] = ctl;
    controller_now_connected = true;

    Serial.print(F("Controller connected: "));
    Serial.print(ctl->getModelName());
    Serial.print(F(" at "));
    printAddress(props.btaddr);
    Serial.println();

    // First controller to connect during pairing becomes this vehicle's own.
    if (!paired_addr_valid) {
      savePairedAddress(props.btaddr);
      applyAllowlist();
      Serial.print(F("Paired. This vehicle is now locked to "));
      printAddress(paired_addr);
      Serial.println();
      Serial.println(F("Type 'forget' to pair a different controller."));
    }

    ctl->setPlayerLEDs(0x01);
    return;
  }

  // No free slot: refuse it.
  Serial.println(F("No free controller slot. Connection refused."));
  ctl->disconnect();
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      Serial.println(F("Controller disconnected."));
      return;
    }
  }
}

ControllerPtr firstConnectedController() {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] && myControllers[i]->isConnected()) {
      return myControllers[i];
    }
  }
  return nullptr;
}

// ===================================================================
// ALLOWLIST
// ===================================================================

/*
  Rebuilds the allowlist from what we have stored. Called at boot and again
  whenever the paired controller changes, so the list always says exactly one
  thing and never accumulates stale entries.
*/
void applyAllowlist() {
  uni_bt_allowlist_remove_all();

  if (paired_addr_valid) {
    bd_addr_t allowed;
    memcpy(allowed, paired_addr, 6);
    uni_bt_allowlist_add_addr(allowed);
    uni_bt_allowlist_set_enabled(true);
  } else {
    uni_bt_allowlist_set_enabled(false);   // Pairing: anyone may knock
  }
}

// ===================================================================
// SETUP
// ===================================================================

void setup_bluetooth() {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) myControllers[i] = nullptr;

  loadPairedAddress();

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);

  applyAllowlist();

  // New connections stay enabled. The allowlist is what keeps other people's
  // controllers out; leaving this on means our own controller can always get
  // back in, even if the stored Bluetooth bonding keys are lost.
  BP32.enableNewBluetoothConnections(true);

  if (paired_addr_valid) {
    Serial.print(F("Paired controller: "));
    printAddress(paired_addr);
    Serial.println();
    led_mode = DISCONNECTED;
  } else {
    Serial.println(F("No controller paired yet. Entering pairing mode."));
    Serial.println(F("Put your controller into pairing mode now."));
    led_mode = PAIRING;
    breathe_time = millis();
  }
}

// ===================================================================
// PAIRING
// ===================================================================

/*
  Drops the current controller, forgets everything, and opens up for a new one.
  Reached by typing "pair" on the serial console or pressing the BOOT button.
*/
void enterPairingMode() {
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    if (myControllers[i] && myControllers[i]->isConnected()) {
      myControllers[i]->disconnect();
      myControllers[i] = nullptr;
    }
  }

  coast_all();

  BP32.forgetBluetoothKeys();
  clearPairedAddress();
  applyAllowlist();
  BP32.enableNewBluetoothConnections(true);

  Serial.println();
  Serial.println(F("PAIRING MODE - the LEDs are breathing blue."));
  Serial.println(F("Put your controller into pairing mode now."));

  uint8_t bt_mac[6];
  if (esp_read_mac(bt_mac, ESP_MAC_BT) == ESP_OK) {
    Serial.print(F("This vehicle's Bluetooth address: "));
    printAddress(bt_mac);
    Serial.println();
  }

  led_mode = PAIRING;
  breathe_time = millis();
}

/*
  The BOOT button on the ESP32 module is wired to GPIO 0 and reads LOW when
  pressed. Watching for the moment it goes low gives us one press, once.
*/
void handlePairButton() {
  static bool last_state = HIGH;
  bool state = digitalRead(PAIR_TRIGGER_PIN);

  if (state == LOW && last_state == HIGH) {
    enterPairingMode();
  }
  last_state = state;
}
