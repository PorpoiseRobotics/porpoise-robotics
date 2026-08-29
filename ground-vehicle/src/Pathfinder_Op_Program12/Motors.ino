/*
  Motors.ino - drive motors, servos, and the maths that turns a thumbstick
  reading into a motor speed.
*/

// ===================================================================
// SETUP
// ===================================================================

void setup_motors() {
  // Channels 0-7 belong to the eight motor pins. Each channel is configured,
  // then attached to its pin; from there we write duty values to the channel.
  for (uint8_t ch = 0; ch < 8; ch++) {
    ledcSetup(ch, PWM_FREQ, PWM_RES);
  }
  for (int i = 0; i < 4; i++) {
    ledcAttachPin(motors[i].in1_pin, motors[i].in1_channel);
    ledcAttachPin(motors[i].in2_pin, motors[i].in2_channel);
  }
  coast_all();
}

void setup_servos() {
  ledcSetup(SERVO1_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO2_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO3_CH, SERVO_FREQ, SERVO_RES);
  ledcSetup(SERVO4_CH, SERVO_FREQ, SERVO_RES);

  ledcAttachPin(SERVO1_PIN, SERVO1_CH);
  ledcAttachPin(SERVO2_PIN, SERVO2_CH);
  ledcAttachPin(SERVO3_PIN, SERVO3_CH);
  ledcAttachPin(SERVO4_PIN, SERVO4_CH);

  write_servo(SERVO1_CH, SERVO_MID_US);
  write_servo(SERVO2_CH, SERVO_MID_US);
  write_servo(SERVO3_CH, SERVO_MID_US);
  write_servo(SERVO4_CH, SERVO_MID_US);
}

// ===================================================================
// INPUT PROCESSING
// ===================================================================

// A per-axis deadzone of -1 means "fall back to the global one".
int get_effective_deadzone(int specific_dz) {
  return (specific_dz >= 0) ? specific_dz : INPUT_DEADZONE;
}

/*
  Turns a raw stick reading into an output value.
  Inside the deadzone the answer is zero. Outside it, the remaining travel is
  scaled by the axis's scale factor and stretched across the output range.
*/
int remap_axis(int raw_value, int specific_dz, float scale_factor, int in_max, int out_max) {
  int dz = get_effective_deadzone(specific_dz);
  if (abs(raw_value) < dz) return 0;

  int sign   = (raw_value > 0) ? 1 : -1;
  int scaled = lround(abs(raw_value) * scale_factor);
  scaled = constrain(scaled, dz, in_max);

  return sign * map(scaled, dz, in_max, 0, out_max);
}

/*
  Moves one step of the ramp from where we are towards where we want to be.

  11.2 wrote this as `current += (target - current) * RAMP_FACTOR`, which in
  integer maths truncates a remainder of 0.5 to zero. Once the gap closed to a
  single count the vehicle stopped converging and sat there forever. Forcing a
  minimum step of one count fixes it.
*/
int ramp_towards(int current, int target) {
  int gap = target - current;
  if (gap == 0) return current;

  int step = (int)(gap * RAMP_FACTOR);
  if (step == 0) step = (gap > 0) ? 1 : -1;

  return current + step;
}

// ===================================================================
// MOTOR OUTPUT
// ===================================================================

// Both pins low: the motor freewheels.
void coast_all() {
  for (int i = 0; i < 4; i++) {
    ledcWrite(motors[i].in1_channel, 0);
    ledcWrite(motors[i].in2_channel, 0);
  }
  current_left = current_right = 0;
}

// Both pins high: the motor is shorted across itself and stops hard.
void active_brake_motor(int motor_index) {
  ledcWrite(motors[motor_index].in1_channel, PWM_MAX);
  ledcWrite(motors[motor_index].in2_channel, PWM_MAX);
}

// Full power in one direction, used by the self-test.
void set_motor_full(int motor_index, int direction) {
  if (direction > 0) {
    ledcWrite(motors[motor_index].in1_channel, PWM_MAX);
    ledcWrite(motors[motor_index].in2_channel, 0);
  } else if (direction < 0) {
    ledcWrite(motors[motor_index].in1_channel, 0);
    ledcWrite(motors[motor_index].in2_channel, PWM_MAX);
  } else {
    ledcWrite(motors[motor_index].in1_channel, 0);
    ledcWrite(motors[motor_index].in2_channel, 0);
  }
}

