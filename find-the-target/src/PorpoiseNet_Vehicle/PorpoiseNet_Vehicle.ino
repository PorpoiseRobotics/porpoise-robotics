/* =====================================================================
 * PorpoiseNet_Vehicle  --  Find the Target: a hunting rover
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   This is the game piece. It drives around with the rover, and:
 *
 *     - When THIS rover finds the target, it tells EVERY other rover and
 *       the base station, and keeps re-telling them until the base
 *       confirms it heard. That is the whole point of the mission.
 *     - When ANOTHER rover finds the target first, this one hears about
 *       it within a fraction of a second, prints who found it and where,
 *       and flashes so its driver can see it without looking at a screen.
 *     - It answers the base station: start, stop, recall, identify, ping.
 *     - It sends routine telemetry once a second so the dashboard can
 *       plot it on the field map.
 *
 *   Two-way, in one sentence: rovers talk to each other AND to the base,
 *   and the base talks back. There is no master and no server.
 *
 * PUT A DIFFERENT NUMBER IN MY_NODE_ID ON EVERY ROVER.
 *   That is the only edit needed per board. Everything else is identical
 *   on all of them, which is exactly what you want when six students are
 *   flashing six boards ten minutes before a demo.
 *
 * BOARD / IDE SETTINGS
 *   Board:  "ESP32 Dev Module"
 *   Core:   esp32 by Espressif Systems, 2.x or 3.x both work
 *   Serial monitor: 115200 baud
 *
 * LIBRARIES
 *   None required. The optional blocks below need Adafruit NeoPixel and
 *   TinyGPSPlus, and are switched OFF by default so this sketch compiles
 *   on a fresh Arduino install with nothing added.
 *
 * WIRING (all optional except the button)
 *   FIND button : one leg to GPIO 4, other leg to GND. No resistor - the
 *                 ESP32's internal pull-up is switched on in setup().
 *   HC-SR04     : TRIG -> GPIO 32, ECHO -> GPIO 25 THROUGH A DIVIDER.
 *                 ECHO is 5 V and GPIO 25 is 3.3 V: 1k from ECHO to the
 *                 pin, 2k from the pin to GND. Skipping this eventually
 *                 kills the pin.
 *   NeoPixels   : DATA -> GPIO 15 (only if USE_NEOPIXEL is 1)
 *   GPS GT-U7   : GPS TX -> GPIO 16 (only if USE_GPS is 1)
 *
 * HOW TO TELL IT IS WORKING
 *   The serial monitor prints a roster every 5 seconds. Every other
 *   powered board should be in it. If one is missing, that board is the
 *   problem - not this one. See the network notes in PorpoiseNet.h.
 * ===================================================================== */

#if __has_include(<PorpoiseNet.h>)
  #include <PorpoiseNet.h>      // installed in Documents/Arduino/libraries
#else
  #include "PorpoiseNet.h"      // or just copied next to this .ino
#endif

// ======================================================================
// ================  THE THREE LINES YOU MUST GET RIGHT  ================
// ======================================================================
#define MY_NODE_ID   11   // UNIQUE per rover: 11, 12, 13, ... (never 0)
#define MY_NET_ID     7   // SAME on every board on this field
#define MY_CHANNEL    1   // SAME on every board on this field (1, 6 or 11)
// ======================================================================
// Node ID convention across this repo: 1 = base, 10-19 rovers,
// 20-29 cameras, 30-39 fixed sensors. Two boards sharing an ID is the
// single most confusing failure mode there is - the roster shows one
// node that keeps changing its mind. Write the number on the board with
// a marker.
// ======================================================================

#define BASE_NODE_ID  1     // who this rover expects to confirm its find

// ---- Optional hardware. Turn on only what is actually bolted to the rover.
#define USE_SONAR     1     // HC-SR04, no library needed
#define USE_NEOPIXEL  0     // needs the "Adafruit NeoPixel" library
#define USE_GPS       0     // needs the "TinyGPSPlus" library

