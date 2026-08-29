/*
  pathfinder_find_controller.ino
  Porpoise Robotics - Pathfinder setup tool

  WHAT THIS PROGRAM DOES
  ----------------------
  Nothing except find out the Bluetooth address of a wireless Nintendo Switch
  style controller and print it on the Serial Monitor, ready to paste into
  pathfinder_nintendoswitch.ino.

  Every controller has its own Bluetooth address, like a house number. A
  Pathfinder vehicle is told the address of the one controller it is allowed
  to listen to, so that in a classroom full of vehicles and controllers, each
  vehicle only ever answers to its own. This program is how you look that
  address up.

  It does not touch the motors, the servos or the LEDs, so you can run it on
  any spare ESP32 board. You do not need a whole vehicle. Connect as many
  controllers as you like, one after another, and it prints each one - handy
  for labelling a whole box of controllers in one sitting.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0.
    File > Preferences > "Additional boards manager URLs", add this line:
      https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json
    Then Tools > Board > Boards Manager, search "bluepad32", and install it.
    Finally pick Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".

  No libraries to install. Bluepad32 arrives with the board package.

  HOW TO USE IT
  -------------
  1. Upload this program.
  2. Open Tools > Serial Monitor and set it to 115200 baud.
  3. Hold the small round SYNC button on the controller until its lights run
     back and forth, which means it is looking for something to connect to.
  4. Wait a few seconds. The address appears, along with the exact line of
     code to copy.
  5. Paste that line over the MY_CONTROLLER line in pathfinder_nintendoswitch.ino,
     upload that program to the vehicle, and the pair is locked together.
     Write the address on a sticker on both the controller and the vehicle.
*/

#include <Bluepad32.h>

// Set to true to also print the button and stick readings, which is a quick way
// to check that every button on a controller still works.
const bool TEST_THE_CONTROLLER = true;

// The controller we are currently reporting on: the most recent one to connect.
// nullptr means "nothing connected".
ControllerPtr activeController = nullptr;

// Remembers the last readings, so we only print when something actually changes
// instead of thousands of times per second.
uint16_t lastButtons = 0;
uint8_t  lastDpad    = 0;

/*
  Bluepad32 calls this by itself whenever a controller connects. We never call it
  ourselves - it is a "callback", a function we hand over for someone else to
  call at the right moment.
*/
void onConnectedController(ControllerPtr controller) {
  ControllerProperties properties = controller->getProperties();

  Serial.println();
  Serial.println("================================================");
  Serial.print("Controller found: ");
  Serial.println(controller->getModelName());

  // btaddr is the Bluetooth address: six numbers, written in hexadecimal.
  Serial.print("Address: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", properties.btaddr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  if (controller->battery() > 0) {
    Serial.printf("Battery: %d out of 255\n", controller->battery());
  }

  Serial.println();
  Serial.println("Copy this line into pathfinder_nintendoswitch.ino:");
  Serial.println();
  Serial.print("    const uint8_t MY_CONTROLLER[6] = { ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("0x%02X", properties.btaddr[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println(" };");
  Serial.println("================================================");
  Serial.println();

  controller->setPlayerLEDs(0x01);   // Light up player LED 1 so you know which pad answered

  activeController = controller;
  lastButtons = 0;
  lastDpad = 0;
}

/*
  Bluepad32 calls this by itself whenever a controller disconnects.
*/
void onDisconnectedController(ControllerPtr controller) {
  Serial.println("Controller disconnected. Ready for the next one.");
  if (activeController == controller) {
    activeController = nullptr;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);   // Give the Serial Monitor a moment to wake up

  Serial.println();
  Serial.println("Pathfinder controller finder");
  Serial.println("Hold the SYNC button on the controller until its lights");
  Serial.println("run back and forth, then wait a few seconds.");

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);   // We only want gamepads, not virtual mice

  // Throw away any controllers this board remembers, so a controller that was
  // used on another vehicle can still connect here.
  BP32.forgetBluetoothKeys();

  // Accept anything that knocks. This program is a lookup tool, so it is the one
  // place where we deliberately do NOT lock ourselves to a single controller.
  BP32.enableNewBluetoothConnections(true);
}

void loop() {
  // Ask Bluepad32 for fresh controller data. This is also where Bluepad32 calls
  // onConnectedController() and onDisconnectedController() for us.
  BP32.update();

  if (!TEST_THE_CONTROLLER) {
    return;
  }
  if (activeController == nullptr || !activeController->isConnected() || !activeController->isGamepad()) {
    return;
  }

  // Print the button codes as they change, so you can check that every button
  // works and see which code your controller sends for each one.
  if (activeController->buttons() != lastButtons || activeController->dpad() != lastDpad) {
    lastButtons = activeController->buttons();
    lastDpad = activeController->dpad();
    Serial.printf("Buttons: 0x%04X   D-pad: 0x%02X   Sticks: LX=%4d LY=%4d RX=%4d RY=%4d\n",
                  lastButtons, lastDpad,
                  activeController->axisX(), activeController->axisY(),
                  activeController->axisRX(), activeController->axisRY());
  }
}
