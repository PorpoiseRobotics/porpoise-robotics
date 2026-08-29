/*
  l3b_deadzone_and_map.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 3

  WHAT THIS PROGRAM DOES
  ----------------------
  Reads the left thumbstick and prints, side by side, the raw number the
  controller sent and the motor speed that number turns into. Nothing moves.

  This is the single most important piece of arithmetic in the whole driving
  program, and it is much easier to understand on a screen than through a
  spinning wheel. It is the exact same stickToSpeed() function that
  pathfinder_ps3.ino uses.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  1. Board package: "esp32" by Espressif Systems, VERSION 3.0.7
       Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  2. Library: "PS3 Controller Host" by Jeffrey van Pernis

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

  Here it stretches "just past the deadzone, up to 127" across "MOTOR_MIN, up
  to MOTOR_MAX". Without it, the vehicle could never reach full speed, because
  the deadzone would have eaten part of the stick travel.

  WHAT TO TRY
  -----------
  1. Push the stick very slowly from centre to full forward and watch both
     columns. Where does speed stop being 0?
  2. Set STICK_DEADZONE to 0 and upload. Leave the controller flat on the desk
     and untouched. Does the speed stay at 0?
  3. Set STICK_DEADZONE to 100. Now how much of the stick travel is wasted?
  4. Set MOTOR_MIN to 60 and watch what happens the instant you leave the
     deadzone. That is what a minimum speed floor does - it is a real option
     for motors that will not start turning at low power.
  5. Change the maxSpeed argument in the turn line from turnMax to MOTOR_MAX.
     What is the point of steering having its own limit?
*/

#include <Ps3Controller.h>

const char *PS3_MAC_ADDRESS = "02:02:03:04:05:08";

// A PS3 stick reports -128 to +127 on each axis.
const int STICK_MAX      = 127;
const int STICK_DEADZONE = 20;    // About 16% of the travel

const int MOTOR_MAX = 255;        // Full speed
const int MOTOR_MIN = 0;          // Slowest speed given, just outside the deadzone

// Steering is held to half power by default, which makes the vehicle much
// easier to aim. The full program lets you toggle this with the stick click.
const int turnMax = MOTOR_MAX / 2;

unsigned long lastPrint = 0;

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

  Ps3.begin(PS3_MAC_ADDRESS);

  Serial.println();
  Serial.println("=== Deadzone and map ===");
  Serial.println("Press the PS button on your controller.");
  Serial.println();
  Serial.println("raw Y\t forward\t raw X\t turn\t| left\t right");
}

void loop() {
  if (!Ps3.isConnected()) {
    delay(200);
    return;
  }

  int leftStickX = Ps3.data.analog.stick.lx;
  int leftStickY = Ps3.data.analog.stick.ly;

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
