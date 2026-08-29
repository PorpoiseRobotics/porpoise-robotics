/*
  a5b_ina219_current.ino
  Porpoise Robotics - Pathfinder advanced course, Lesson 5

  WHAT THIS PROGRAM DOES
  ----------------------
  Reads battery voltage and current draw from the INA219 sensor and prints
  them, then drives one motor so you can watch what a motor actually costs.
  It also records the current SIGNATURE of a motor spin-up, which is what the
  self-test in Pathfinder_Op_Program12 makes its pass/fail decision on.

  Gen 3 vehicles only. On a Gen 2 there is no sensor and the program says so.

  SAFETY
  ------
  Wheels off the ground. The "spin" command drives one motor at full power.

  BEFORE YOU CAN COMPILE THIS
  ---------------------------
  Board package: "esp32_bluepad32" by Ricardo Quesada, version 4.1.0
    Tools > Board > esp32_bluepad32 > "ESP32 Dev Module".
  Libraries: none. Wire ships with the core.

  COMMANDS (115200 baud)
  ----------------------
    read            one reading
    watch           four readings a second until you type 'stop'
    stop
    baseline        measure the resting draw with the motors off
    spin <0-3>      spin one motor and record its current signature
    test            run every motor in turn and judge each one
    regs            dump the raw registers

  MEASURING CURRENT WITHOUT INTERRUPTING IT
  -----------------------------------------
  You cannot measure current directly. What the INA219 does is put a very small
  known resistor - a SHUNT - in the path, and measure the voltage across it.
  Ohm's law does the rest:

        I = V / R

  The shunt on this board is 0.0025 ohms, so two amps drops only 5 millivolts
  across it. That is deliberate: a bigger resistor would be easier to read but
  would waste power and rob the motors of voltage. Reading five millivolts
  accurately is the sensor's whole job.

  The shunt register reports in units of 10 microvolts, which is where the
  0.01 in the maths below comes from.

  DRIVING A CHIP WITHOUT A LIBRARY
  --------------------------------
  There is a perfectly good Adafruit INA219 library. Op 12 does not use it, and
  neither does this, because the whole driver is about forty lines and seeing
  the register writes makes the datasheet readable instead of mysterious.

        0x00  configuration     voltage range, resolution, mode
        0x01  shunt voltage     what we divide by R
        0x02  bus voltage       the battery, in the top 13 bits
        0x05  calibration

  Note the read: write the register number, then a REPEATED START rather than a
  stop, then read two bytes. That is what Wire.endTransmission(false) means. A
  full stop in the middle would let another master grab the bus between the two
  halves of one logical read.

  WHAT A HEALTHY MOTOR LOOKS LIKE
  -------------------------------
  A motor that is about to turn has almost no back-EMF, so it draws a large
  spike as it breaks away, then settles as it comes up to speed.

        disconnected   almost nothing, no spike
        healthy        a clear spike, then a lower steady draw
        jammed         a large draw that never settles

  So the self-test is not one number, it is a shape. And both thresholds are
  measured ABOVE the resting baseline, not as absolutes - a vehicle whose
  electronics idle at half an amp would fail every motor otherwise. Op 11.2
  measured a baseline, printed it, and then compared against fixed absolutes
  anyway. Op 12 fixed that.

  WHAT TO TRY
  -----------
  1. "baseline" with everything off. Now switch the headlights on and take
     another. Where did the difference go?
  2. "spin 0" with the wheel free. Now hold the wheel and do it again. Compare
     the peak and the average.
  3. Unplug one motor and run "test". Which numbers give it away?
  4. Work out how long a 3300 mAh pack lasts at the average you measured.
  5. Compare your maths with the LED power budget from the beginner course.
*/

#include <Wire.h>

// ===================================================================
// CONFIGURATION
// ===================================================================

const uint8_t INA219_I2C_ADDR = 0x40;
const int     I2C_SDA_PIN     = 32;
const int     I2C_SCL_PIN     = 33;
const float   SHUNT_RESISTOR_OHMS = 0.0025f;

const uint8_t REG_CONFIG = 0x00;
const uint8_t REG_SHUNT  = 0x01;
const uint8_t REG_BUS    = 0x02;
const uint8_t REG_POWER  = 0x03;
const uint8_t REG_CURRENT = 0x04;
const uint8_t REG_CALIB  = 0x05;

const int PWM_FREQ = 30000;
const int PWM_RES  = 10;
const int PWM_MAX  = 1023;

