/*
  Blinker.ino - one subsystem, in its own tab.

  Everything that touches the LED lives here and nowhere else. The main sketch
  calls setup_blinker() once and update_blinker() every pass, and never has to
  know how either of them works.

  Notice what this file does NOT have:
    - no #include of Config.h. The main sketch already included it, and the IDE
      concatenates the tabs into one file before compiling.
    - no declaration of blink_mode. It is a global in the main sketch, so it is
      visible from here.
    - no prototypes for its own functions. The IDE generates those.

  That convenience is also the trap: because every tab can see every global,
  nothing stops you reaching into another subsystem and changing its variables
  from the wrong place. The discipline that keeps a program like Op 12
  readable is a convention, not a rule the compiler enforces - each tab owns
  its own state, and other tabs go through its functions.
*/

// State that belongs to this subsystem alone. "static" at file scope keeps a
// variable private to this tab, which is how you say "nobody else should touch
// this" in a language that will not stop them.
static unsigned long last_blink = 0;
static unsigned long last_mode_change = 0;
static bool led_is_on = false;

void setup_blinker() {
  pinMode(LED_PIN, OUTPUT);      // LED_PIN comes from Config.h
  digitalWrite(LED_PIN, LOW);
}

/*
  Non-blocking: called every pass of loop(), does something only when it is
  time to. Nothing in this sketch ever calls delay(), which is what lets the
  blinker and the reporter run on their own separate schedules.
*/
void update_blinker(unsigned long now) {
  // Swap between slow and fast every few seconds, so there is something to see.
  if (now - last_mode_change >= MODE_SWITCH_MS) {
    last_mode_change = now;
    blink_mode = (blink_mode == BLINK_SLOW) ? BLINK_FAST : BLINK_SLOW;
  }

  unsigned long interval = (blink_mode == BLINK_SLOW) ? BLINK_SLOW_MS
                                                      : BLINK_FAST_MS;

  if (now - last_blink >= interval) {
    last_blink = now;
    led_is_on = !led_is_on;
    digitalWrite(LED_PIN, led_is_on ? HIGH : LOW);
  }
}
