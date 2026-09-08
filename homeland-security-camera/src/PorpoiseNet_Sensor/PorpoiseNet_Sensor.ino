/* =====================================================================
 * PorpoiseNet_Sensor  --  a fixed perimeter sensor node
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   A bare ESP32 with a motion sensor on one pin and a door switch on
 *   another. When either trips it broadcasts one PN_ALERT. Two things
 *   then happen by themselves, with no server in the middle:
 *     - the camera node hears it and takes a picture,
 *     - the base station hears it, prints it, and confirms receipt.
 *   The sensor then hears the camera's receipt come back, so it knows
 *   its alert was acted on. That is the two-way loop, closed, with three
 *   boards and no infrastructure.
 *
 *   It also obeys ARM and DISARM from the base station, which is why it
 *   listens as well as talks.
 *
 * WHY THIS IS A SEPARATE BOARD FROM THE CAMERA
 *   Money and pins. A plain ESP32 costs a few dollars, so a perimeter
 *   can have six of them; cameras are dearer and one can cover several
 *   sensors. And the camera board has almost no spare GPIO left (see the
 *   pin budget in PorpoiseNet_CameraNode.ino), whereas this board has
 *   twenty. Splitting them is the cheaper AND simpler option.
 *
 * BOARD / IDE SETTINGS
 *   Board: "ESP32 Dev Module"    Serial monitor: 115200 baud
 *   No libraries required.
 *
 * WIRING
 *   PIR (HC-SR501): VCC -> 5V, GND -> GND, OUT -> GPIO 27.
 *     Its output is 3.3 V, so no divider is needed. It has two little
 *     orange trimmers: sensitivity and how long the output stays high.
 *     Turn the time trimmer to minimum while testing or you will spend
 *     an afternoon wondering why it only triggers once.
 *     A PIR also needs 30-60 seconds after power-up to settle. This
 *     sketch waits that out and says so rather than firing false alerts.
 *   Door / reed switch: one leg to GPIO 26, the other to GND. The
 *     internal pull-up is used, so closed = LOW = door shut.
 *   Status LED: the on-board one. Blinks on a trip.
 *
 * ZONES: USE A NUMBER, NOT A NAME
 *   MY_ZONE below is an integer and should stay one. Every ESP-NOW
 *   message is unencrypted and readable by anyone nearby, so "zone 3"
 *   is the right amount to broadcast and "back door by the gym" is not.
 *   Keep the number-to-place mapping on paper, off this repository, and
 *   off the air.
 *
 * BATTERY LIFE, IF THIS EVER RUNS ON ONE
 *   As written this board listens all the time, which is what lets the
 *   base station arm and disarm it. That costs roughly 80-120 mA, so a
 *   2000 mAh pack lasts under a day. ESP32 deep sleep would stretch that
 *   to months - but a sleeping radio hears nothing, so it could no
 *   longer be disarmed remotely, and a PIR trip would take ~300 ms to
 *   wake and send. That is a real design decision with a real trade-off,
 *   not an oversight: mains power keeps the two-way behaviour, battery
 *   power buys life by giving it up. Decide it deliberately.
 * ===================================================================== */

#if __has_include(<PorpoiseNet.h>)
  #include <PorpoiseNet.h>
#else
  #include "PorpoiseNet.h"
#endif

// ======================================================================
// ================  THE THREE LINES YOU MUST GET RIGHT  ================
// ======================================================================
#define MY_NODE_ID   31   // fixed sensors are 30-39 by convention
#define MY_NET_ID     7   // SAME on every board on this site
#define MY_CHANNEL    1   // SAME on every board on this site
// ======================================================================

#define MY_ZONE       3   // a number. Not a name. See the note above.

// ---- Pins ----
#define PIR_PIN      27
#define DOOR_PIN     26
#define STATUS_LED   LED_BUILTIN

// ---- Behaviour ----
#define USE_PIR              1
#define USE_DOOR_SWITCH      1
#define PIR_WARMUP_MS        40000   // PIRs lie for the first half minute
#define ALERT_COOLDOWN_MS     4000   // one alert per this window, per sensor
#define ROSTER_EVERY_MS      15000
#define HEARTBEAT_EVERY_MS   30000   // "still here" telemetry

// ============================== State =================================
bool          armed         = true;
uint32_t      pirTrips      = 0;
uint32_t      doorTrips     = 0;
unsigned long lastPirAlert  = 0;
unsigned long lastDoorAlert = 0;
unsigned long lastRoster    = 0;
unsigned long lastHeartbeat = 0;
unsigned long warmupUntil   = 0;
bool          warmupDone    = false;

// ============================ Raise an alert ==========================
void raise(uint8_t sensorKind, uint8_t severity, uint32_t count, const char *note) {
  PnAlert a = {};
  a.sensor   = sensorKind;
  a.severity = severity;
  a.zone     = MY_ZONE;
  a.count    = count;
  a.value    = 0.0f;
  pnCopyStr(a.note, sizeof(a.note), note);

  Serial.println(F("#"));
  Serial.print  (F("# *** ALERT: "));   Serial.print(note);
  Serial.print  (F("  (zone "));        Serial.print(MY_ZONE);
  Serial.print  (F(", trip #"));        Serial.print(count);
  Serial.println(F(") ***"));

  // needAck: keep resending until the base station confirms. An
  // intrusion alert that vanished into a noisy afternoon is worse than
  // no sensor at all, because everyone believes the sensor is working.
  PorpoiseNet.raiseAlert(a, true);

  // Three quick blinks so a person standing next to it can see it fired.
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH); delay(60);
    digitalWrite(STATUS_LED, LOW);  delay(60);
  }
}

