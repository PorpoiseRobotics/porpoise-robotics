/*
  l2b_speed_ramp.ino
  Porpoise Robotics - Pathfinder beginner course (PS3 track), Lesson 2

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
  Board package: "esp32" by Espressif Systems, VERSION 3.0.7
    Tools > Board > ESP32 Arduino > "ESP32 Dev Module".
  Libraries: none.

  THE MATHS TO WATCH FOR
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
  1. Upload it and watch. At roughly what duty value do the wheels START to
     turn? At what value does the sound stop changing?
  2. Change STEP_MS from 30 to 5. Now the ramp is much faster. Does the vehicle
     behave differently?
  3. Change PWM_FREQ from 20000 to 300 and upload. Listen. That whine is the
     switching frequency, now down inside the range of human hearing. Change it
     back afterwards.
  4. Write down the highest duty value at which the vehicle still creeps
     smoothly rather than jerking. That is useful when you tune your own
     driving program.
*/

// All four motors, two pins each.
const int FRONT_LEFT_A  = 12, FRONT_LEFT_B  = 13;
const int REAR_LEFT_A   = 18, REAR_LEFT_B   = 19;
const int FRONT_RIGHT_A = 22, FRONT_RIGHT_B = 23;
const int REAR_RIGHT_A  = 16, REAR_RIGHT_B  = 17;

const int PWM_FREQ = 20000;   // 20 kHz, above human hearing
const int PWM_BITS = 8;       // Duty values 0..255
const int PWM_MAX  = 255;

const int STEP_MS = 30;       // Milliseconds between one duty value and the next

void setup() {
  Serial.begin(115200);

  ledcAttach(FRONT_LEFT_A,  PWM_FREQ, PWM_BITS);
  ledcAttach(FRONT_LEFT_B,  PWM_FREQ, PWM_BITS);
  ledcAttach(REAR_LEFT_A,   PWM_FREQ, PWM_BITS);
  ledcAttach(REAR_LEFT_B,   PWM_FREQ, PWM_BITS);
  ledcAttach(FRONT_RIGHT_A, PWM_FREQ, PWM_BITS);
  ledcAttach(FRONT_RIGHT_B, PWM_FREQ, PWM_BITS);
  ledcAttach(REAR_RIGHT_A,  PWM_FREQ, PWM_BITS);
  ledcAttach(REAR_RIGHT_B,  PWM_FREQ, PWM_BITS);

  allMotors(0);

  Serial.println("Speed ramp. Wheels off the ground, please.");
  Serial.println("duty  |  duty cycle");
  delay(2000);
}

/*
  Sends the same speed to all four motors.
  A positive speed drives the A pins, a negative speed drives the B pins, and
  zero switches both off so the motors coast.
*/
void allMotors(int speed) {
  int forwardDuty = (speed > 0) ? speed : 0;
  int reverseDuty = (speed < 0) ? -speed : 0;

  ledcWrite(FRONT_LEFT_A,  forwardDuty);   ledcWrite(FRONT_LEFT_B,  reverseDuty);
  ledcWrite(REAR_LEFT_A,   forwardDuty);   ledcWrite(REAR_LEFT_B,   reverseDuty);
  ledcWrite(FRONT_RIGHT_A, forwardDuty);   ledcWrite(FRONT_RIGHT_B, reverseDuty);
  ledcWrite(REAR_RIGHT_A,  forwardDuty);   ledcWrite(REAR_RIGHT_B,  reverseDuty);
}

/*
  Prints the duty value and what percentage of each cycle that works out as.
  The "* 100L" uses a long, because 255 * 100 is fine but it is a good habit
  once you start multiplying before you divide.
*/
void report(int duty) {
  long percent = (long)abs(duty) * 100L / PWM_MAX;
  Serial.print(duty);
  Serial.print("\t ");
  Serial.print(percent);
  Serial.println("%");
}

void loop() {
  Serial.println("--- forward, winding up ---");
  for (int duty = 0; duty <= PWM_MAX; duty += 5) {
    allMotors(duty);
    report(duty);
    delay(STEP_MS);
  }

  Serial.println("--- forward, winding down ---");
  for (int duty = PWM_MAX; duty >= 0; duty -= 5) {
    allMotors(duty);
    report(duty);
    delay(STEP_MS);
  }

  allMotors(0);
  Serial.println("--- stopped ---");
  delay(2000);

  Serial.println("--- reverse, winding up ---");
  for (int duty = 0; duty <= PWM_MAX; duty += 5) {
    allMotors(-duty);
    report(-duty);
    delay(STEP_MS);
  }

  Serial.println("--- reverse, winding down ---");
  for (int duty = PWM_MAX; duty >= 0; duty -= 5) {
    allMotors(-duty);
    report(-duty);
    delay(STEP_MS);
  }

  allMotors(0);
  Serial.println("--- stopped ---");
  delay(3000);
}