// ---- Pins ----
#define FIND_BUTTON_PIN  4
#define TRIG_PIN        32
#define ECHO_PIN        25
#define LED_PIN         15   // NeoPixel data
#define NUM_LEDS        16
#define GPS_RX_PIN      16
#define GPS_TX_PIN      18

// ---- Behaviour ----
#define TELEMETRY_EVERY_MS   1000
#define ROSTER_EVERY_MS      5000
#define AUTO_FIND_INCHES     8.0f  // sonar closer than this = a find
#define FIND_COOLDOWN_MS     5000  // ignore repeat finds for this long

#if USE_NEOPIXEL
  #include <Adafruit_NeoPixel.h>
  Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif
#if USE_GPS
  #include <TinyGPSPlus.h>
  TinyGPSPlus    gps;
  HardwareSerial GPSserial(2);
#endif

// ============================== State =================================
bool          hunting        = true;   // false after STOP, true after START
bool          targetFound    = false;  // has ANYONE found it yet this round
uint16_t      foundBy        = 0;      // which node found it
unsigned long lastTelemetry  = 0;
unsigned long lastRoster     = 0;
unsigned long lastFindMs     = 0;
unsigned long flashUntilMs   = 0;      // LEDs held on until this time

// ============================== LEDs ==================================
// Kept behind functions so the sketch reads the same whether the rover
// has a NeoPixel strip or just the little blue on-board LED.
void showColor(uint8_t r, uint8_t g, uint8_t b) {
#if USE_NEOPIXEL
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(r, g, b));
  strip.show();
#else
  digitalWrite(LED_BUILTIN, (r || g || b) ? HIGH : LOW);
#endif
}

void flashFor(unsigned long ms, uint8_t r, uint8_t g, uint8_t b) {
  showColor(r, g, b);
  flashUntilMs = millis() + ms;
}

// ============================== Sonar =================================
// Returns inches, or -1 if no echo came back. -1 is normal: it means
// nothing is within about 5 m, or the sensor is not wired.
float readDistanceInches() {
#if USE_SONAR
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (dur == 0) return -1.0f;
  return dur / 148.0f;         // microseconds there-and-back -> inches
#else
  return -1.0f;
#endif
}

// ===================== Where is this rover? ===========================
// Returns whatever position information the rover actually has. With no
// GPS fitted it reports zeros, and the base station simply plots nothing
// - the find still gets announced, which is the part that matters.
//
// Note this returns a plain struct rather than filling in references to
// the packet's fields. The packet structs are __attribute__((packed)),
// and a packed field may sit at an address the CPU cannot reference
// directly, so C++ refuses to bind a reference to one. Copying whole
// values in and out is the way to work with a packed struct.
struct RoverPosition {
  double latitude;
  double longitude;
  float  headingDeg;
};

RoverPosition readPosition() {
  RoverPosition p = { 0.0, 0.0, 0.0f };
#if USE_GPS
  while (GPSserial.available()) gps.encode(GPSserial.read());
  if (gps.location.isValid()) {
    p.latitude  = gps.location.lat();
    p.longitude = gps.location.lng();
  }
  if (gps.course.isValid()) p.headingDeg = gps.course.deg();
#endif
  return p;
}

// ======================= ANNOUNCING A FIND ============================
// This is the heart of the mission. Note what it does NOT do: it does
// not send to the base and then ask the base to tell the others. It
// broadcasts once, and every rover in range hears it at the same instant.
void announceFind(uint8_t contactType, float rangeInches, const char *note) {
  if (millis() - lastFindMs < FIND_COOLDOWN_MS) return;  // debounce the mission
  lastFindMs = millis();

  PnTargetReport report = {};
  report.contactType = contactType;
  report.confidence  = (rangeInches > 0 && rangeInches < AUTO_FIND_INCHES) ? 90 : 70;
  report.rangeInches = (rangeInches > 0) ? rangeInches : 0.0f;
  RoverPosition here  = readPosition();
  report.latitude     = here.latitude;
  report.longitude    = here.longitude;
  report.headingDeg   = here.headingDeg;
  pnCopyStr(report.note, sizeof(report.note), note);

  Serial.println(F("#"));
  Serial.println(F("# *** TARGET FOUND - telling the whole fleet ***"));
  Serial.print  (F("#     range ")); Serial.print(report.rangeInches, 1);
  Serial.print  (F(" in   lat "));   Serial.print(report.latitude, 6);
  Serial.print  (F("  lon "));       Serial.println(report.longitude, 6);

  // needAck = true: keep resending until the base station confirms.
  // A find that nobody heard is the same as no find at all.
  PorpoiseNet.reportTarget(report, true);

  targetFound = true;
  foundBy     = MY_NODE_ID;
  flashFor(4000, 0, 60, 0);          // green: it was us
}

