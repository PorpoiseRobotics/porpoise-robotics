/* =====================================================================
 * PorpoiseNet_Base  --  the board on the laptop's USB port
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   It is the network's ears and mouth on the operator's side:
 *
 *   INBOUND   every message from every rover, camera and sensor is
 *             printed to the serial port. Telemetry also goes out as one
 *             line of JSON, in the same shape base_station.py already
 *             reads, so the existing dashboard keeps working unchanged.
 *   OUTBOUND  anything typed into the serial monitor becomes a command
 *             broadcast to the fleet. Type "recall" and every rover is
 *             told to come home. That is the second half of "two-way",
 *             and it is the half the old ESPNow_Receiver_v3 never had.
 *   CONFIRMING this node is the one that answers acknowledgement
 *             requests. When a rover announces a find, the find is not
 *             considered delivered until this board replies. That is
 *             what stops a find being lost to a burst of interference.
 *
 * BOARD / IDE SETTINGS
 *   Board:  "ESP32 Dev Module"
 *   Core:   esp32 by Espressif Systems, 2.x or 3.x
 *   Serial monitor: 115200 baud, line ending "Newline"
 *   No libraries required.
 *
 * SERIAL OUTPUT FORMAT (unchanged from v3, on purpose)
 *   Lines starting "# " are for humans. Lines starting "{" are JSON for
 *   base_station.py. Both are visible in the Arduino serial monitor, so
 *   the same output serves debugging and the web page.
 *
 * TYPE "help" AND PRESS ENTER for the command list.
 *
 * IMPORTANT: only one program at a time can hold the USB port. Close the
 * Arduino serial monitor before starting base_station.py, or the Python
 * side will report "no port found" while looking straight at it.
 * ===================================================================== */

#if __has_include(<PorpoiseNet.h>)
  #include <PorpoiseNet.h>
#else
  #include "PorpoiseNet.h"
#endif

// ======================================================================
// ================  THE THREE LINES YOU MUST GET RIGHT  ================
// ======================================================================
#define MY_NODE_ID    1   // the base is always 1 by convention
#define MY_NET_ID     7   // SAME on every board on this field
#define MY_CHANNEL    1   // SAME on every board on this field
// ======================================================================

#define ROSTER_EVERY_MS   10000

// Emit a JSON line for events (finds, alerts) as well as telemetry?
// Leave 0 unless base_station.py has been taught to tell them apart:
// today it treats every JSON line as a telemetry packet, so event lines
// would overwrite the vehicle's readings on the dashboard.
#define EMIT_EVENT_JSON   0

unsigned long lastRoster = 0;
uint32_t      findCount  = 0;
uint32_t      alertCount = 0;

// ========================== JSON telemetry ============================
// Key names match what base_station.py and dashboard.html already
// expect. If you rename one here, rename it there in the same commit or
// the dashboard goes blank with no error message.
void printTelemetryJson(const PnMessage &msg, const PnTelemetry &t, int8_t rssi) {
  Serial.print(F("{\"node\":"));          Serial.print(msg.origin);
  Serial.print(F(",\"seq\":"));           Serial.print(msg.seq);
  Serial.print(F(",\"uptime_ms\":"));     Serial.print(msg.originMs);
  Serial.print(F(",\"sonar_ok\":"));      Serial.print((t.health & PN_HEALTH_RANGE_OK) ? 1 : 0);
  Serial.print(F(",\"gps_fix\":"));       Serial.print((t.health & PN_HEALTH_GPS_FIX)  ? 1 : 0);
  Serial.print(F(",\"distance_in\":"));   Serial.print(t.rangeInches, 1);
  Serial.print(F(",\"temp_f\":"));        Serial.print(t.tempF, 1);
  Serial.print(F(",\"sats\":"));          Serial.print(t.satellites);
  Serial.print(F(",\"lat\":"));           Serial.print(t.latitude, 6);
  Serial.print(F(",\"lon\":"));           Serial.print(t.longitude, 6);
  Serial.print(F(",\"speed_mph\":"));     Serial.print(t.speedMph, 1);
  Serial.print(F(",\"course_deg\":"));    Serial.print(t.headingDeg, 1);
  Serial.print(F(",\"batt_pct\":"));      Serial.print(t.battPercent);
  Serial.print(F(",\"rssi\":"));          Serial.print(rssi);
  Serial.println(F("}"));
}

