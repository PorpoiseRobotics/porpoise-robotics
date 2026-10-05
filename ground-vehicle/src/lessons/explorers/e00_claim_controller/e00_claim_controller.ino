/*
  e00_claim_controller
  Porpoise Robotics - Pathfinder Explorers, INSTRUCTOR SETUP

  WHAT THIS PROGRAM DOES
  ----------------------
  Teaches one vehicle which controller is its own. Run it once per vehicle,
  before Lesson 1. Students do not use it.

  It saves the controller's Bluetooth address in the vehicle's flash memory.
  Every Explorers sketch reads it back from there, so students never have to
  type an address, and uploading other programs does not erase it. (Choosing
  Tools > "Erase All Flash Before Sketch Upload: Enabled" does. Leave it off.)

  HOW TO USE IT
  -------------
  1. Switch OFF every other controller in the room. This program accepts the
     first controller that knocks.
  2. Upload this program to the vehicle, wheels up, and open the Serial
     Monitor at 115200 baud. The lights blink BLUE while it searches.
  3. Hold the small round SYNC button on the controller until its lights run
     back and forth. When it connects, the vehicle's lights turn solid BLUE
     and the Serial Monitor shows the controller's address.
  4. Press A on that controller. The lights flash GREEN three times and the
     controller buzzes. The claim is saved.
  5. Press the other buttons. The Serial Monitor names each one, so you can
     check that the letters printed on this controller match what the course
     expects. If they do not, note it on the controller's sticker.
  6. Put matching stickers on the vehicle and the controller - the same number
     on both. Then upload e01a_learner_drive, ready for Lesson 1.

  To give a vehicle a different controller, just run this again.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0.
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Library: "Adafruit NeoPixel" by Adafruit.
*/

#include "explorer.h"

ControllerPtr candidate = nullptr;    // The controller that has just connected
bool claimedThisTime = false;

void onFound(ControllerPtr pad) {
  if (candidate != nullptr) {
    pad->disconnect();                // One at a time
    return;
  }
  candidate = pad;
  claimedThisTime = false;

  Serial.println();
  Serial.print("Found a controller: ");
  Serial.println(pad->getModelName());
  Serial.print("Its address is ");
  printAddress(pad->getProperties().btaddr);
  Serial.println();
  Serial.println("Press A on THAT controller to make it this vehicle's own.");

  pad->setPlayerLEDs(0x01);
  setAllLights(BLUE);
}

void onLost(ControllerPtr pad) {
  if (pad == candidate) {
    candidate = nullptr;
    Serial.println("Controller left. Waiting for the next one...");
  }
}

void setup() {
  startRover();

  uint8_t current[6];
  Serial.println();
  if (loadClaimedController(current)) {
    Serial.print("This vehicle currently belongs to controller ");
    printAddress(current);
    Serial.println();
  } else {
    Serial.println("This vehicle has no controller yet.");
  }
  Serial.println("Hold SYNC on the ONE controller you want, until its lights run.");

  BP32.setup(&onFound, &onLost);
  BP32.enableVirtualDevice(false);

  // Forget old pairings and empty the guest list, so any controller can knock.
  BP32.forgetBluetoothKeys();
  uni_bt_allowlist_remove_all();
  uni_bt_allowlist_set_enabled(false);
  BP32.enableNewBluetoothConnections(true);
}

void loop() {
  BP32.update();

  if (candidate == nullptr || !candidate->isConnected()) {
    if (blinkIsOn()) {
      setAllLights(mixColor(0, 0, 60));    // Slow blue blink: searching
    } else {
      lightsOff();
    }
    delay(10);
    return;
  }

  // The same buttonA()...buttonY() mapping the lessons use.
  buttonsBefore = buttonsNow;
  buttonsNow = candidate->buttons();

  if (padTapped(BUTTON_B) && !claimedThisTime) {        // A on a Switch pad
    if (saveClaimedController(candidate->getProperties().btaddr)) {
      claimedThisTime = true;
      Serial.println();
      Serial.println("SAVED. This vehicle now belongs to this controller.");
      Serial.println("Put matching stickers on both, then upload e01a_learner_drive.");
      candidate->playDualRumble(0, 400, 0x80, 0x80);
      for (int flash = 0; flash < 3; flash++) {
        setAllLights(GREEN);
        delay(250);
        lightsOff();
        delay(250);
      }
      setAllLights(mixColor(0, 60, 0));
    } else {
      Serial.println("Could not save. Try pressing A again.");
    }
  }

  if (padTapped(BUTTON_B)) Serial.println("You pressed: A");
  if (padTapped(BUTTON_A)) Serial.println("You pressed: B");
  if (padTapped(BUTTON_Y)) Serial.println("You pressed: X");
  if (padTapped(BUTTON_X)) Serial.println("You pressed: Y");
  if (padTapped(BUTTON_SHOULDER_L)) Serial.println("You pressed: L");
  if (padTapped(BUTTON_SHOULDER_R)) Serial.println("You pressed: R");
  if (padTapped(BUTTON_TRIGGER_L))  Serial.println("You pressed: ZL");
  if (padTapped(BUTTON_TRIGGER_R))  Serial.println("You pressed: ZR");

  delay(10);
}