/*
  HYBRID DRIVE

  Rather than pulsing one pin and holding the other low, this holds one pin
  high and pulses the other one low. The DRV8871 spends the off part of each
  cycle braking instead of coasting, which gives noticeably better control at
  low duty cycles.
*/
void set_motor_hybrid(uint8_t in1_ch, uint8_t in2_ch, int duty) {
  duty = constrain(duty, -PWM_MAX, PWM_MAX);

  if (duty == 0) {
    ledcWrite(in1_ch, 0);
    ledcWrite(in2_ch, 0);
    return;
  }

  int inverse = PWM_MAX - abs(duty);
  if (duty > 0) {
    ledcWrite(in1_ch, PWM_MAX);
    ledcWrite(in2_ch, inverse);
  } else {
    ledcWrite(in1_ch, inverse);
    ledcWrite(in2_ch, PWM_MAX);
  }
}

// Left pair and right pair, tank style.
void update_motors() {
  set_motor_hybrid(motors[0].in1_channel, motors[0].in2_channel, current_left);
  set_motor_hybrid(motors[2].in1_channel, motors[2].in2_channel, current_left);
  set_motor_hybrid(motors[1].in1_channel, motors[1].in2_channel, current_right);
  set_motor_hybrid(motors[3].in1_channel, motors[3].in2_channel, current_right);
}

// ===================================================================
// SERVOS
// ===================================================================

/*
  Sends one servo pulse of the requested width.

  The ESP32 counts duty, not microseconds. At 16-bit resolution a whole 20 ms
  cycle is 65536 counts, so counts = microseconds * 65536 / 20000. The long
  cast matters: 2000 * 65536 overflows a 16-bit int and is uncomfortably close
  to the limit of a 32-bit one.
*/
void write_servo(uint8_t channel, int microseconds) {
  microseconds = constrain(microseconds, SERVO_MIN_US, SERVO_MAX_US);
  long duty = (long)microseconds * 65536L / 20000L;
  ledcWrite(channel, duty);
}

/*
  The right stick drives four servos, one per direction. Each servo sweeps its
  full travel across half of the stick's range and sits centred otherwise, so
  a single stick can aim four independent things.
*/
void update_servos(ControllerPtr gp) {
  int rs_y = remap_axis(gp->axisRY(), INPUT_DEADZONE_RS_Y, rs_y_sc, AXIS_MAX, AXIS_MAX);
  int rs_x = remap_axis(gp->axisRX(), INPUT_DEADZONE_RS_X, rs_x_sc, AXIS_MAX, AXIS_MAX);

  // Servo 1: right stick up
  write_servo(SERVO1_CH, rs_y < 0 ? map(rs_y, -AXIS_MAX, 0, SERVO_MAX_US, SERVO_MID_US)
                                  : SERVO_MID_US);
  // Servo 3: right stick down
  write_servo(SERVO3_CH, rs_y > 0 ? map(rs_y, 0, AXIS_MAX, SERVO_MID_US, SERVO_MAX_US)
                                  : SERVO_MID_US);
  // Servo 2: right stick left
  write_servo(SERVO2_CH, rs_x < 0 ? map(rs_x, -AXIS_MAX, 0, SERVO_MAX_US, SERVO_MID_US)
                                  : SERVO_MID_US);
  // Servo 4: right stick right
  write_servo(SERVO4_CH, rs_x > 0 ? map(rs_x, 0, AXIS_MAX, SERVO_MID_US, SERVO_MAX_US)
                                  : SERVO_MID_US);
}

// ===================================================================
// THROTTLE MODES
// ===================================================================

void cycleThrottleMode() {
  if (throttle_mode == MODE_FAST) {
    throttle_mode = MODE_NORMAL;
    active_ls_x_sc = NORMAL_MODE_X_SCALE;
    active_ls_y_sc = NORMAL_MODE_Y_SCALE;
    Serial.println(F("Throttle mode: NORMAL (reduced steering)"));
  } else {
    throttle_mode = MODE_FAST;
    active_ls_x_sc = ls_x_sc;
    active_ls_y_sc = ls_y_sc;
    Serial.println(F("Throttle mode: FAST (full steering)"));
  }
}
