/*
  l2a_one_motor.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 2

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives ONE motor - the front left one - forwards, stops, backwards, stops,
  over and over. Only one motor moves, so the vehicle cannot drive away from
  you while you are still learning what the numbers do.

  SAFETY
  ------
  Put the vehicle up on a block so all four wheels spin free before you upload
  this. Keep fingers, hair and cables away from the wheels.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32" by Espressif Systems, VERSION 3.0.7
    Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  Libraries: none.

  HOW ONE MOTOR IS WIRED
  ----------------------
  Each motor has a DRV8871 H-bridge driver chip, and each driver takes TWO
  signals from the ESP32. Call them A and B:

        A high, B low   ->  motor turns one way
        A low,  B high  ->  motor turns the other way
        both low        ->  motor coasts to a stop
        both high       ->  motor brakes hard (we do not use that here)

  So the DIRECTION is which pin you drive, and the SPEED is how hard you drive
  it. That is what an H-bridge is for, and it is why every motor needs two pins
  instead of one.

  WHY ledcWrite AND NOT analogWrite
  ---------------------------------
  A pin can only be fully on or fully off. To get a half speed we switch it on
  and off thousands of times a second and vary how much of each cycle is "on".
  That is PULSE WIDTH MODULATION, and the fraction that is on is the DUTY
  CYCLE. The ESP32 has hardware that does this for us; ledcAttach() sets it up
  and ledcWrite() sets the duty.

  With 8 bits of resolution the duty runs 0 to 255:
        0   = off             (0% duty)
        64  = quarter power   (25% duty)
        128 = half power      (50% duty)
        255 = full power      (100% duty)

  WHAT TO TRY
  -----------
  1. Upload it and watch the front left wheel. Does it match the Serial
     Monitor?
  2. Change SPEED from 200 to 60. Does the wheel still turn? Now try 30, then
     20. Find the smallest number that still gets the wheel moving. Write it
     down - that number is why MOTOR_MIN exists in the full program.
  3. Swap MOTOR_PIN_A and MOTOR_PIN_B and upload. What changed?
  4. Change MOTOR_PIN_A and MOTOR_PIN_B to 22 and 23. Which wheel moves now?
*/

// The four motors on this vehicle, and the two pins each one uses:
//        front left   12, 13
//        rear left    18, 19
//        front right  22, 23
//        rear right   16, 17
// This program only uses the first pair.
const int MOTOR_PIN_A = 12;
const int MOTOR_PIN_B = 13;

const int PWM_FREQ = 20000;   // 20 kHz. Above human hearing, so no motor whine.
const int PWM_BITS = 8;       // 8 bits of resolution means duty values 0..255

const int SPEED = 200;        // How hard to drive the motor, 0 to 255

void setup() {
  Serial.begin(115200);

  // Set up hardware PWM on both pins: (pin, frequency, resolution).
  // After this we can call ledcWrite(pin, 0..255) on either of them.
  ledcAttach(MOTOR_PIN_A, PWM_FREQ, PWM_BITS);
  ledcAttach(MOTOR_PIN_B, PWM_FREQ, PWM_BITS);

  // Both pins off, so the motor is definitely still before we start.
  ledcWrite(MOTOR_PIN_A, 0);
  ledcWrite(MOTOR_PIN_B, 0);

  Serial.println("One motor test. Wheels off the ground, please.");
  delay(2000);
}

void loop() {
  Serial.println("Forward");
  ledcWrite(MOTOR_PIN_A, SPEED);   // Drive A...
  ledcWrite(MOTOR_PIN_B, 0);       // ...and leave B off
  delay(2000);

  Serial.println("Stop");
  ledcWrite(MOTOR_PIN_A, 0);
  ledcWrite(MOTOR_PIN_B, 0);
  delay(2000);

  Serial.println("Reverse");
  ledcWrite(MOTOR_PIN_A, 0);       // Now the other way round:
  ledcWrite(MOTOR_PIN_B, SPEED);   // drive B and leave A off
  delay(2000);

  Serial.println("Stop");
  ledcWrite(MOTOR_PIN_A, 0);
  ledcWrite(MOTOR_PIN_B, 0);
  delay(2000);
}
