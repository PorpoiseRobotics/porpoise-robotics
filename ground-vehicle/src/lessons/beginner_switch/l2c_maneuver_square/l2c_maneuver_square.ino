/*
  l2c_maneuver_square.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 2

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives a square with no controller and no sensors: forward, turn 90 degrees,
  four times, then stop for twenty seconds so you can pick the vehicle up.

  This is DEAD RECKONING. The vehicle has no idea where it is. It is doing
  exactly what you told it to do for exactly as long as you told it to, and
  the only reason the shape comes out square is that your numbers were right.
  Aircraft and ships navigated this way for centuries, and submersibles still
  fall back on it when they cannot see a satellite.

  SAFETY
  ------
  This one drives on the floor, on purpose. Clear a space about three metres
  square, keep bags and feet out of it, and be ready to switch the vehicle off
  at the power switch. It starts moving as soon as it is powered up or reset.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none.

  THE TWO EQUATIONS THIS PROGRAM IS BUILT ON
  ------------------------------------------
        distance = speed x time
        degrees turned = turn rate x time

  You know the time, because you wrote it in the delay(). You have to MEASURE
  the speed and the turn rate for your own vehicle, because they depend on your
  motors, your wheels, your battery charge and the floor you are on.

        wheel circumference = pi x diameter = 3.1416 x 3.0 in = 9.42 in
        one wheel turn      = 9.42 / 12 = 0.79 ft

  Measure how far the vehicle travels during FORWARD_MS at DRIVE_SPEED, divide
  by the time, and you have its speed in feet per second. Do the same for one
  full 360 degree spin and you have its turn rate in degrees per second.

  WHAT TO TRY
  -----------
  1. Run it and watch where the vehicle ends up. Does it come back to the start?
  2. Measure the side of the square it actually drives. Work out the speed:
        speed = distance / (FORWARD_MS / 1000)
  3. The corners are almost certainly not 90 degrees. Adjust TURN_MS by 25
     milliseconds at a time until the vehicle comes back to its start.
  4. Now change FORWARD_MS and predict how big the new square will be before
     you run it. How close were you?
  5. Copy the loop and turn it into a triangle. What angle does each corner
     need, and what does that do to TURN_MS?
  6. Recharge the battery and run it again with the same numbers. What happens,
     and what does that tell you about dead reckoning?
*/

const int FRONT_LEFT_PIN_A  = 12, FRONT_LEFT_CH_A  = 0;
const int FRONT_LEFT_PIN_B  = 13, FRONT_LEFT_CH_B  = 1;
const int REAR_LEFT_PIN_A   = 18, REAR_LEFT_CH_A   = 2;
const int REAR_LEFT_PIN_B   = 19, REAR_LEFT_CH_B   = 3;
const int FRONT_RIGHT_PIN_A = 22, FRONT_RIGHT_CH_A = 4;
const int FRONT_RIGHT_PIN_B = 23, FRONT_RIGHT_CH_B = 5;
const int REAR_RIGHT_PIN_A  = 16, REAR_RIGHT_CH_A  = 6;
const int REAR_RIGHT_PIN_B  = 17, REAR_RIGHT_CH_B  = 7;

const int PWM_FREQ = 20000;
const int PWM_BITS = 8;

// The numbers you are going to tune.
const int DRIVE_SPEED = 150;    // Duty value used to drive straight
const int TURN_SPEED  = 150;    // Duty value used for the corners
const int FORWARD_MS  = 1500;   // How long each side of the square takes
const int TURN_MS     = 500;    // How long a 90 degree corner takes. TUNE ME.

void attachMotorPwm(int pin, int channel) {
  ledcSetup(channel, PWM_FREQ, PWM_BITS);
  ledcAttachPin(pin, channel);
}

/*
  Sets the left pair and the right pair separately. This is TANK DRIVE: with
  both sides the same you go in a straight line, and with the two sides
  opposite you spin on the spot.
*/
void drive(int leftSpeed, int rightSpeed) {
  ledcWrite(FRONT_LEFT_CH_A,  leftSpeed  > 0 ?  leftSpeed  : 0);
  ledcWrite(FRONT_LEFT_CH_B,  leftSpeed  < 0 ? -leftSpeed  : 0);
  ledcWrite(REAR_LEFT_CH_A,   leftSpeed  > 0 ?  leftSpeed  : 0);
  ledcWrite(REAR_LEFT_CH_B,   leftSpeed  < 0 ? -leftSpeed  : 0);
  ledcWrite(FRONT_RIGHT_CH_A, rightSpeed > 0 ?  rightSpeed : 0);
  ledcWrite(FRONT_RIGHT_CH_B, rightSpeed < 0 ? -rightSpeed : 0);
  ledcWrite(REAR_RIGHT_CH_A,  rightSpeed > 0 ?  rightSpeed : 0);
  ledcWrite(REAR_RIGHT_CH_B,  rightSpeed < 0 ? -rightSpeed : 0);
}

void goForward()  { drive( DRIVE_SPEED,  DRIVE_SPEED); Serial.println("Forward"); }
void goBackward() { drive(-DRIVE_SPEED, -DRIVE_SPEED); Serial.println("Backward"); }
void spinRight()  { drive( TURN_SPEED,  -TURN_SPEED);  Serial.println("Spin right"); }
void spinLeft()   { drive(-TURN_SPEED,   TURN_SPEED);  Serial.println("Spin left"); }
void stopMoving() { drive(0, 0);                       Serial.println("Stop"); }

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

  stopMoving();

  Serial.println("Square maneuver. Clear the floor.");
  Serial.println("Starting in 5 seconds...");
  delay(5000);
}

void loop() {
  // Four sides and four corners makes a square.
  for (int corner = 0; corner < 4; corner++) {
    Serial.print("--- side ");
    Serial.print(corner + 1);
    Serial.println(" of 4 ---");

    goForward();
    delay(FORWARD_MS);

    spinRight();
    delay(TURN_MS);
  }

  stopMoving();
  Serial.println("Finished. Pick your vehicle up.");
  delay(20000);
}