// The same thresholds Op 12 uses, both measured ABOVE the baseline.
const float PEAK_ABOVE_BASELINE_A = 1.500f;
const float AVG_ABOVE_BASELINE_A  = 1.000f;

const int BASELINE_DURATION = 500;
const int SPIN_DURATION     = 300;

struct Motor {
  int in1_pin, in2_pin;
  uint8_t in1_channel, in2_channel;
  const char *name;
};

Motor motors[4] = {
  {13, 12, 0, 1, "front left"},
  {23, 22, 2, 3, "front right"},
  {19, 18, 4, 5, "rear left"},
  {17, 16, 6, 7, "rear right"}
};

bool sensor_present = false;
float baseline_current = 0.0f;
bool watching = false;
unsigned long last_watch = 0;

String input_line = "";

// ===================================================================
// THE DRIVER
// ===================================================================

void ina219Write16(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value >> 8);       // High byte first
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

uint16_t ina219Read16(uint8_t reg) {
  Wire.beginTransmission(INA219_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);          // Repeated start, do not release the bus
  Wire.requestFrom((uint8_t)INA219_I2C_ADDR, (uint8_t)2);
  return Wire.available() >= 2 ? (Wire.read() << 8) | Wire.read() : 0;
}

bool isIna219Present() {
  Wire.beginTransmission(INA219_I2C_ADDR);
  return Wire.endTransmission() == 0;
}

void ina219Init() {
  ina219Write16(REG_CONFIG, 0x8000);              // Reset
  delay(1);
  ina219Write16(REG_CONFIG, 0b0001100111001111);  // 32 V range, 12-bit, continuous
  ina219Write16(REG_CALIB, 20999);                // Calibration for this shunt
}

/*
  The shunt register is SIGNED - current can flow either way - so it has to be
  read into an int16_t. Reading it as unsigned turns every regenerative braking
  current into a very large positive number.
*/
float readCurrentAmps() {
  int16_t raw = (int16_t)ina219Read16(REG_SHUNT);
  return (raw * 0.01f) / (SHUNT_RESISTOR_OHMS * 1000.0f);
}

/*
  The bus voltage sits in the top 13 bits, with status bits below it, so it has
  to be shifted down before it means anything. One count is 4 millivolts.
*/
float readBusVolts() {
  uint16_t value = ina219Read16(REG_BUS);
  return (float)((value >> 3) * 4) * 0.001f;
}

// ===================================================================
// MOTORS
// ===================================================================

void setupMotors() {
  for (int i = 0; i < 4; i++) {
    ledcSetup(motors[i].in1_channel, PWM_FREQ, PWM_RES);
    ledcSetup(motors[i].in2_channel, PWM_FREQ, PWM_RES);
    ledcAttachPin(motors[i].in1_pin, motors[i].in1_channel);
    ledcAttachPin(motors[i].in2_pin, motors[i].in2_channel);
  }
  coastAll();
}

void coastAll() {
  for (int i = 0; i < 4; i++) {
    ledcWrite(motors[i].in1_channel, 0);
    ledcWrite(motors[i].in2_channel, 0);
  }
}

void setMotorFull(int index, int direction) {
  ledcWrite(motors[index].in1_channel, direction > 0 ? PWM_MAX : 0);
  ledcWrite(motors[index].in2_channel, direction < 0 ? PWM_MAX : 0);
}

// ===================================================================
// MEASUREMENT
// ===================================================================

/*
  Samples for the given number of milliseconds and reports back the peak and
  the average. Sampling as fast as the loop allows is what catches the spike -
  one reading taken at the wrong moment would miss it entirely.
*/
void sampleCurrent(int duration_ms, float &peak_out, float &average_out) {
  unsigned long start = millis();
  float sum = 0.0f, peak = 0.0f;
  long count = 0;

  while (millis() - start < (unsigned long)duration_ms) {
    float amps = readCurrentAmps();
    if (amps > peak) peak = amps;
    sum += amps;
    count++;
  }

  peak_out = peak;
  average_out = count > 0 ? sum / count : 0.0f;
}

void measureBaseline() {
  coastAll();
  delay(100);

  float peak, average;
  sampleCurrent(BASELINE_DURATION, peak, average);
  baseline_current = average;

  Serial.printf("baseline: %.3f A average, %.3f A peak, battery %.2f V\n",
                baseline_current, peak, readBusVolts());
}

