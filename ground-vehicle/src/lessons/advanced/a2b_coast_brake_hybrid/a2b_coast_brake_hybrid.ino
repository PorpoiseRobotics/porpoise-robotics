/*
  a2b_coast_brake_hybrid.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 2

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives the front left motor three different ways and lets you switch between
  them from the serial console, so you can feel the difference. It also
  demonstrates the speed ramp from Op Program 12.

  SAFETY
  ------
  Wheels off the ground. One motor, up to full power.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none.

  COMMANDS (115200 baud)
  ----------------------
    sign             slow decay: pulse one pin, hold the other low
    hybrid           fast decay: hold one pin high, pulse the other low
    go <n>           target duty, -1023 to 1023
    ramp on|off      turn the speed ramp on or off
    brake            both pins high, motor stops hard
    coast            both pins low, motor freewheels
    status

  THE FOUR STATES OF AN H-BRIDGE
  ------------------------------
        IN1   IN2    what the motor sees
        ---   ---    -------------------
        high  low    driven one way
        low   high   driven the other way
        low   low    COAST - both terminals floating, motor freewheels
        high  high   BRAKE - both terminals shorted together

  Coast and brake are not the same thing. A coasting motor keeps rolling. A
  braked motor is shorted across itself, so the voltage it generates as it
  spins drives a current that opposes the spin, and it stops quickly.

  SIGN-MAGNITUDE VS HYBRID DRIVE
  ------------------------------
  Say you want half power forward.

  SIGN-MAGNITUDE, which is what the beginner programs do, pulses IN1 at 50%
  and holds IN2 low. The motor alternates between DRIVEN and COAST. During the
  off part of the cycle nothing controls the motor at all, so the current in
  the windings decays slowly and the actual speed depends heavily on the load.
  This is called SLOW DECAY.

  HYBRID DRIVE, which is what Op Program 12 does, holds IN1 high and pulses
  IN2 at the inverse. The motor alternates between DRIVEN and BRAKE. The
  braking part actively pulls the current down, so the average is much closer
  to what you asked for. This is FAST DECAY, and the result is noticeably
  better control at low duty, which is exactly where you need it.

  Look at the maths in setMotorHybrid(): the duty you ask for turns into
  PWM_MAX on one pin and (PWM_MAX - duty) on the other. At duty 0 both are
  PWM_MAX, which is BRAKE, not coast. That is why Op 12 tests for zero
  separately and calls coast_all() instead.

  THE SPEED RAMP
  --------------
  Snapping straight to a new speed makes the vehicle lurch and yanks a lot of
  current out of the battery. Op 12 closes a fraction of the remaining gap each
  step instead:

        step = (target - current) x RAMP_FACTOR

  With integer maths that has a trap. Once the gap is down to one count,
  1 x 0.5 truncates to 0 and the ramp stalls one short of the target forever.
  Op 11.2 had that bug. rampTowards() below forces a minimum step of one
  count, which is the fix.

  WHAT TO TRY
  -----------
  1. "sign", then "go 100". Now pinch the tyre gently. Now "hybrid", "go 100",
     and pinch it again. Which one holds its speed?
  2. Find the lowest "go" value that turns the wheel in each mode.
  3. "ramp off", then "go 1023" from a standstill, then "ramp on" and do it
     again. Listen to the difference.
  4. Set RAMP_FACTOR to 0.9, then 0.1. What does it change?
  5. Change rampTowards to drop its minimum-step line, then "go 500" and watch
     the printout. Where does it stop?
*/

const int MOTOR_PIN_A = 12;    // Front left
const int MOTOR_PIN_B = 13;
const int CHANNEL_A   = 0;
const int CHANNEL_B   = 1;

const int PWM_FREQ = 30000;    // Same as Op Program 12
const int PWM_RES  = 10;       // 10 bits: 0..1023
const int PWM_MAX  = 1023;

const float RAMP_FACTOR   = 0.5f;   // Fraction of the gap closed each step
const int   RAMP_DELAY_MS = 10;

enum DriveMode { MODE_SIGN, MODE_HYBRID };

DriveMode drive_mode = MODE_HYBRID;
bool ramp_enabled = true;

int target_duty  = 0;
int current_duty = 0;
unsigned long last_ramp = 0;

String input_line = "";

// ===================================================================
// MOTOR OUTPUT
// ===================================================================

// Both pins low: the motor freewheels.
void coastMotor() {
  ledcWrite(CHANNEL_A, 0);
  ledcWrite(CHANNEL_B, 0);
}

// Both pins high: the motor is shorted across itself and stops hard.
void brakeMotor() {
  ledcWrite(CHANNEL_A, PWM_MAX);
  ledcWrite(CHANNEL_B, PWM_MAX);
}

