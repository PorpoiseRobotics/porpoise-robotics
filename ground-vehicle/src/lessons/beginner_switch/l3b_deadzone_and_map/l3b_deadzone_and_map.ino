/*
  l3b_deadzone_and_map.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Reads the left thumbstick and prints, side by side, the raw number the
  controller sent and the motor speed that number turns into. Nothing moves.

  This is the single most important piece of arithmetic in the whole driving
  program, and it is much easier to understand on a screen than through a
  spinning wheel. It is the exact same stickToSpeed() function that
  pathfinder_nintendoswitch.ino uses.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none. Bluepad32 arrives with the board package.

  FILL IN MY_CONTROLLER FIRST
  ---------------------------
  Paste in the line that l3a_controller_check printed for your controller.
  Until you do, this vehicle will not accept any controller at all and will
  say so on the Serial Monitor.

  This is how every Switch-track vehicle keeps other people out: the ALLOWLIST
  is Bluepad32's guest list, and a controller that is not on it is turned away
  before the connection is even accepted. It is the mirror image of what the
  PS3 track does, where the CONTROLLER is told which vehicle to talk to.

  THE TWO IDEAS
  -------------
  DEADZONE. A thumbstick that has been thumbed by a hundred students does not
  come back to exactly zero. If we believed it, the vehicle would creep across
  the room on its own. So any reading smaller than STICK_DEADZONE is treated as
  a firm zero.

  MAP. Once we are past the deadzone we still want the FULL range of motor
  speed available, not the leftover part. map() takes a number that lives in
  one range and works out where it sits in another:

        map(value, fromLow, fromHigh, toLow, toHigh)

  Here it stretches "just past the deadzone, up to 511" across "MOTOR_MIN, up
  to MOTOR_MAX". Without it, the vehicle could never reach full speed, because
  the deadzone would have eaten part of the stick travel.

  WHAT TO TRY
  -----------
  1. Push the stick very slowly from centre to full forward and watch both
     columns. Where does speed stop being 0?
  2. Set STICK_DEADZONE to 0 and upload. Put the controller down and do not
     touch it. Does the speed stay at 0?
  3. Set STICK_DEADZONE to 400. Now how much of the stick travel is wasted?
  4. Set MOTOR_MIN to 60 and watch what happens the instant you leave the
     deadzone. That is what a minimum speed floor does - it is a real option
     for motors that will not start turning at low power.
  5. Change the maxSpeed argument in the turn line from turnMax to MOTOR_MAX.
     What is the point of steering having its own limit?
*/

#include <Bluepad32.h>
#include <uni.h>                // Lets us use the Bluetooth allowlist

// --- The one controller this program will talk to --------------------
// Run l3a_controller_check to get this line.
const uint8_t MY_CONTROLLER[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Bluepad32 reports every stick axis as -512 to +511.
const int STICK_MAX      = 511;
const int STICK_DEADZONE = 60;    // About 12% of the travel

const int MOTOR_MAX = 255;        // Full speed
const int MOTOR_MIN = 0;          // Slowest speed given, just outside the deadzone

// Steering is held to half power by default, which makes the vehicle much
// easier to aim. The full program lets you toggle this with the stick click.
const int turnMax = MOTOR_MAX / 2;

ControllerPtr myController = nullptr;
bool addressIsSet = false;
unsigned long lastPrint = 0;

bool isMyController(const uint8_t *address) {
  for (int i = 0; i < 6; i++) {
    if (address[i] != MY_CONTROLLER[i]) {
      return false;
    }
  }
  return true;
}

void onConnectedController(ControllerPtr controller) {
  if (myController != nullptr || !isMyController(controller->getProperties().btaddr)) {
    controller->disconnect();
    return;
  }
  myController = controller;
  controller->setPlayerLEDs(0x01);
  Serial.println("Controller connected.");
}

void onDisconnectedController(ControllerPtr controller) {
  if (myController == controller) {
    myController = nullptr;
    Serial.println("Controller disconnected.");
  }
}

/*
  Turns a thumbstick reading into a motor speed.
  Inside the deadzone the answer is 0. Outside it, the rest of the stick travel
  is stretched across MOTOR_MIN..maxSpeed.
*/
int stickToSpeed(int stickValue, int maxSpeed) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return 0;
  }
  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, maxSpeed);
  size = constrain(size, MOTOR_MIN, maxSpeed);
  return (stickValue > 0) ? size : -size;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=== Deadzone and map ===");

  // Has somebody filled in MY_CONTROLLER yet? All zeros means "no".
  for (int i = 0; i < 6; i++) {
    if (MY_CONTROLLER[i] != 0x00) {
      addressIsSet = true;
    }
  }

  if (!addressIsSet) {
    Serial.println("MY_CONTROLLER has not been filled in.");
    Serial.println("Upload l3a_controller_check, copy the line it prints,");
    Serial.println("paste it into the top of this program, and upload again.");
    return;
  }

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);

  // Rebuild the guest list from scratch every boot, so this sketch is always
  // the single source of truth about who is allowed to connect.
  bd_addr_t allowed;
  memcpy(allowed, MY_CONTROLLER, 6);
  uni_bt_allowlist_remove_all();
  uni_bt_allowlist_add_addr(allowed);
  uni_bt_allowlist_set_enabled(true);
  BP32.enableNewBluetoothConnections(true);

  Serial.println("Waiting for your controller...");
  Serial.println();
  Serial.println("raw Y\t forward\t raw X\t turn\t| left\t right");
}

void loop() {
  if (!addressIsSet) {
    delay(1000);
    return;
  }

  BP32.update();

  if (myController == nullptr || !myController->isConnected()) {
    delay(100);
    return;
  }

  int leftStickX = myController->axisX();
  int leftStickY = myController->axisY();

  // The stick gives a NEGATIVE number when pushed up, so flip the sign to get
  // a "forward" number that is positive when we want to go forward.
  int forward = stickToSpeed(-leftStickY, MOTOR_MAX);
  int turn    = stickToSpeed(leftStickX, turnMax);

  // Mixing forward and turn together is what lets you steer while moving.
  // Turning right means the left wheels have to go faster than the right ones.
  int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);
  int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);

  // Print about ten times a second, which is fast enough to feel live and slow
  // enough to read.
  if (millis() - lastPrint >= 100) {
    lastPrint = millis();

    Serial.print(leftStickY);   Serial.print("\t ");
    Serial.print(forward);      Serial.print("\t\t ");
    Serial.print(leftStickX);   Serial.print("\t ");
    Serial.print(turn);         Serial.print("\t| ");
    Serial.print(leftSpeed);    Serial.print("\t ");
    Serial.println(rightSpeed);
  }
}