// ====================== messages from the network =====================
void onNetworkMessage(const PnMessage &msg, int8_t rssi) {
  (void)rssi;
  switch (msg.type) {

    case PN_COMMAND: {
      PnCommand c;
      if (!PorpoiseNet.asCommand(msg, c)) return;
      switch (c.command) {
        case PN_CMD_ARM:
          armed = true;
          Serial.println(F("# COMMAND: ARM - watching."));
          break;
        case PN_CMD_DISARM:
          // Deliberately loud. A disarmed sensor that everybody thinks
          // is armed is the most dangerous state this system has.
          armed = false;
          Serial.println(F("# COMMAND: DISARM - NOT watching. Trips are ignored."));
          break;
        case PN_CMD_PING:
          PorpoiseNet.say(armed ? "sensor armed" : "sensor DISARMED", msg.origin);
          break;
        case PN_CMD_IDENTIFY:
          Serial.println(F("# COMMAND: IDENTIFY - blinking."));
          for (int i = 0; i < 10; i++) {
            digitalWrite(STATUS_LED, HIGH); delay(100);
            digitalWrite(STATUS_LED, LOW);  delay(100);
          }
          break;
        default:
          break;
      }
      break;
    }

    // ---------- the camera reporting back on OUR alert ----------
    // This is the half of the conversation that a one-way sensor never
    // gets: confirmation that something happened as a result.
    case PN_EVIDENCE: {
      PnEvidence e;
      if (!PorpoiseNet.asEvidence(msg, e)) return;
      if (e.aboutOrigin == MY_NODE_ID) {
        Serial.print(F("# camera "));  Serial.print(msg.origin);
        Serial.print(F(" photographed our alert -> '"));
        Serial.print(e.name);          Serial.print(F("' ("));
        Serial.print(e.bytes);         Serial.println(F(" bytes)"));
      }
      break;
    }

    case PN_TEXT:
      Serial.print(F("# node ")); Serial.print(msg.origin);
      Serial.print(F(" says: ")); Serial.println((const char *)msg.payload);
      break;

    default:
      break;
  }
}

// ============================== Setup =================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("# ====================================="));
  Serial.print  (F("#  PorpoiseNet_Sensor - node "));
  Serial.print(MY_NODE_ID);
  Serial.print  (F(", zone "));
  Serial.println(MY_ZONE);
  Serial.println(F("# ====================================="));

  pinMode(STATUS_LED, OUTPUT);
#if USE_PIR
  pinMode(PIR_PIN, INPUT);
#endif
#if USE_DOOR_SWITCH
  pinMode(DOOR_PIN, INPUT_PULLUP);
#endif

  if (!PorpoiseNet.begin(MY_NODE_ID, PN_ROLE_SENSOR, MY_CHANNEL, MY_NET_ID)) {
    Serial.println(F("# Radio failed to start - halting."));
    while (true) { digitalWrite(STATUS_LED, HIGH); delay(200);
                   digitalWrite(STATUS_LED, LOW);  delay(200); }
  }
  PorpoiseNet.onMessage(onNetworkMessage);

  warmupUntil = millis() + PIR_WARMUP_MS;
  Serial.print(F("# PIR warming up for "));
  Serial.print(PIR_WARMUP_MS / 1000);
  Serial.println(F("s - trips are ignored until then, on purpose."));
}

// =============================== Loop =================================
void loop() {
  PorpoiseNet.loop();
  const unsigned long now = millis();

  if (!warmupDone && (long)(now - warmupUntil) >= 0) {
    warmupDone = true;
    Serial.println(F("# PIR settled. Watching."));
  }

#if USE_PIR
  // Edge-triggered: alert when the PIR goes high, not while it stays
  // high. An HC-SR501 holds its output up for seconds after motion.
  static bool pirWas = false;
  bool pirNow = (digitalRead(PIR_PIN) == HIGH);
  if (pirNow && !pirWas && armed && warmupDone &&
      (now - lastPirAlert) > ALERT_COOLDOWN_MS) {
    lastPirAlert = now;
    raise(PN_SENSOR_MOTION, 2, ++pirTrips, "motion");
  }
  pirWas = pirNow;
#endif

#if USE_DOOR_SWITCH
  // Pull-up wiring: LOW = contact closed = door shut. Going HIGH means
  // the magnet moved away from the reed switch, i.e. the door opened.
  static bool doorWasOpen = false;
  bool doorOpen = (digitalRead(DOOR_PIN) == HIGH);
  if (doorOpen && !doorWasOpen && armed &&
      (now - lastDoorAlert) > ALERT_COOLDOWN_MS) {
    lastDoorAlert = now;
    raise(PN_SENSOR_DOOR, 3, ++doorTrips, "door opened");
  }
  doorWasOpen = doorOpen;
#endif

  // ---- Heartbeat telemetry: proof of life, not an event ----
  // Without this, "no alerts all night" and "the board died at 9pm" look
  // identical from the base station. They are not the same thing.
  if (now - lastHeartbeat >= HEARTBEAT_EVERY_MS) {
    lastHeartbeat = now;
    PnTelemetry t = {};
    t.health      = armed ? PN_HEALTH_BATT_OK : 0;
    t.battPercent = 255;
    PorpoiseNet.sendTelemetry(t);
  }

  if (now - lastRoster >= ROSTER_EVERY_MS) {
    lastRoster = now;
    PorpoiseNet.printRoster();
    Serial.print(F("# state "));      Serial.print(armed ? F("ARMED") : F("DISARMED"));
    Serial.print(F("  motion trips "));Serial.print(pirTrips);
    Serial.print(F("  door trips "));  Serial.println(doorTrips);
  }
}