// ================= EVERY MESSAGE FROM THE NETWORK =====================
// Called from PorpoiseNet.loop(), once per new message. Duplicates and
// echoes have already been filtered out, so anything arriving here is
// genuinely new.
void onNetworkMessage(const PnMessage &msg, int8_t rssi) {
  switch (msg.type) {

    // ---------- another rover found the target ----------
    case PN_TARGET: {
      PnTargetReport r;
      if (!PorpoiseNet.asTarget(msg, r)) return;
      targetFound = true;
      foundBy     = msg.origin;

      Serial.println(F("#"));
      Serial.print  (F("# >>> VEHICLE ")); Serial.print(msg.origin);
      Serial.println(F(" FOUND THE TARGET <<<"));
      Serial.print  (F("#     note: '"));  Serial.print(r.note);
      Serial.print  (F("'  confidence ")); Serial.print(r.confidence);
      Serial.print  (F("%  range "));      Serial.print(r.rangeInches, 1);
      Serial.println(F(" in"));
      if (r.latitude != 0.0) {
        Serial.print(F("#     at lat ")); Serial.print(r.latitude, 6);
        Serial.print(F("  lon "));        Serial.println(r.longitude, 6);
      }
      Serial.print  (F("#     signal ")); Serial.print(rssi);
      Serial.println(F(" dBm - drive toward it or stand down, coach's call."));

      flashFor(4000, 60, 0, 60);     // purple: somebody else got there first
      break;
    }

    // ---------- the base station is telling us to do something ----------
    case PN_COMMAND: {
      PnCommand c;
      if (!PorpoiseNet.asCommand(msg, c)) return;
      switch (c.command) {
        case PN_CMD_START:
          hunting = true; targetFound = false; foundBy = 0;
          Serial.println(F("# COMMAND: START - hunt is on."));
          flashFor(1500, 0, 40, 0);
          break;
        case PN_CMD_STOP:
          hunting = false;
          Serial.println(F("# COMMAND: STOP - stop reporting finds."));
          flashFor(1500, 40, 0, 0);
          break;
        case PN_CMD_RECALL:
          Serial.println(F("# COMMAND: RECALL - drive back to the start line."));
          flashFor(3000, 40, 20, 0);
          break;
        case PN_CMD_RESET:
          targetFound = false; foundBy = 0; lastFindMs = 0;
          Serial.println(F("# COMMAND: RESET - target cleared, new round."));
          break;
        case PN_CMD_IDENTIFY:
          // "Which board is number 13?" - the answer, in light.
          Serial.println(F("# COMMAND: IDENTIFY - flashing."));
          flashFor(5000, 60, 60, 60);
          break;
        case PN_CMD_PING:
          // The two-way test with no hardware at all: the base asks,
          // this rover answers, and the answer comes back over the same
          // radio link. If this works, the network works.
          Serial.print(F("# COMMAND: PING from node ")); Serial.println(msg.origin);
          PorpoiseNet.say("pong from a rover", msg.origin);
          break;
        default:
          Serial.print(F("# COMMAND: unknown code ")); Serial.println(c.command);
          break;
      }
      break;
    }

    // ---------- a camera photographed somebody's event ----------
    case PN_EVIDENCE: {
      PnEvidence e;
      if (!PorpoiseNet.asEvidence(msg, e)) return;
      Serial.print(F("# camera node ")); Serial.print(msg.origin);
      Serial.print(F(" saved '"));       Serial.print(e.name);
      Serial.print(F("' ("));            Serial.print(e.bytes);
      Serial.println(F(" bytes) for that event."));
      break;
    }

    case PN_TEXT:
      Serial.print(F("# node ")); Serial.print(msg.origin);
      Serial.print(F(" says: ")); Serial.println((const char *)msg.payload);
      break;

    // HELLO and TELEMETRY from other nodes arrive constantly and are
    // already reflected in the roster; printing them would bury the
    // messages that matter.
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
  Serial.print  (F("#  PorpoiseNet_Vehicle - rover "));
  Serial.println(MY_NODE_ID);
  Serial.println(F("# ====================================="));

  pinMode(FIND_BUTTON_PIN, INPUT_PULLUP);   // button shorts the pin to GND
#if USE_SONAR
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
#endif
#if USE_NEOPIXEL
  strip.begin(); strip.setBrightness(20); strip.clear(); strip.show();
#else
  pinMode(LED_BUILTIN, OUTPUT);
#endif
#if USE_GPS
  GPSserial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
#endif

  if (!PorpoiseNet.begin(MY_NODE_ID, PN_ROLE_VEHICLE, MY_CHANNEL, MY_NET_ID)) {
    // A rover that cannot talk is not a rover, it is a paperweight.
    Serial.println(F("# Radio failed to start. Halting so the failure is"));
    Serial.println(F("# obvious instead of pretending to work."));
    while (true) { showColor(60, 0, 0); delay(300); showColor(0, 0, 0); delay(300); }
  }
  PorpoiseNet.onMessage(onNetworkMessage);

  // Relaying on: a rover in the middle of the field repeats messages for
  // one at the far end, which is free extra range (see PorpoiseNet.h,
  // STEP 7). Turn it off only if the airtime is genuinely too busy.
  PorpoiseNet.setRelay(true);

  Serial.println(F("# Press the FIND button (GPIO 4 to GND) to announce a find."));
  Serial.println(F("# Ready."));
}

// =============================== Loop =================================
void loop() {
  // Must run every pass. This is where received messages get delivered,
  // relays go out and acknowledgements are chased.
  PorpoiseNet.loop();

  const unsigned long now = millis();

  // ---- LED flash timeout ----
  if (flashUntilMs && (long)(now - flashUntilMs) >= 0) {
    flashUntilMs = 0;
    showColor(0, 0, 0);
  }

  // ---- Did we find it? Two ways: a button, or the sonar. ----
  float inches = readDistanceInches();

  static bool buttonWasDown = false;
  bool buttonDown = (digitalRead(FIND_BUTTON_PIN) == LOW);
  if (buttonDown && !buttonWasDown && hunting) {
    delay(25);                                   // mechanical bounce
    if (digitalRead(FIND_BUTTON_PIN) == LOW) {
      announceFind(1, inches, "spotted by driver");
    }
  }
  buttonWasDown = buttonDown;

  if (hunting && inches > 0 && inches < AUTO_FIND_INCHES) {
    announceFind(2, inches, "sonar contact");
  }

  // ---- Routine telemetry, once a second ----
  if (now - lastTelemetry >= TELEMETRY_EVERY_MS) {
    lastTelemetry = now;
    PnTelemetry t = {};
    t.health      = (inches > 0) ? PN_HEALTH_RANGE_OK : 0;
    t.battPercent = 255;                         // 255 = not measured
    t.rangeInches = (inches > 0) ? inches : 0.0f;
    RoverPosition here = readPosition();
    t.latitude    = here.latitude;
    t.longitude   = here.longitude;
    t.headingDeg  = here.headingDeg;
    if (t.latitude != 0.0) t.health |= PN_HEALTH_GPS_FIX;
    // Telemetry is sent WITHOUT an ack request: it is fine to lose one,
    // another arrives in a second. Acks are for events, not for streams.
    PorpoiseNet.sendTelemetry(t, BASE_NODE_ID);
  }

  // ---- Who else is out there? ----
  if (now - lastRoster >= ROSTER_EVERY_MS) {
    lastRoster = now;
    PorpoiseNet.printRoster();
    if (targetFound) {
      Serial.print(F("# round status: target already found by node "));
      Serial.println(foundBy);
    }
  }
}
