/*
  l2b_speed_ramp.ino
  Porpoise Robotics - Pathfinder beginner course (Nintendo Switch track), Lesson 2

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives all four motors forward, slowly winding the speed up from 0 to full
  and back down to 0 again, printing the duty value and the duty cycle as a
  percentage as it goes. Then it does the same in reverse.

  This is the program that makes the connection between a NUMBER IN THE CODE
  and HOW FAST THE WHEELS ACTUALLY TURN. Watch the wheels and read the Serial
  Monitor at the same time.

  SAFETY
  ------
  Wheels off the ground. All four are driven, and it does reach full speed.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none.

  EIGHT PINS, EIGHT CHANNELS
  --------------------------
  Four motors, two pins each, so eight PWM channels - numbers 0 to 7. Every one
  has to be different. Setting them up eight times by hand is repetitive, so
  attachMotorPwm() below does the two-line job once and gets called eight
  times. That is what functions are for.

  THE MATH TO WATCH FOR
  ----------------------
  Duty cycle is the fraction of each PWM cycle that the pin spends switched on:

        duty cycle % = duty value / 255 * 100

  The wheel speed is roughly proportional to that, but only roughly. Down at
  the bottom of the range the motor has to overcome friction before it turns
  at all, so a duty of 20 may give you no movement whatsoever while a duty of
  40 gives you a slow crawl. That bottom end is not a straight line, and
  finding where it stops being one is the point of this exercise.

  WHAT TO TRY
  -----------
  1. Upload it, and watch the wheels and the Serial Monitor together. Each
     duty value is held for STEP_MS, long enough to see whether the wheels
     have started. At what duty do they START to turn? Do all four start
     at the same number? At what value does the sound stop changing?
  2. DUTY_STEP is 5, so your answer is only good to within 5. Set it to 1
     and run it again to pin the number down exactly. The ramp is much
     slower now - that is the price of the precision.
  3. Put DUTY_STEP back to 5 and change STEP_MS from 300 to 20. Can you
     still tell where the wheels start? That is why it is slow to begin
     with.
  4. Change PWM_FREQ from 20000 to 300 and upload. Listen. That whine is the
     switching frequency, now down inside the range of human hearing. Change it
     back afterwards.
  5. Write down the highest duty value at which the vehicle still creeps
     smoothly rather than jerking. That is useful when you tune your own
     driving program.
*/

// All four motors: two pins each, and a different channel number for each pin.
const int FRONT_LEFT_PIN_A  = 12, FRONT_LEFT_CH_A  = 0;
const int FRONT_LEFT_PIN_B  = 13, FRONT_LEFT_CH_B  = 1;
const int REAR_LEFT_PIN_A   = 18, REAR_LEFT_CH_A   = 2;
const int REAR_LEFT_PIN_B   = 19, REAR_LEFT_CH_B   = 3;
const int FRONT_RIGHT_PIN_A = 22, FRONT_RIGHT_CH_A = 4;
const int FRONT_RIGHT_PIN_B = 23, FRONT_RIGHT_CH_B = 5;
const int REAR_RIGHT_PIN_A  = 16, REAR_RIGHT_CH_A  = 6;
const int REAR_RIGHT_PIN_B  = 17, REAR_RIGHT_CH_B  = 7;

const int PWM_FREQ = 20000;   // 20 kHz, above human hearing
const int PWM_BITS = 8;       // Duty values 0..255
const int PWM_MAX  = 255;

// How long each duty value is held, in milliseconds. Slow on purpose: long
// enough to read the number and see whether the wheels have started yet.
const int STEP_MS   = 300;
const int DUTY_STEP = 5;      // How far the duty moves each step. Try 1.

/*
  Gets one PWM channel ready and connects it to one pin.
*/
void attachMotorPwm(int pin, int channel) {
  ledcSetup(channel, PWM_FREQ, PWM_BITS);
  ledcAttachPin(pin, channel);
}

/*
  Sends the same speed to all four motors.
  A positive speed drives the A channels, a negative speed drives the B
  channels, and zero switches both off so the motors coast.
*/
void allMotors(int speed) {
  int forwardDuty = (speed > 0) ? speed : 0;
  int reverseDuty = (speed < 0) ? -speed : 0;

  ledcWrite(FRONT_LEFT_CH_A,  forwardDuty);   ledcWrite(FRONT_LEFT_CH_B,  reverseDuty);
  ledcWrite(REAR_LEFT_CH_A,   forwardDuty);   ledcWrite(REAR_LEFT_CH_B,   reverseDuty);
  ledcWrite(FRONT_RIGHT_CH_A, forwardDuty);   ledcWrite(FRONT_RIGHT_CH_B, reverseDuty);
  ledcWrite(REAR_RIGHT_CH_A,  forwardDuty);   ledcWrite(REAR_RIGHT_CH_B,  reverseDuty);
}

/*
  Prints the duty value and what percentage of each cycle that works out as.
*/
void report(int duty) {
  long percent = (long)abs(duty) * 100L / PWM_MAX;
  Serial.print(duty);
  Serial.print("\t ");
  Serial.print(percent);
  Serial.println("%");
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

  allMotors(0);

  Serial.println("Speed ramp. Wheels off the ground, please.");
  Serial.println("duty  |  duty cycle");
  delay(2000);
}

void loop() {
  Serial.println("--- forward, winding up ---");
  for (int duty = 0; duty <= PWM_MAX; duty += DUTY_STEP) {
    allMotors(duty);
    report(duty);
    delay(STEP_MS);
  }

  Serial.println("--- forward, winding down ---");
  for (int duty = PWM_MAX; duty >= 0; duty -= DUTY_STEP) {
    allMotors(duty);
    report(duty);
    delay(STEP_MS);
  }

  allMotors(0);
  Serial.println("--- stopped ---");
  delay(2000);

  Serial.println("--- reverse, winding up ---");
  for (int duty = 0; duty <= PWM_MAX; duty += DUTY_STEP) {
    allMotors(-duty);
    report(-duty);
    delay(STEP_MS);
  }

  Serial.println("--- reverse, winding down ---");
  for (int duty = PWM_MAX; duty >= 0; duty -= DUTY_STEP) {
    allMotors(-duty);
    report(-duty);
    delay(STEP_MS);
  }

  allMotors(0);
  Serial.println("--- stopped ---");
  delay(3000);
}
