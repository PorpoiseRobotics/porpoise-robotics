/*
  a2a_pwm_resolution.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 2

  WHAT THIS PROGRAM DOES
  ----------------------
  Drives the front left motor from a serial console, at your choice of PWM
  resolution, and prints what each duty value means in percent and in volts.

  The beginner programs use 8-bit PWM. Op Program 12 uses 10-bit. This program
  is about why that choice exists and what it buys you.

  SAFETY
  ------
  Wheels off the ground. One motor is driven, up to full power.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none.

  COMMANDS (115200 baud)
  ----------------------
    bits <8|10|12>   change the resolution and re-attach the channels
    freq <hz>        change the switching frequency
    duty <n>         set the duty directly, 0 to (2^bits - 1)
    pct <n>          set the duty as a percentage, 0 to 100
    step             sweep the whole range in 16 steps, printing each one
    stop             both channels to zero
    status           show where things stand

  RESOLUTION, FREQUENCY, AND THE TRADE BETWEEN THEM
  -------------------------------------------------
  A duty value is a count, not a voltage. At N bits the range is 0 to 2^N - 1:

        8 bits   ->  0..255     256 steps
        10 bits  ->  0..1023    1024 steps
        12 bits  ->  0..4095    4096 steps

  More bits means finer control near the bottom of the range, which is exactly
  where a motor is hardest to drive smoothly. At 8 bits one step is 0.39% of
  full power; at 10 bits it is 0.098%. On a vehicle that ramps its speed - and
  Op 12 does - that finer grain is the difference between a smooth start and a
  visible staircase.

  It is not free. The ESP32 LEDC peripheral runs from an 80 MHz clock, and:

        max frequency = 80,000,000 / 2^bits

  So 8 bits allows up to 312 kHz, 10 bits up to 78 kHz, 12 bits up to 19.5 kHz.
  Ask for a combination the hardware cannot make and ledcSetup returns 0 and
  you get silence, which is why this program checks the return value - a habit
  worth keeping.

  Op 12 runs 10 bits at 30 kHz. That is comfortably inside the limit, above
  hearing, and gives four times the resolution of the beginner programs.

  WHAT TO TRY
  -----------
  1. Run "step" at 8 bits, then "bits 10" and run it again. Watch the volts
     column, and listen at the very bottom of the range.
  2. Try "bits 12" then "freq 30000". What does the console tell you?
  3. Find the lowest "pct" at which the wheel turns at all. Is it the same at
     8 and at 10 bits?
  4. Set "freq 300" and listen. Then "freq 20000". Where does the whine go?
*/

const int MOTOR_PIN_A = 12;    // Front left
const int MOTOR_PIN_B = 13;
const int CHANNEL_A   = 0;
const int CHANNEL_B   = 1;

const float SUPPLY_VOLTS = 16.0f;   // Roughly a charged 4S pack

int pwm_bits = 8;
int pwm_freq = 20000;
int duty_now = 0;

String input_line = "";

int maxDuty() {
  return (1 << pwm_bits) - 1;      // 1 << 8 is 256, so max is 255
}

/*
  Sets both channels up at the current bits and frequency, and reports whether
  the hardware could actually do it. ledcSetup returns the frequency it managed
  to configure, or 0 if the combination is impossible.
*/
bool applyPwmSettings() {
  uint32_t got_a = ledcSetup(CHANNEL_A, pwm_freq, pwm_bits);
  uint32_t got_b = ledcSetup(CHANNEL_B, pwm_freq, pwm_bits);

  ledcAttachPin(MOTOR_PIN_A, CHANNEL_A);
  ledcAttachPin(MOTOR_PIN_B, CHANNEL_B);

  if (got_a == 0 || got_b == 0) {
    Serial.printf("REFUSED: %d bits at %d Hz is beyond the peripheral.\n",
                  pwm_bits, pwm_freq);
    Serial.printf("At %d bits the ceiling is %ld Hz.\n",
                  pwm_bits, 80000000L / (1L << pwm_bits));
    return false;
  }

  Serial.printf("PWM: %d bits (0..%d), asked %d Hz, got %lu Hz\n",
                pwm_bits, maxDuty(), pwm_freq, (unsigned long)got_a);
  return true;
}

void setDuty(int duty) {
  duty_now = constrain(duty, 0, maxDuty());
  ledcWrite(CHANNEL_A, duty_now);
  ledcWrite(CHANNEL_B, 0);
}

void reportDuty() {
  float fraction = (float)duty_now / (float)maxDuty();
  Serial.printf("duty %4d / %4d   %5.1f%%   about %5.2f V\n",
                duty_now, maxDuty(), fraction * 100.0f,
                fraction * SUPPLY_VOLTS);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println(F("=== PWM resolution ==="));
  Serial.println(F("Wheels off the ground. Type 'help'."));

  applyPwmSettings();
  setDuty(0);
}

void loop() {
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
}

void runCommand(String line) {
  line.trim();
  line.toLowerCase();

  if (line == "help") {
    Serial.println(F("  bits <8|10|12>   resolution"));
    Serial.println(F("  freq <hz>        switching frequency"));
    Serial.println(F("  duty <n>         raw duty value"));
    Serial.println(F("  pct <n>          duty as a percentage"));
    Serial.println(F("  step             sweep the range in 16 steps"));
    Serial.println(F("  stop             motor off"));
    Serial.println(F("  status           where things stand"));

  } else if (line == "status") {
    Serial.printf("%d bits, %d Hz, ceiling %ld Hz\n",
                  pwm_bits, pwm_freq, 80000000L / (1L << pwm_bits));
    reportDuty();

  } else if (line == "stop") {
    setDuty(0);
    Serial.println(F("stopped"));

  } else if (line == "step") {
    Serial.println(F("--- sweeping ---"));
    for (int i = 0; i <= 16; i++) {
      setDuty((long)maxDuty() * i / 16);
      reportDuty();
      delay(400);
    }
    setDuty(0);
    Serial.println(F("--- stopped ---"));

  } else if (line.startsWith("bits ")) {
    int value = line.substring(5).toInt();
    if (value < 1 || value > 14) {
      Serial.println(F("bits must be 1 to 14"));
      return;
    }
    int previous = pwm_bits;
    pwm_bits = value;
    setDuty(0);
    if (!applyPwmSettings()) {
      pwm_bits = previous;         // Put it back rather than leave it broken
      applyPwmSettings();
    }

  } else if (line.startsWith("freq ")) {
    long value = line.substring(5).toInt();
    if (value < 50 || value > 500000) {
      Serial.println(F("freq must be 50 to 500000"));
      return;
    }
    int previous = pwm_freq;
    pwm_freq = (int)value;
    setDuty(0);
    if (!applyPwmSettings()) {
      pwm_freq = previous;
      applyPwmSettings();
    }

  } else if (line.startsWith("duty ")) {
    setDuty(line.substring(5).toInt());
    reportDuty();

  } else if (line.startsWith("pct ")) {
    int percent = constrain(line.substring(4).toInt(), 0, 100);
    setDuty((long)maxDuty() * percent / 100);
    reportDuty();

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
