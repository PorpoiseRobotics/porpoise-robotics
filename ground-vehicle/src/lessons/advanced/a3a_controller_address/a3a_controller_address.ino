/*
  a3a_controller_address.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Shows the Bluetooth allowlist working, live, from the serial console. It
  prints the address of anything that connects, lets you lock the board to one
  controller, and lets you unlock it again - so you can watch a controller be
  accepted and then refused without recompiling anything.

  Nothing moves. Safe on the bench.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none. Bluepad32 arrives with the board package.

  COMMANDS (115200 baud)
  ----------------------
    open          allowlist off - anything may connect
    lock          allowlist on, holding whatever is connected right now
    lockaddr <a>  allowlist on, holding an address you type in
    forget        drop the bonding keys and disconnect
    whoami        this board's own Bluetooth address
    status

  THE PROBLEM THE ALLOWLIST SOLVES
  --------------------------------
  A classroom has twenty vehicles and twenty controllers switched on at once.
  Bluetooth pairing on its own does not help: a Switch pad in pairing mode will
  happily connect to whichever host answers first. Without something stopping
  it, two students end up driving the same vehicle.

  Bluepad32 answers this with an ALLOWLIST - a guest list checked before a
  connection is accepted. Three calls:

        uni_bt_allowlist_remove_all();
        uni_bt_allowlist_add_addr(address);
        uni_bt_allowlist_set_enabled(true);

  Two things about them are easy to get wrong:

    - They must be called AFTER BP32.setup(), because setup is what brings the
      Bluetooth stack up. Before that there is no list to add to.

    - Rebuild the list from scratch every boot rather than adding to it. That
      way the program is always the single source of truth about who may drive,
      and stale entries cannot accumulate.

  Leaving enableNewBluetoothConnections(true) on is deliberate. The allowlist
  is what keeps other people out; leaving new connections enabled means your
  own controller can always get back in even if the stored bonding keys are
  lost.

  WHERE EACH PROGRAM KEEPS ITS ADDRESS
  ------------------------------------
    pathfinder_nintendoswitch   a constant in the source, MY_CONTROLLER
    Pathfinder_Op_Program12     EEPROM, written the first time you pair
    this sketch                 RAM, so it is gone at the next reset

  The next sketch, a3b_eeprom_settings, is about the middle one.

  BYTE ORDER, AND A BUG WORTH KNOWING ABOUT
  -----------------------------------------
  btaddr is a uint8_t[6] already in printed order, so byte 0 prints first.
  Op Program 11.2 printed it backwards by counting down from 5, which meant
  the address it told you to use was the reverse of the real one. Op 12 fixed
  it. If an address will not work, print it at both ends and compare.

  WHAT TO TRY
  -----------
  1. Connect a controller. Type "lock". Now disconnect it and connect a
     DIFFERENT one. Watch the refusal.
  2. Type "open" and try the second controller again.
  3. Type "lockaddr 00:11:22:33:44:55" and try to connect anything.
  4. Type "whoami" and compare with what a Bluetooth scanner app shows.
*/

#include <Bluepad32.h>
#include <uni.h>
#include <esp_mac.h>

ControllerPtr myController = nullptr;

bool    lock_enabled = false;
uint8_t locked_addr[6] = { 0, 0, 0, 0, 0, 0 };
uint8_t last_seen_addr[6] = { 0, 0, 0, 0, 0, 0 };
bool    have_seen_one = false;

String input_line = "";

/*
  Prints a Bluetooth address the normal way, most significant byte first.
*/
void printAddress(const uint8_t *address) {
  for (int i = 0; i < 6; i++) {
    if (address[i] < 0x10) Serial.print('0');
    Serial.print(address[i], HEX);
    if (i < 5) Serial.print(':');
  }
}

/*
  Rebuilds the allowlist from what we currently believe. Called whenever the
  lock changes, so the list always says exactly one thing.
*/
void applyAllowlist() {
  uni_bt_allowlist_remove_all();

  if (lock_enabled) {
    bd_addr_t allowed;
    memcpy(allowed, locked_addr, 6);
    uni_bt_allowlist_add_addr(allowed);
    uni_bt_allowlist_set_enabled(true);
  } else {
    uni_bt_allowlist_set_enabled(false);
  }

  BP32.enableNewBluetoothConnections(true);
}