// ====================== messages from the field =======================
void onNetworkMessage(const PnMessage &msg, int8_t rssi) {
  switch (msg.type) {

    case PN_TELEMETRY: {
      PnTelemetry t;
      if (!PorpoiseNet.asTelemetry(msg, t)) return;
      printTelemetryJson(msg, t, rssi);
      break;
    }

    // ---------- THE GAME EVENT ----------
    // PorpoiseNet has already sent the acknowledgement back to the rover
    // by the time this runs, because this node is the ack responder.
    case PN_TARGET: {
      PnTargetReport r;
      if (!PorpoiseNet.asTarget(msg, r)) return;
      findCount++;
      Serial.println(F("# ==================================="));
      Serial.print  (F("#  TARGET FOUND by vehicle "));  Serial.println(msg.origin);
      Serial.print  (F("#  find #"));                    Serial.print(findCount);
      Serial.print  (F("   confidence "));               Serial.print(r.confidence);
      Serial.print  (F("%   range "));                   Serial.print(r.rangeInches, 1);
      Serial.println(F(" in"));
      Serial.print  (F("#  note: "));                    Serial.println(r.note);
      if (r.latitude != 0.0) {
        Serial.print(F("#  position lat ")); Serial.print(r.latitude, 6);
        Serial.print(F("  lon "));           Serial.print(r.longitude, 6);
        Serial.print(F("  heading "));       Serial.print(r.headingDeg, 0);
        Serial.println(F(" deg"));
      } else {
        Serial.println(F("#  position: no GPS fix on that vehicle"));
      }
      Serial.print  (F("#  signal ")); Serial.print(rssi);
      Serial.println(F(" dBm   (acknowledged)"));
      Serial.println(F("# ==================================="));
#if EMIT_EVENT_JSON
      Serial.print(F("{\"event\":\"target\",\"node\":")); Serial.print(msg.origin);
      Serial.print(F(",\"lat\":"));  Serial.print(r.latitude, 6);
      Serial.print(F(",\"lon\":"));  Serial.print(r.longitude, 6);
      Serial.println(F("}"));
#endif
      break;
    }

    // ---------- THE SECURITY EVENT ----------
    // The same base station serves the camera project. Nothing here is
    // game-specific; a sensor tripping and a rover finding a cone are
    // the same kind of thing to this board.
    case PN_ALERT: {
      PnAlert a;
      if (!PorpoiseNet.asAlert(msg, a)) return;
      alertCount++;
      Serial.println(F("# ==================================="));
      Serial.print  (F("#  ALERT from node "));  Serial.print(msg.origin);
      Serial.print  (F("  zone "));              Serial.println(a.zone);
      Serial.print  (F("#  sensor type "));      Serial.print(a.sensor);
      Serial.print  (F("   severity "));         Serial.print(a.severity);
      Serial.print  (F("   trip #"));            Serial.println(a.count);
      Serial.print  (F("#  note: "));            Serial.println(a.note);
      Serial.print  (F("#  signal "));           Serial.print(rssi);
      Serial.println(F(" dBm   (acknowledged)"));
      Serial.println(F("# ==================================="));
      break;
    }

    case PN_EVIDENCE: {
      PnEvidence e;
      if (!PorpoiseNet.asEvidence(msg, e)) return;
      Serial.print(F("# EVIDENCE from camera "));  Serial.print(msg.origin);
      Serial.print(F(": '"));                      Serial.print(e.name);
      Serial.print(F("'  "));                      Serial.print(e.bytes);
      Serial.print(F(" bytes  result "));          Serial.print(e.result);
      Serial.print(F("  (for node "));             Serial.print(e.aboutOrigin);
      Serial.print(F(" message #"));               Serial.print(e.aboutSeq);
      Serial.println(F(")"));
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

// ===================== commands typed by the operator =================
void printHelp() {
  Serial.println(F("# ---- commands (type one, press Enter) ----"));
  Serial.println(F("#   start        every rover: begin the hunt"));
  Serial.println(F("#   stop         every rover: stop reporting finds"));
  Serial.println(F("#   recall       every rover: come back to the start line"));
  Serial.println(F("#   reset        clear the found-target state, new round"));
  Serial.println(F("#   arm          security sensors: start reporting"));
  Serial.println(F("#   disarm       security sensors: stay quiet"));
  Serial.println(F("#   snapshot     cameras: take a picture now"));
  Serial.println(F("#   ping         ask every node to answer"));
  Serial.println(F("#   ping 12      ask node 12 to answer"));
  Serial.println(F("#   id 12        make node 12 flash so you can find it"));
  Serial.println(F("#   roster       list every node heard recently"));
  Serial.println(F("#   help         this list"));
}

void handleTypedCommand(String line) {
  line.trim();
  line.toLowerCase();
  if (line.length() == 0) return;

  // Split "ping 12" into a word and an optional number.
  String word = line;
  uint16_t arg = PN_BROADCAST_ID;
  int space = line.indexOf(' ');
  if (space > 0) {
    word = line.substring(0, space);
    arg  = (uint16_t)line.substring(space + 1).toInt();
  }

  if      (word == "help")   { printHelp(); return; }
  else if (word == "roster") { PorpoiseNet.printRoster(); return; }

  uint8_t cmd = 0;
  if      (word == "start")    cmd = PN_CMD_START;
  else if (word == "stop")     cmd = PN_CMD_STOP;
  else if (word == "recall")   cmd = PN_CMD_RECALL;
  else if (word == "reset")    cmd = PN_CMD_RESET;
  else if (word == "arm")      cmd = PN_CMD_ARM;
  else if (word == "disarm")   cmd = PN_CMD_DISARM;
  else if (word == "snapshot") cmd = PN_CMD_SNAPSHOT;
  else if (word == "ping")     cmd = PN_CMD_PING;
  else if (word == "id" || word == "identify") cmd = PN_CMD_IDENTIFY;

  if (cmd == 0) {
    Serial.print(F("# unknown command: ")); Serial.println(word);
    Serial.println(F("# type 'help' for the list"));
    return;
  }

  PorpoiseNet.sendCommand(cmd, 0, arg, "");
  Serial.print(F("# sent '")); Serial.print(word);
  if (arg == PN_BROADCAST_ID) Serial.println(F("' to every node"));
  else { Serial.print(F("' to node ")); Serial.println(arg); }
}

// ============================== Setup =================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("# ====================================="));
  Serial.println(F("#  PorpoiseNet_Base  (base station)"));
  Serial.println(F("# ====================================="));

  if (!PorpoiseNet.begin(MY_NODE_ID, PN_ROLE_BASE, MY_CHANNEL, MY_NET_ID)) {
    Serial.println(F("# Radio failed to start - nothing will be received."));
    while (true) delay(1000);
  }
  PorpoiseNet.onMessage(onNetworkMessage);

  // The base is the node that confirms important messages. PorpoiseNet
  // switches this on for PN_ROLE_BASE already; it is spelled out here
  // because it is the single most important setting on this board.
  PorpoiseNet.setAckResponder(true);

  printHelp();
  Serial.println(F("# Waiting for the fleet..."));
}

// =============================== Loop =================================
void loop() {
  PorpoiseNet.loop();

  // ---- Anything typed in the serial monitor becomes a command ----
  static String typed;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (typed.length()) { handleTypedCommand(typed); typed = ""; }
    } else if (typed.length() < 40) {
      typed += c;
    }
  }

  if (millis() - lastRoster >= ROSTER_EVERY_MS) {
    lastRoster = millis();
    PorpoiseNet.printRoster();
    Serial.print(F("# session totals: finds ")); Serial.print(findCount);
    Serial.print(F("  alerts "));                Serial.println(alertCount);
  }
}
