/*
  Lighting.ino - everything that touches the 32-LED strip.

  11.2 used FastLED, which gave us CRGB, fill_solid, nscale8 and sin8 for free.
  This program uses Adafruit NeoPixel instead, so the handful of helpers we
  actually relied on are written out here. They are short, and having them in
  the open makes the colour maths visible rather than magic.
*/

// ===================================================================
// COLOUR HELPERS
// ===================================================================

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return strip.Color(r, g, b);
}

// Scales a packed colour towards black. 255 leaves it alone, 0 turns it off.
uint32_t scale_colour(uint32_t colour, uint8_t scale) {
  uint8_t r = (uint8_t)(colour >> 16);
  uint8_t g = (uint8_t)(colour >> 8);
  uint8_t b = (uint8_t)colour;
  return strip.Color((r * scale) / 255, (g * scale) / 255, (b * scale) / 255);
}

// Fills a run of LEDs with one colour. The NeoPixel equivalent of fill_solid.
void fill_range(int first, int count, uint32_t colour) {
  for (int i = first; i < first + count && i < NUM_LEDS; i++) {
    strip.setPixelColor(i, colour);
  }
}

/*
  An 8-bit sine, the same shape FastLED's sin8() produced: feed it 0..255 for
  one full turn and it returns 0..255 with 128 as the midpoint. Used by the
  breathing and pulsing animations.
*/
uint8_t sine8(uint8_t theta) {
  float radians = (theta / 256.0f) * 2.0f * PI;
  return (uint8_t)(128.0f + 127.0f * sinf(radians));
}

// ===================================================================
// STATIC LIGHTING STATES
// ===================================================================

// A soft green glow while no controller has been found yet.
void set_disconnected_lighting() {
  fill_range(0, NUM_LEDS, rgb(0, 38, 0));
  strip.show();
}

/*
  Normal driving lights: white across the front, red across the rear.
  Only redrawn when something actually changed. 11.2 pushed this to the strip
  on every single pass of loop(), which is a great deal of pointless work.
*/
void set_standby_lighting() {
  strip.clear();

  if (running_lights_on) {
    uint8_t level = full_headlights ? HEADLIGHT_BRIGHT : HEADLIGHT_DIM;
    fill_range(LEFT_FRONT_START, CORNER_LEN * 2, rgb(level, level, level));
    fill_range(RIGHT_REAR_START, CORNER_LEN * 2, rgb(TAILLIGHT_LEVEL, 0, 0));
  }

  strip.show();
  standby_dirty = false;
}

// ===================================================================
// ANIMATIONS
// ===================================================================

// Green pulse while searching for a controller.
void update_disconnected_lighting(unsigned long now) {
  static unsigned long last_update = 0;
  if (now - last_update < 10) return;
  last_update = now;

  uint8_t level = map(sine8(now / 16), 0, 255, 30, 80);
  fill_range(0, NUM_LEDS, rgb(0, level, 0));
  strip.show();
}

// Blue breathing while waiting for a controller to pair.
void update_pairing_breathing(unsigned long now) {
  if (now - breathe_time < 20) return;
  breathe_time = now;

  uint8_t phase = (uint8_t)((now % BREATHE_PERIOD_MS) * 255 / BREATHE_PERIOD_MS);
  uint8_t level = map(sine8(phase), 0, 255, 0, BREATHE_MAX);

  fill_range(0, NUM_LEDS, scale_colour(rgb(0, 50, 255), level));
  strip.show();
}