void onConnectedController(ControllerPtr controller) {
  ControllerProperties properties = controller->getProperties();

  memcpy(last_seen_addr, properties.btaddr, 6);
  have_seen_one = true;

  if (myController != nullptr) {
    Serial.print(F("Second controller refused: "));
    printAddress(properties.btaddr);
    Serial.println();
    controller->disconnect();
    return;
  }

  myController = controller;
  controller->setPlayerLEDs(0x01);

  Serial.print(F("Connected: "));
  Serial.print(controller->getModelName());
  Serial.print(F("  at  "));
  printAddress(properties.btaddr);
  Serial.println();

  if (!lock_enabled) {
    Serial.println(F("The allowlist is OFF. Type 'lock' to hold this one."));
  }
}

void onDisconnectedController(ControllerPtr controller) {
  if (myController == controller) {
    myController = nullptr;
    Serial.println(F("Disconnected."));
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println(F("=== Controller address and allowlist ==="));

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);

  // AFTER BP32.setup(). Before it there is no allowlist to configure.
  applyAllowlist();

  Serial.println(F("Allowlist starts OFF. Type 'help'."));
}

void loop() {
  BP32.update();

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

/*
  Reads "00:11:22:33:44:55" into six bytes. Returns false if it does not look
  like an address, rather than storing half of one.
*/
bool parseAddress(String text, uint8_t *out) {
  text.trim();
  text.replace("-", ":");
  if (text.length() != 17) return false;

  for (int i = 0; i < 6; i++) {
    String pair = text.substring(i * 3, i * 3 + 2);
    char *end;
    long value = strtol(pair.c_str(), &end, 16);
    if (*end != '\0' || value < 0 || value > 255) return false;
    out[i] = (uint8_t)value;
  }
  return true;
}

void runCommand(String line) {
  line.trim();
  String lower = line;
  lower.toLowerCase();

  if (lower == "help") {
    Serial.println(F("  open          allowlist off"));
    Serial.println(F("  lock          allowlist on, holding what is connected"));
    Serial.println(F("  lockaddr <a>  allowlist on, holding an address"));
    Serial.println(F("  forget        drop bonding keys and disconnect"));
    Serial.println(F("  whoami        this board's Bluetooth address"));
    Serial.println(F("  status"));

  } else if (lower == "open") {
    lock_enabled = false;
    applyAllowlist();
    Serial.println(F("Allowlist OFF. Anything may connect."));

  } else if (lower == "lock") {
    if (!have_seen_one) {
      Serial.println(F("Nothing has connected yet. Connect something first."));
      return;
    }
    memcpy(locked_addr, last_seen_addr, 6);
    lock_enabled = true;
    applyAllowlist();
    Serial.print(F("Allowlist ON, holding "));
    printAddress(locked_addr);
    Serial.println();

  } else if (lower.startsWith("lockaddr ")) {
    uint8_t parsed[6];
    if (!parseAddress(line.substring(9), parsed)) {
      Serial.println(F("Could not read that. Use 00:11:22:33:44:55"));
      return;
    }
    memcpy(locked_addr, parsed, 6);
    lock_enabled = true;
    applyAllowlist();
    Serial.print(F("Allowlist ON, holding "));
    printAddress(locked_addr);
    Serial.println();

  } else if (lower == "forget") {
    if (myController != nullptr) {
      myController->disconnect();
      myController = nullptr;
    }
    BP32.forgetBluetoothKeys();
    Serial.println(F("Bonding keys dropped."));

  } else if (lower == "whoami") {
    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_BT) == ESP_OK) {
      Serial.print(F("This board: "));
      printAddress(mac);
      Serial.println();
    } else {
      Serial.println(F("Could not read the MAC."));
    }

  } else if (lower == "status") {
    Serial.print(F("allowlist "));
    Serial.println(lock_enabled ? F("ON") : F("OFF"));
    if (lock_enabled) {
      Serial.print(F("locked to  "));
      printAddress(locked_addr);
      Serial.println();
    }
    Serial.print(F("connected  "));
    Serial.println(myController != nullptr ? F("yes") : F("no"));

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
