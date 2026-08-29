/*
  SelfTest.ino - the powered-up motor check, Gen 3 vehicles only.

  Jumper GPIO 34 high at boot and each motor is driven forward and then in
  reverse while the current sensor watches. A motor that is disconnected draws
  almost nothing; a motor that is jammed draws a lot and keeps drawing it. A
  healthy one spikes as it breaks away, then settles.

  The whole thing is a state machine driven from loop(), so the LEDs keep
  animating and the serial console keeps responding throughout.

  WHAT CHANGED FROM 11.2
  11.2 measured a baseline current with the motors off, printed it, and then
  compared each motor against fixed absolute thresholds, ignoring the baseline
  entirely. Here both thresholds are measured relative to that baseline, so a
  vehicle whose electronics idle at half an amp is judged fairly.
*/

void run_self_test() {
  unsigned long now = millis();
  static bool    blink_state = false;
  static uint8_t sub_phase = 0;          // 0 spinning, 1 braking
  static bool    result_announced = false;

  // Any button press clears the pass/fail screen and hands over to driving.
  if (test_state == TEST_RESULT) {
    ControllerPtr gp = firstConnectedController();
    if (gp && gp->buttons() != 0) {
      Serial.println(F("Self-test cleared. Entering DRIVE mode."));
      test_state = TEST_DISABLED;
      result_announced = false;
      led_mode = STANDBY;
      standby_dirty = true;
      return;
    }
  }

  switch (test_state) {

    case CHECK_TRIGGER:
      // Give the pin a moment to settle before believing it.
      if (now >= TEST_START_DELAY) {
        bool jumpered = (digitalRead(TEST_PIN) == HIGH);
        test_state = (jumpered && (vehicle_config.capabilities & CAP_SELF_TEST))
                     ? START_TEST : TEST_DISABLED;
      }
      break;

    case START_TEST:
      Serial.println(F("\n--- STARTING SELF-TEST ---"));
      Serial.printf("VBat: %.2fV\n", get_bus_voltage());
      current_test_step = 0;
      sub_phase = 0;
      self_test_passed = true;
      result_announced = false;
      reset_current_tracking();
      test_phase_start = now;
      test_state = BASELINE_CURRENT;
      break;

    case BASELINE_CURRENT:
      // Measure what the vehicle draws with everything stopped.
      update_ina219();
      if (now - test_phase_start >= BASELINE_DURATION) {
        baselineCurrent = get_average_current();
        Serial.printf("Baseline current: %.3fA\n", baselineCurrent);
        reset_current_tracking();
        test_phase_start = now;
        test_state = RUNNING_TESTS;
      }
      break;

    case RUNNING_TESTS:
      if (current_test_step >= NUM_TEST_STEPS) {
        test_phase_start = now;
        test_state = TEST_RESULT;
        break;
      }

      if (sub_phase == 0) {
        // ---- Spinning ----
        set_motor_full(test_sequence[current_test_step].motor_index,
                       test_sequence[current_test_step].direction);
        update_ina219();

        if (now - test_phase_start > SPIN_DURATION) {
          float avg  = get_average_current();
          float peak_above = current_peak_pos - baselineCurrent;
          float avg_above  = avg - baselineCurrent;

          Serial.printf("TEST [%s] -> I-Avg: %.3fA (%+.3f) | I-Max: %.3fA (%+.3f)",
                        test_sequence[current_test_step].name,
                        avg, avg_above, current_peak_pos, peak_above);

          // Healthy: a clear spike on break-away, then settles back down.
          bool healthy = (peak_above > PEAK_ABOVE_BASELINE_A) &&
                         (avg_above  < AVG_ABOVE_BASELINE_A);

          if (healthy) {
            Serial.println(F(" | PASS"));
            sub_phase = 1;
            test_phase_start = now;
          } else {
            Serial.println(F(" | FAIL"));
            Serial.println(peak_above <= PEAK_ABOVE_BASELINE_A
                           ? F("  No current spike: motor disconnected or not turning?")
                           : F("  Current stayed high: motor jammed or binding?"));
            coast_all();
            self_test_passed = false;
            test_phase_start = now;
            test_state = TEST_RESULT;
          }
        }
      } else {
        // ---- Braking, so the next motor starts from standstill ----
        active_brake_motor(test_sequence[current_test_step].motor_index);
        if (now - test_phase_start > BRAKE_DURATION) {
          coast_all();
          current_test_step++;
          sub_phase = 0;
          reset_current_tracking();
          test_phase_start = now;
        }
      }
      break;

    case TEST_RESULT:
      if (!result_announced) {
        Serial.printf("--- SELF-TEST %s ---\n", self_test_passed ? "PASSED" : "FAILED");
        Serial.println(F("Press any button on the controller to drive."));
        result_announced = true;
      }
      // Blink green for pass, red for fail, until somebody acknowledges it.
      if (now - test_phase_start >= PASS_BLINK_DELAY) {
        test_phase_start = now;
        blink_state = !blink_state;
        uint32_t colour = blink_state ? (self_test_passed ? rgb(0, 255, 0) : rgb(255, 0, 0))
                                      : rgb(0, 0, 0);
        fill_range(0, NUM_LEDS, colour);
        strip.show();
      }
      break;

    case TEST_DISABLED:
      break;
  }
}