/*
  Turn signals, as a bar of amber that grows outward from the centre of the
  vehicle towards the corner that is turning. Front and rear animate together.
*/
void update_larson_scanner() {
  uint32_t amber = rgb(255, 100, 0);

  // Repaint the base lighting first, so the signal overlays normal lights.
  strip.clear();
  if (running_lights_on) {
    uint8_t level = full_headlights ? HEADLIGHT_BRIGHT : HEADLIGHT_DIM;
    fill_range(LEFT_FRONT_START, CORNER_LEN * 2, rgb(level, level, level));
    fill_range(RIGHT_REAR_START, CORNER_LEN * 2, rgb(TAILLIGHT_LEVEL, 0, 0));
  }

  int num_lit = (larson_phase % CORNER_LEN) + 1;

  if (led_mode == TURN_LEFT) {
    fill_range(LEFT_FRONT_START, CORNER_LEN, 0);
    fill_range(LEFT_REAR_START, CORNER_LEN, 0);
    for (int i = 0; i < num_lit; i++) {
      strip.setPixelColor(RIGHT_FRONT_START - 1 - i, amber);   // Front, centre outward
      strip.setPixelColor(LEFT_REAR_START + i, amber);         // Rear, mirrored
    }
  } else if (led_mode == TURN_RIGHT) {
    fill_range(RIGHT_FRONT_START, CORNER_LEN, 0);
    fill_range(RIGHT_REAR_START, CORNER_LEN, 0);
    for (int i = 0; i < num_lit; i++) {
      strip.setPixelColor(RIGHT_FRONT_START + i, amber);
      strip.setPixelColor(LEFT_REAR_START - 1 - i, amber);
    }
  }

  strip.show();
}

/*
  KITT scanner. One bright dot sweeps across the front with a fading tail
  behind it, and the matching rear dot is drawn at REAR_LAST - position so the
  two stay side by side. 11.2 drew a bare dot with no tail.
*/
void update_kitt_scanner() {
  strip.clear();

  uint32_t base = scale_colour(rgb(KITT_R, KITT_G, KITT_B), KITT_BRIGHTNESS);

  for (int tail = 0; tail <= KITT_TAIL; tail++) {
    int pos = kitt_pos - (kitt_dir * tail);
    if (pos < 0 || pos > FRONT_LAST) continue;

    uint32_t colour = scale_colour(base, 255 >> tail);
    strip.setPixelColor(pos, colour);
    strip.setPixelColor(REAR_LAST - pos, colour);
  }

  strip.show();
}

void start_kitt_scanner() {
  // Remember what we were showing so we can go back to it afterwards.
  if (led_mode != KITT_SCANNER) kitt_return_mode = led_mode;

  led_mode = KITT_SCANNER;
  kitt_time = 0;
  kitt_pos = 0;
  kitt_dir = 1;
  kitt_cycles_done = 0;
}

// ===================================================================
// THE LIGHTING STATE MACHINE
// ===================================================================

/*
  Called once per pass of loop(). Everything that animates is driven from here
  off millis(), so no animation ever blocks the vehicle from driving.
*/
void update_lighting(unsigned long now, uint8_t dpad) {

  if (led_mode == PAIRING) {
    update_pairing_breathing(now);
    return;
  }

  if (led_mode == TURN_LEFT || led_mode == TURN_RIGHT) {
    if (now - larson_time < LARSON_DELAY_MS) return;
    larson_time = now;

    larson_phase++;
    if (larson_phase >= CORNER_LEN) {
      larson_phase = 0;
      turn_signal_cycles++;

      // Keep going while the D-pad is held; otherwise stop after TURN_CYCLES.
      bool still_held = (led_mode == TURN_LEFT  && (dpad & DPAD_LEFT)) ||
                        (led_mode == TURN_RIGHT && (dpad & DPAD_RIGHT));
      if (!still_held && turn_signal_cycles >= TURN_CYCLES) {
        led_mode = STANDBY;
        standby_dirty = true;
        return;
      }
    }
    update_larson_scanner();
    return;
  }

  if (led_mode == KITT_SCANNER) {
    if (now - kitt_time < (unsigned long)KITT_SPEED_MS) return;
    kitt_time = now;

    update_kitt_scanner();

    kitt_pos += kitt_dir;
    if (kitt_pos >= FRONT_LAST) {
      kitt_pos = FRONT_LAST;
      kitt_dir = -1;
    } else if (kitt_pos <= 0) {
      kitt_pos = 0;
      kitt_dir = 1;

      kitt_cycles_done++;
      if (kitt_cycles_done >= KITT_CYCLES) {
        // Turn signals do not survive a scanner run; anything else does.
        led_mode = (kitt_return_mode == TURN_LEFT || kitt_return_mode == TURN_RIGHT)
                   ? STANDBY : kitt_return_mode;
        standby_dirty = true;
      }
    }
    return;
  }

  if (led_mode == STANDBY && standby_dirty) {
    set_standby_lighting();
  }
}
