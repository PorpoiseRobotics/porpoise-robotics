/*
  l3c_tank_drive.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives the vehicle with the left thumbstick. That is all it does - no LEDs,
  no servos, no scanner. It is the driving half of pathfinder_nintendoswitch.ino
  with everything else stripped out, so you can read the whole thing in one go.

  If you understand this program, you understand how the vehicle drives.

  SAFETY
  ------
  Wheels off the ground for the first upload. Once you are happy that the
  controls do what you expect, put it on the floor in a clear space. If the
  controller disconnects the motors stop by themselves.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none. Bluepad32 arrives with the board package.

  FILL IN MY_CONTROLLER FIRST
  ---------------------------
  Paste in the line that l3a_controller_check printed for your controller.
  Until you do, this vehicle refuses every controller and will not drive.

  CONTROLS
  --------
    Left stick   Drive. Up = forward, down = reverse, left/right = turn.

  HOW TANK DRIVE WORKS
  --------------------
  This vehicle has no steering rack. The two wheels on the left are driven
  together and the two on the right are driven together, and it turns by
  running one side faster than the other. That is why it is called tank drive.

        left  = forward + turn
        right = forward - turn

  Push straight up:   forward = 200, turn = 0    ->  left 200, right 200
  Push up and right:  forward = 200, turn = 60   ->  left 260, right 140
  Push right only:    forward = 0,   turn = 127  ->  left 127, right -127

  That last one is a spin on the spot. And notice the 260 - it is over the
  maximum, which is what constrain() is there to catch.

  WHAT TO TRY
  -----------
  1. Drive it. Then set turnMax to MOTOR_MAX and drive it again. Which is
     easier to control? That is why the full program starts you on half.
  2. Swap the + and the - in the mixing lines. What happens when you steer?
  3. Change the mixing to  left = forward + turn  and  right = forward  only.
     Why is that a worse way to steer?
  4. Comment out the drive(0, 0) in the disconnected branch and think hard
     about what would happen if your battery ran out mid-drive. Put it back.
*/

#include <Bluepad32.h>
#include <uni.h>

// --- The one controller this vehicle will talk to --------------------
const uint8_t MY_CONTROLLER[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// --- Motors ---
// Two pins per motor, and a different PWM channel number for each pin.
const int FRONT_LEFT_PIN_A  = 12, FRONT_LEFT_CH_A  = 0;
const int FRONT_LEFT_PIN_B  = 13, FRONT_LEFT_CH_B  = 1;
const int REAR_LEFT_PIN_A   = 18, REAR_LEFT_CH_A   = 2;
const int REAR_LEFT_PIN_B   = 19, REAR_LEFT_CH_B   = 3;
const int FRONT_RIGHT_PIN_A = 22, FRONT_RIGHT_CH_A = 4;
const int FRONT_RIGHT_PIN_B = 23, FRONT_RIGHT_CH_B = 5;
const int REAR_RIGHT_PIN_A  = 16, REAR_RIGHT_CH_A  = 6;
const int REAR_RIGHT_PIN_B  = 17, REAR_RIGHT_CH_B  = 7;

const int MOTOR_PWM_FREQ = 20000;
const int MOTOR_PWM_BITS = 8;
const int MOTOR_MAX      = 255;
const int MOTOR_MIN      = 0;

const int turnMax = MOTOR_MAX / 2;   // Steering held to half power

// --- Thumbsticks ---
const int STICK_MAX      = 511;
const int STICK_DEADZONE = 60;

ControllerPtr myController = nullptr;
bool addressIsSet = false;
bool wasConnected = false;

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
  Serial.print("Controller connected: ");
  Serial.println(controller->getModelName());
}

void onDisconnectedController(ControllerPtr controller) {
  if (myController == controller) {
    myController = nullptr;
  }
}

int stickToSpeed(int stickValue, int maxSpeed) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return 0;
  }
  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, maxSpeed);
  size = constrain(size, MOTOR_MIN, maxSpeed);
  return (stickValue > 0) ? size : -size;
}

void attachMotorPwm(int pin, int channel) {
  ledcSetup(channel, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttachPin(pin, channel);
}

/*
  Drives one motor. Speed runs from -255 (full reverse) to +255 (full forward).
  Zero puts both channels low, which lets the motor coast to a stop.
*/
void setMotor(int channelA, int channelB, int speed) {
  speed = constrain(speed, -MOTOR_MAX, MOTOR_MAX);
  if (speed >= 0) {
    ledcWrite(channelA, speed);
    ledcWrite(channelB, 0);
  } else {
    ledcWrite(channelA, 0);
    ledcWrite(channelB, -speed);   // -speed turns the negative number positive
  }
}

void drive(int leftSpeed, int rightSpeed) {
  setMotor(FRONT_LEFT_CH_A,  FRONT_LEFT_CH_B,  leftSpeed);
  setMotor(REAR_LEFT_CH_A,   REAR_LEFT_CH_B,   leftSpeed);
  setMotor(FRONT_RIGHT_CH_A, FRONT_RIGHT_CH_B, rightSpeed);
  setMotor(REAR_RIGHT_CH_A,  REAR_RIGHT_CH_B,  rightSpeed);
}

void setup() {
  Serial.begin(115200);

  attachMotorPwm(FRONT_LEFT_PIN_A,  FRONT_LEFT_CH_A);
  attachMotorPwm(FRONT_LEFT_PIN_B,  FRONT_LEFT_CH_B);
  attachMotorPwm(REAR_LEFT_PIN_A,   REAR_LEFT_CH_A);
  attachMotorPwm(REAR_LEFT_PIN_B,   REAR_LEFT_CH_B);
  attachMotorPwm(FRONT_RIGHT_PIN_A, FRONT_RIGHT_CH_A);
  attachMotorPwm(FRONT_RIGHT_PIN_B, FRONT_RIGHT_CH_B);
  attachMotorPwm(REAR_RIGHT_PIN_A,  REAR_RIGHT_CH_A);
  attachMotorPwm(REAR_RIGHT_PIN_B,  REAR_RIGHT_CH_B);
  drive(0, 0);

  for (int i = 0; i < 6; i++) {
    if (MY_CONTROLLER[i] != 0x00) {
      addressIsSet = true;
    }
  }

  if (!addressIsSet) {
    Serial.println("MY_CONTROLLER has not been filled in, so this vehicle will");
    Serial.println("not drive. Run l3a_controller_check and paste the line it");
    Serial.println("prints into the top of this program.");
    return;
  }

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);

  bd_addr_t allowed;
  memcpy(allowed, MY_CONTROLLER, 6);
  uni_bt_allowlist_remove_all();
  uni_bt_allowlist_add_addr(allowed);
  uni_bt_allowlist_set_enabled(true);
  BP32.enableNewBluetoothConnections(true);

  Serial.println("Tank drive. Waiting for the controller...");
}

void loop() {
  if (!addressIsSet) {
    delay(1000);
    return;
  }

  BP32.update();

  // ---- No controller? Stop everything and wait. ----
  if (myController == nullptr || !myController->isConnected()) {
    if (wasConnected) {
      Serial.println("Controller disconnected. Motors stopped.");
      wasConnected = false;
    }
    drive(0, 0);      // Safety first: never drive without a controller
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    myController->setPlayerLEDs(0x01);
    Serial.println("Drive carefully.");
  }

  int leftStickX = myController->axisX();
  int leftStickY = myController->axisY();

  int forward = stickToSpeed(-leftStickY, MOTOR_MAX);
  int turn    = stickToSpeed(leftStickX, turnMax);

  int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);
  int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);

  drive(leftSpeed, rightSpeed);
}