/*
  Spins one motor one way and reports its signature against the baseline.
  Returns true if it looks healthy.
*/
bool spinAndJudge(int index, int direction, bool quiet) {
  float peak, average;

  setMotorFull(index, direction);
  sampleCurrent(SPIN_DURATION, peak, average);
  coastAll();
  delay(150);

  float peak_above = peak - baseline_current;
  float avg_above  = average - baseline_current;

  bool ok = (peak_above >= PEAK_ABOVE_BASELINE_A) &&
            (avg_above  >= AVG_ABOVE_BASELINE_A);

  if (!quiet) {
    Serial.printf("  %-12s %s   peak %+.3f A   avg %+.3f A   %s\n",
                  motors[index].name,
                  direction > 0 ? "fwd" : "rev",
                  peak_above, avg_above,
                  ok ? "PASS" : "FAIL");
  }
  return ok;
}

void runFullTest() {
  Serial.println(F("--- self-test ---"));
  measureBaseline();
  Serial.printf("thresholds: peak >= %.3f A and avg >= %.3f A above baseline\n",
                PEAK_ABOVE_BASELINE_A, AVG_ABOVE_BASELINE_A);

  bool all_ok = true;
  for (int i = 0; i < 4; i++) {
    if (!spinAndJudge(i,  1, false)) all_ok = false;
    if (!spinAndJudge(i, -1, false)) all_ok = false;
  }

  Serial.println(all_ok ? F("--- ALL MOTORS PASS ---")
                        : F("--- ONE OR MORE MOTORS FAILED ---"));
  Serial.println(F("A motor that fails both directions is usually unplugged."));
  Serial.println(F("A motor that draws a lot and never settles is jammed."));
}

// ===================================================================
// SETUP AND LOOP
// ===================================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  setupMotors();

  Serial.println();
  Serial.println(F("=== INA219 current sensing ==="));

  sensor_present = isIna219Present();
  if (!sensor_present) {
    Serial.println(F("No INA219 found at 0x40."));
    Serial.println(F("This is a Gen 2 vehicle, or the sensor is not connected."));
    Serial.println(F("Run a5a_i2c_scan to check the bus."));
    return;
  }

  ina219Init();
  Serial.println(F("INA219 found. Gen 3 vehicle."));
  Serial.printf("battery %.2f V\n", readBusVolts());
  Serial.println(F("Wheels off the ground. Type 'help'."));

  measureBaseline();
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

  if (watching && sensor_present && millis() - last_watch >= 250) {
    last_watch = millis();
    Serial.printf("%.2f V   %+.3f A\n", readBusVolts(), readCurrentAmps());
  }
}

void runCommand(String line) {
  line.trim();
  line.toLowerCase();

  if (line == "help") {
    Serial.println(F("  read              one reading"));
    Serial.println(F("  watch / stop      four readings a second"));
    Serial.println(F("  baseline          resting draw, motors off"));
    Serial.println(F("  spin <0-3>        one motor, both directions"));
    Serial.println(F("  test              every motor, judged"));
    Serial.println(F("  regs              raw registers"));
    return;
  }

  if (!sensor_present) {
    Serial.println(F("No sensor on this vehicle."));
    return;
  }

  if (line == "read") {
    Serial.printf("%.2f V   %+.3f A\n", readBusVolts(), readCurrentAmps());

  } else if (line == "watch") {
    watching = true;
    Serial.println(F("watching. Type 'stop'."));

  } else if (line == "stop") {
    watching = false;
    coastAll();
    Serial.println(F("stopped"));

  } else if (line == "baseline") {
    measureBaseline();

  } else if (line == "test") {
    runFullTest();

  } else if (line == "regs") {
    Serial.printf("  0x00 config   0x%04X\n", ina219Read16(REG_CONFIG));
    Serial.printf("  0x01 shunt    %d  (10 uV units, signed)\n",
                  (int16_t)ina219Read16(REG_SHUNT));
    Serial.printf("  0x02 bus      0x%04X  ->  %.2f V\n",
                  ina219Read16(REG_BUS), readBusVolts());
    Serial.printf("  0x03 power    0x%04X\n", ina219Read16(REG_POWER));
    Serial.printf("  0x04 current  0x%04X\n", ina219Read16(REG_CURRENT));
    Serial.printf("  0x05 calib    0x%04X\n", ina219Read16(REG_CALIB));

  } else if (line.startsWith("spin ")) {
    int index = line.substring(5).toInt();
    if (index < 0 || index > 3) {
      Serial.println(F("Motor must be 0 to 3."));
      return;
    }
    Serial.printf("--- %s ---\n", motors[index].name);
    spinAndJudge(index,  1, false);
    spinAndJudge(index, -1, false);

  } else {
    Serial.println(F("Unknown. Type 'help'."));
  }
}