/*
  SIGN-MAGNITUDE. Pulse one pin, hold the other low. Slow decay.
*/
void setMotorSign(int duty) {
  duty = constrain(duty, -PWM_MAX, PWM_MAX);
  if (duty >= 0) {
    ledcWrite(CHANNEL_A, duty);
    ledcWrite(CHANNEL_B, 0);
  } else {
    ledcWrite(CHANNEL_A, 0);
    ledcWrite(CHANNEL_B, -duty);
  }
}

/*
  HYBRID. Hold one pin high, pulse the other low. Fast decay.

  Note the zero case. Without it, duty 0 would put PWM_MAX on both pins, which
  is a hard brake rather than a coast.
*/
void setMotorHybrid(int duty) {
  duty = constrain(duty, -PWM_MAX, PWM_MAX);

  if (duty == 0) {
    coastMotor();
    return;
  }

  int inverse = PWM_MAX - abs(duty);
  if (duty > 0) {
    ledcWrite(CHANNEL_A, PWM_MAX);
    ledcWrite(CHANNEL_B, inverse);
  } else {
    ledcWrite(CHANNEL_A, inverse);
    ledcWrite(CHANNEL_B, PWM_MAX);
  }
}

void applyDuty(int duty) {
  if (drive_mode == MODE_HYBRID) {
    setMotorHybrid(duty);
  } else {
    setMotorSign(duty);
  }
}

/*
  Moves one step of the ramp from where we are towards where we want to be.

  The minimum-step line is the whole point. In integer maths a gap of 1 times
  0.5 truncates to 0, and without the fix the ramp stalls one count short of
  its target and stays there.
*/
int rampTowards(int current, int target) {
  int gap = target - current;
  if (gap == 0) return current;

  int step = (int)(gap * RAMP_FACTOR);
  if (step == 0) step = (gap > 0) ? 1 : -1;

  return current + step;
}

// ===================================================================
// SETUP AND LOOP
// ===================================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  ledcSetup(CHANNEL_A, PWM_FREQ, PWM_RES);
  ledcSetup(CHANNEL_B, PWM_FREQ, PWM_RES);
  ledcAttachPin(MOTOR_PIN_A, CHANNEL_A);
  ledcAttachPin(MOTOR_PIN_B, CHANNEL_B);
  coastMotor();

  Serial.println();
  Serial.println(F("=== Coast, brake, and hybrid drive ==="));
  Serial.println(F("Wheels off the ground. Type 'help'."));
}

void loop() {
  unsigned long now = millis();

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (input_line.length() > 0) {
        runCommand(input_line);
        input_line = "";
      }
    } else if (c >= 32 && c < 127 && input_line.length() < 40) {
      input_line += c;
    }
  }

  if (!ramp_enabled) {
    if (current_duty != target_duty) {
      current_duty = target_duty;
      applyDuty(current_duty);
    }
    return;
  }

  if (current_duty != target_duty && now - last_ramp >= RAMP_DELAY_MS) {
    last_ramp = now;
    current_duty = rampTowards(current_duty, target_duty);
    applyDuty(current_duty);

    // Print the last few steps, where the stall bug used to show up.
    if (abs(target_duty - current_duty) < 8) {
      Serial.printf("ramping: %d -> %d\n", current_duty, target_duty);
    }
  }
}

void runCommand(String line) {
  line.trim();
  line.toLowerCase();

  if (line == "help") {
    Serial.println(F("  sign             slow decay drive"));
    Serial.println(F("  hybrid           fast decay drive"));
    Serial.println(F("  go <n>           target duty, -1023 to 1023"));
    Serial.println(F("  ramp on|off      speed ramping"));
    Serial.println(F("  brake            both pins high"));
    Serial.println(F("  coast            both pins low"));
    Serial.println(F("  status"));

  } else if (line == "sign") {
    drive_mode = MODE_SIGN;
    applyDuty(current_duty);
    Serial.println(F("mode: SIGN-MAGNITUDE (slow decay)"));

  } else if (line == "hybrid") {
    drive_mode = MODE_HYBRID;
    applyDuty(current_duty);
    Serial.println(F("mode: HYBRID (fast decay)"));

  } else if (line == "brake") {
    target_duty = current_duty = 0;
    brakeMotor();
    Serial.println(F("BRAKE - both pins high"));

  } else if (line == "coast") {
    target_duty = current_duty = 0;
    coastMotor();
    Serial.println(F("COAST - both pins low"));

  } else if (line == "ramp on") {
    ramp_enabled = true;
    Serial.println(F("ramp on"));

  } else if (line == "ramp off") {
    ramp_enabled = false;
    Serial.println(F("ramp off"));

  } else if (line == "status") {
    Serial.printf("mode %s, ramp %s, current %d, target %d\n",
                  drive_mode == MODE_HYBRID ? "HYBRID" : "SIGN",
                  ramp_enabled ? "on" : "off",
                  current_duty, target_duty);

  } else if (line.startsWith("go ")) {
    target_duty = constrain(line.substring(3).toInt(), -PWM_MAX, PWM_MAX);
    Serial.printf("target %d\n", target_duty);

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
