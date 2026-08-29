/*
  l3c_tank_drive.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives the vehicle with the left thumbstick. That is all it does - no LEDs,
  no servos, no scanner. It is the driving half of pathfinder_ps3.ino with
  everything else stripped out, so you can read the whole thing in one go.

  If you understand this program, you understand how the vehicle drives.

  SAFETY
  ------
  Wheels off the ground for the first upload. Once you are happy that the
  controls do what you expect, put it on the floor in a clear space. If the
  controller disconnects the motors stop by themselves.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  2. Library: "PS3 Controller Host" by Jeffrey van Pernis

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

#include <Ps3Controller.h>

const char *PS3_MAC_ADDRESS = "02:02:03:04:05:08";

// --- Motors ---
const int FRONT_LEFT_A  = 12, FRONT_LEFT_B  = 13;
const int REAR_LEFT_A   = 18, REAR_LEFT_B   = 19;
const int FRONT_RIGHT_A = 22, FRONT_RIGHT_B = 23;
const int REAR_RIGHT_A  = 16, REAR_RIGHT_B  = 17;

const int MOTOR_PWM_FREQ = 20000;
const int MOTOR_PWM_BITS = 8;
const int MOTOR_MAX      = 255;
const int MOTOR_MIN      = 0;

const int turnMax = MOTOR_MAX / 2;   // Steering held to half power

// --- Thumbsticks ---
const int STICK_MAX      = 127;
const int STICK_DEADZONE = 20;

bool wasConnected = false;

int stickToSpeed(int stickValue, int maxSpeed) {
  if (abs(stickValue) < STICK_DEADZONE) {
    return 0;
  }
  int size = map(abs(stickValue), STICK_DEADZONE, STICK_MAX, MOTOR_MIN, maxSpeed);
  size = constrain(size, MOTOR_MIN, maxSpeed);
  return (stickValue > 0) ? size : -size;
}

/*
  Drives one motor. Speed runs from -255 (full reverse) to +255 (full forward).
  Zero puts both pins low, which lets the motor coast to a stop.
*/
void setMotor(int pinA, int pinB, int speed) {
  speed = constrain(speed, -MOTOR_MAX, MOTOR_MAX);
  if (speed >= 0) {
    ledcWrite(pinA, speed);
    ledcWrite(pinB, 0);
  } else {
    ledcWrite(pinA, 0);
    ledcWrite(pinB, -speed);   // -speed turns the negative number positive
  }
}

void drive(int leftSpeed, int rightSpeed) {
  setMotor(FRONT_LEFT_A,  FRONT_LEFT_B,  leftSpeed);
  setMotor(REAR_LEFT_A,   REAR_LEFT_B,   leftSpeed);
  setMotor(FRONT_RIGHT_A, FRONT_RIGHT_B, rightSpeed);
  setMotor(REAR_RIGHT_A,  REAR_RIGHT_B,  rightSpeed);
}

void setup() {
  Serial.begin(115200);

  ledcAttach(FRONT_LEFT_A,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_LEFT_B,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_LEFT_A,   MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_LEFT_B,   MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_RIGHT_A, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(FRONT_RIGHT_B, MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_RIGHT_A,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  ledcAttach(REAR_RIGHT_B,  MOTOR_PWM_FREQ, MOTOR_PWM_BITS);
  drive(0, 0);

  Ps3.begin(PS3_MAC_ADDRESS);
  Serial.println("Tank drive. Waiting for the controller...");
}

void loop() {
  // ---- No controller? Stop everything and wait. ----
  if (!Ps3.isConnected()) {
    if (wasConnected) {
      Serial.println("Controller disconnected. Motors stopped.");
      wasConnected = false;
    }
    drive(0, 0);      // Safety first: never drive without a controller
    return;
  }

  if (!wasConnected) {
    wasConnected = true;
    Ps3.setPlayer(1);
    Serial.println("Controller connected. Drive carefully.");
  }

  int leftStickX = Ps3.data.analog.stick.lx;
  int leftStickY = Ps3.data.analog.stick.ly;

  int forward = stickToSpeed(-leftStickY, MOTOR_MAX);
  int turn    = stickToSpeed(leftStickX, turnMax);

  int leftSpeed  = constrain(forward + turn, -MOTOR_MAX, MOTOR_MAX);
  int rightSpeed = constrain(forward - turn, -MOTOR_MAX, MOTOR_MAX);

  drive(leftSpeed, rightSpeed);
}
