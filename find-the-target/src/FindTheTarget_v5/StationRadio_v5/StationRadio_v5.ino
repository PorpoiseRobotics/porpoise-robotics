/* =====================================================================
 * StationRadio_v5  --  Find the Target v5, the board on the laptop
 * ---------------------------------------------------------------------
 * WHAT THIS BOARD DOES
 *   It is the laptop's radio. It sits on the laptop's USB port and:
 *     - hears every vehicle's sensor readings over PorpoiseNet and
 *       passes them to the dashboard,
 *     - carries chat lines and map marks between stations,
 *     - blinks your own vehicle's LEDs when a chat or mark arrives.
 *   The dashboard (Dashboard/start_dashboard.py) does all the drawing;
 *   this board just moves messages between the radio and the USB cable.
 *
 * ONE-TIME SETUP (each board, once)
 *   1. Upload this sketch. Every station uploads exactly the same file.
 *   2. Give it a station number: the dashboard asks for one the first
 *      time it sees this board, or type   station 3   in Serial Monitor.
 *      It is saved on the board, so you never do this again.
 *
 * BOARD / IDE SETTINGS
 *   Board:          "ESP32 Dev Module"
 *   Board package:  esp32 by Espressif Systems, 3.3.12
 *   Libraries:      none to install (PorpoiseNet.h is in this folder)
 *   Serial Monitor: 115200 baud. Close it before starting the dashboard:
 *                   only one program at a time can use the USB port.
 *
 * WHAT GOES OVER THE USB CABLE
 *   Board -> laptop: one JSON object per line, always with a "type":
 *     hello, telemetry, chat, mark, marklist, sync, sent, roster, saved,
 *     error. Lines starting with "# " are notes for people; the dashboard
 *     shows them in its log and otherwise ignores them.
 *   Laptop -> board: plain words, the same ones you can type yourself:
 *     chat <to> <ref> <text>       to = 0 for everyone, or a station 1-9
 *     mark add <id> <kind> <lat> <lon> <label>
 *     mark remove <id>
 *     marklist <id,id,id>          "these are all my marks"
 *     sync                         ask every station to resend its marks
 *     station <n>   info   roster   identify   help
 *
 * SAFETY: radio messages are not secret. Anyone nearby with an ESP32 can
 * read every chat. Use station numbers, never names.
 * ===================================================================== */

#include <Preferences.h>
#include "PorpoiseNet.h"        // the copy in THIS folder

#define FIRMWARE_VERSION "5.0"

// ====================== Network (same on EVERY board) ================
#define FTT_NET_ID   50     // must match VehicleSensors_v5
#define FTT_CHANNEL  1      // must match VehicleSensors_v5

#define MAX_STATIONS     9
#define ROSTER_EVERY_MS  5000
#define CMD_LINE_MAX         220    // longest command line we accept from the laptop

Preferences prefs;
uint8_t station = 0;
bool    meshUp  = false;

// ---------------------------------------------------------------------
// Remember which dashboard "ref" number belongs to which radio message,
// so when the network says "message #57 was confirmed" we can tell the
// dashboard "your chat 12 arrived".
// ---------------------------------------------------------------------
struct RefSlot { uint32_t seq; long ref; };
RefSlot refs[8] = {};
uint8_t refNext = 0;

void rememberRef(uint32_t seq, long ref) {
  refs[refNext] = { seq, ref };
  refNext = (refNext + 1) % 8;
}
long refFor(uint32_t seq) {
  for (auto &r : refs) if (r.seq == seq) return r.ref;
  return -1;
}

// ============================ JSON helpers ===========================
// Print text as a JSON string: quotes and backslashes get a backslash in
// front, and control characters are dropped. Letters with accents and
// emoji (UTF-8) pass through untouched.
void printJsonString(const char *s) {
  Serial.print('"');
  for (; *s; s++) {
    char c = *s;
    if (c == '"' || c == '\\') { Serial.print('\\'); Serial.print(c); }
    else if ((uint8_t)c < 0x20) { /* skip tabs, newlines and other control bytes */ }
    else Serial.print(c);
  }
  Serial.print('"');
}

void printFloat(const char *key, float v, int digits) {
  Serial.print(",\""); Serial.print(key); Serial.print("\":");
  if (isnan(v) || isinf(v)) Serial.print("null"); else Serial.print(v, digits);
}

void printHello() {
  Serial.print(F("{\"type\":\"hello\",\"board\":\"station-radio\",\"fw\":\""));
  Serial.print(F(FIRMWARE_VERSION));
  Serial.print(F("\",\"station\":")); Serial.print(station);
  Serial.print(F(",\"node\":"));      Serial.print(station);
  Serial.print(F(",\"net\":"));       Serial.print(FTT_NET_ID);
  Serial.print(F(",\"ch\":"));        Serial.print(FTT_CHANNEL);
  Serial.print(F(",\"mac\":\""));     Serial.print(meshUp ? PorpoiseNet.macStr() : "");
  Serial.println(F("\"}"));
}

void printError(const char *text) {
  Serial.print(F("{\"type\":\"error\",\"text\":"));
  printJsonString(text);
  Serial.println('}');
}

void printSent(long ref, const char *state) {
  Serial.print(F("{\"type\":\"sent\",\"ref\":")); Serial.print(ref);
  Serial.print(F(",\"state\":\"")); Serial.print(state);
  Serial.println(F("\"}"));
}

void printRosterJson() {
  Serial.print(F("{\"type\":\"roster\",\"nodes\":["));
  bool first = true;
  for (int i = 0; i < PN_MAX_PEERS; i++) {
    const PnPeer *p = PorpoiseNet.peer(i);
    if (!p) continue;
    if (!first) Serial.print(',');
    first = false;
    Serial.print(F("{\"id\":"));     Serial.print(p->id);
    Serial.print(F(",\"role\":\"")); Serial.print(pnRoleName(p->role));
    Serial.print(F("\",\"rssi\":")); if (p->macKnown) Serial.print(p->rssi); else Serial.print(F("null"));
    Serial.print(F(",\"age_s\":"));  Serial.print((millis() - p->lastHeardMs) / 1000);
    Serial.print('}');
  }
  Serial.println(F("]}"));
}

// ===================== Radio -> dashboard ============================
void tellOwnVehicle(uint8_t command) {
  PorpoiseNet.sendCommand(command, 0, 10 + station, "");
}

void onNetworkMessage(const PnMessage &msg, int8_t rssi) {
  const bool relayed = msg.flags & PN_FLAG_RELAYED;

  switch (msg.type) {
    case PN_TELEMETRY_FULL: {
      PnTelemetryFull t;
      if (!PorpoiseNet.asTelemetryFull(msg, t)) return;
      // A vehicle is node 10+S, so its station number is origin - 10.
      int st = (msg.origin > 10 && msg.origin <= 10 + MAX_STATIONS) ? msg.origin - 10 : 0;
      // Same key names as the v4 receiver, so the dashboard code carries over.
      Serial.print(F("{\"type\":\"telemetry\",\"station\":")); Serial.print(st);
      Serial.print(F(",\"node\":"));       Serial.print(msg.origin);
      Serial.print(F(",\"seq\":"));        Serial.print(msg.seq);
      Serial.print(F(",\"uptime_ms\":"));  Serial.print(msg.originMs);
      Serial.print(F(",\"sonar_ok\":"));   Serial.print((t.health & PN_TFULL_RANGE_OK) ? 1 : 0);
      Serial.print(F(",\"bmp_ok\":"));     Serial.print((t.health & PN_TFULL_BMP_OK) ? 1 : 0);
      Serial.print(F(",\"gps_fix\":"));    Serial.print((t.health & PN_TFULL_GPS_FIX) ? 1 : 0);
      Serial.print(F(",\"mag_ok\":"));     Serial.print((t.health & PN_TFULL_COMPASS_OK) ? 1 : 0);
      printFloat("distance_in",  (t.health & PN_TFULL_RANGE_OK) ? t.rangeInches : 0.0f, 1);
      printFloat("temp_f",       t.tempF, 1);
      printFloat("pressure_atm", t.pressureAtm, 4);
      printFloat("bmp_alt_ft",   t.bmpAltitudeFeet, 1);
      Serial.print(F(",\"sats\":"));       Serial.print(t.satellites);
      Serial.print(F(",\"lat\":"));        Serial.print(t.latitude, 7);
      Serial.print(F(",\"lon\":"));        Serial.print(t.longitude, 7);
      printFloat("speed_mph",    t.speedMph, 1);
      printFloat("gps_alt_ft",   t.gpsAltitudeFeet, 1);
      printFloat("course_deg",   t.courseDeg, 1);
      printFloat("heading_deg",  (t.health & PN_TFULL_COMPASS_OK) ? t.headingDeg : 0.0f, 1);
      Serial.print(F(",\"rssi\":"));       if (relayed) Serial.print(F("null")); else Serial.print(rssi);
      Serial.print(F(",\"relayed\":"));    Serial.print(relayed ? F("true") : F("false"));
      Serial.println('}');
      return;
    }

    case PN_CHAT: {
      PnChat c;
      if (!PorpoiseNet.asChat(msg, c)) return;
      Serial.print(F("{\"type\":\"chat\",\"id\":\""));
      Serial.print(msg.origin); Serial.print('-'); Serial.print(msg.seq);
      Serial.print(F("\",\"from\":")); Serial.print(msg.origin);
      Serial.print(F(",\"to\":"));     Serial.print(msg.dest);
      Serial.print(F(",\"text\":"));   printJsonString(c.text);
      Serial.println('}');
      if (msg.origin != station) tellOwnVehicle(PN_CMD_NOTIFY);
      return;
    }

    case PN_MARK: {
      PnMark m;
      if (!PorpoiseNet.asMark(msg, m)) return;
      Serial.print(F("{\"type\":\"mark\",\"from\":")); Serial.print(msg.origin);
      Serial.print(F(",\"action\":\""));
      Serial.print(m.action == PN_MARK_REMOVE ? F("remove") : F("add"));
      Serial.print(F("\",\"id\":"));   Serial.print(m.markId);
      Serial.print(F(",\"kind\":"));   Serial.print(m.kind);
      Serial.print(F(",\"lat\":"));    Serial.print(m.latitude, 7);
      Serial.print(F(",\"lon\":"));    Serial.print(m.longitude, 7);
      Serial.print(F(",\"label\":"));  printJsonString(m.label);
      Serial.println('}');
      return;
    }

    case PN_MARK_LIST: {
      PnMarkList l;
      if (!PorpoiseNet.asMarkList(msg, l)) return;
      Serial.print(F("{\"type\":\"marklist\",\"from\":")); Serial.print(msg.origin);
      Serial.print(F(",\"ids\":["));
      for (int i = 0; i < l.count; i++) { if (i) Serial.print(','); Serial.print(l.ids[i]); }
      Serial.println(F("]}"));
      return;
    }

    case PN_COMMAND: {
      PnCommand c;
      if (!PorpoiseNet.asCommand(msg, c)) return;
      if (c.command == PN_CMD_SYNC && msg.origin != station) {
        Serial.print(F("{\"type\":\"sync\",\"from\":")); Serial.print(msg.origin);
        Serial.println('}');
      }
      return;
    }
  }
}

void onSendResult(uint16_t dest, uint32_t seq, bool confirmed) {
  long ref = refFor(seq);
  if (ref >= 0) printSent(ref, confirmed ? "confirmed" : "failed");
}

// ===================== Dashboard -> radio ============================
// Split off the first word of `rest` into `word`; `rest` keeps the remainder.
String nextWord(String &rest) {
  rest.trim();
  int sp = rest.indexOf(' ');
  String w = (sp < 0) ? rest : rest.substring(0, sp);
  rest = (sp < 0) ? String("") : rest.substring(sp + 1);
  return w;
}

void printHelp() {
  Serial.println(F("# Commands (type one and press Enter):"));
  Serial.println(F("#   station 3                 set this radio's station number (1-9)"));
  Serial.println(F("#   chat 0 1 hello everyone   chat to everyone (the 1 is any number)"));
  Serial.println(F("#   chat 2 1 hi station two   chat to station 2 only"));
  Serial.println(F("#   mark add 1 2 32.7150 -117.1610 Big rock"));
  Serial.println(F("#                             kinds: 1 target 2 hazard 3 waypoint 4 meet"));
  Serial.println(F("#   mark remove 1             remove your mark number 1"));
  Serial.println(F("#   identify                  make your vehicle flash its color"));
  Serial.println(F("#   roster    info    sync"));
}

void handleCommand(String line) {
  line.trim();
  if (!line.length()) return;
  String rest = line;
  String cmd  = nextWord(rest);
  cmd.toLowerCase();

  if (cmd == "station") {
    long n = rest.toInt();
    if (n < 1 || n > MAX_STATIONS) { printError("station must be 1-9"); return; }
    prefs.putUChar("station", (uint8_t)n);
    Serial.print(F("# Saved: this is now the radio for station ")); Serial.println(n);
    Serial.println(F("{\"type\":\"saved\",\"what\":\"station\"}"));
    Serial.println(F("# Restarting..."));
    delay(300);
    ESP.restart();
  }
  if (cmd == "info") { printHello(); return; }
  if (cmd == "help") { printHelp();  return; }

  if (!meshUp) {
    printError("no station number yet: type station 1-9");
    return;
  }

  if (cmd == "chat") {
    long to  = nextWord(rest).toInt();
    long ref = nextWord(rest).toInt();
    if (to < 0 || to > MAX_STATIONS || to == station) { printError("chat: bad station"); return; }
    if (!rest.length()) { printError("chat: no text"); return; }
    // To one station: ask for confirmation. To everyone: no confirmation
    // (every station answering at once would jam the channel).
    bool needAck = (to != 0);
    bool ok = PorpoiseNet.sendChat(rest.c_str(), (uint16_t)to, needAck);
    if (!ok)          printSent(ref, "busy");
    else if (needAck) { rememberRef(PorpoiseNet.lastSentSeq(), ref); printSent(ref, "sending"); }
    else              printSent(ref, "sent");
    return;
  }

  if (cmd == "mark") {
    String action = nextWord(rest); action.toLowerCase();
    PnMark m = {};
    m.markId = (uint16_t)nextWord(rest).toInt();
    if (m.markId == 0) { printError("mark: id must be 1 or more"); return; }
    if (action == "add") {
      m.action    = PN_MARK_ADD;
      m.kind      = (uint8_t)nextWord(rest).toInt();
      m.latitude  = nextWord(rest).toDouble();
      m.longitude = nextWord(rest).toDouble();
      pnCopyStr(m.label, sizeof(m.label), rest.c_str());
    } else if (action == "remove") {
      m.action = PN_MARK_REMOVE;
    } else { printError("mark: use add or remove"); return; }
    PorpoiseNet.sendMark(m);
    return;
  }

  if (cmd == "marklist") {
    PnMarkList l = {};
    while (rest.length() && l.count < PN_MARK_LIST_MAX) {
      int comma = rest.indexOf(',');
      String one = comma < 0 ? rest : rest.substring(0, comma);
      rest = comma < 0 ? String("") : rest.substring(comma + 1);
      long id = one.toInt();
      if (id > 0) l.ids[l.count++] = (uint16_t)id;
    }
    PorpoiseNet.sendMarkList(l);
    return;
  }

  if (cmd == "sync")     { PorpoiseNet.sendCommand(PN_CMD_SYNC); return; }
  if (cmd == "identify") { PorpoiseNet.sendCommand(PN_CMD_IDENTIFY, 0, 10 + station, ""); return; }
  if (cmd == "roster")   { PorpoiseNet.printRoster(); printRosterJson(); return; }
  printHelp();
}

void readSerial() {
  static String line;
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\r') continue;
    if (ch == '\n') { handleCommand(line); line = ""; }
    else if (line.length() < CMD_LINE_MAX) line += ch;
  }
}

// ============================== Setup ================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("# ====================================="));
  Serial.println(F("#  StationRadio_v5  (Find the Target)"));
  Serial.println(F("# ====================================="));

  prefs.begin("ftt", false);
  station = prefs.getUChar("station", 0);
  if (station > MAX_STATIONS) station = 0;

  if (station == 0) {
    printHello();
    Serial.println(F("#"));
    Serial.println(F("# >>> This radio has no station number yet. <<<"));
    Serial.println(F("# >>> Type   station 3   (your number) and press Enter. <<<"));
    Serial.println(F("#"));
    return;
  }

  // The radio is a "base" on the network: it answers when a chat sent
  // to this station asks for confirmation.
  if (!PorpoiseNet.begin(station, PN_ROLE_BASE, FTT_CHANNEL, FTT_NET_ID)) {
    printError("radio failed to start - unplug and plug back in");
    return;
  }
  PorpoiseNet.onMessage(onNetworkMessage);
  PorpoiseNet.onSendResult(onSendResult);
  meshUp = true;
  printHello();
  // Just switched on: ask everyone for their marks so our map fills in.
  PorpoiseNet.sendCommand(PN_CMD_SYNC);
}

// =============================== Loop ================================
void loop() {
  readSerial();
  if (!meshUp) {
    // Repeat the hello now and then, so a dashboard started later still
    // learns that this board needs a station number.
    static unsigned long lastHello = 0;
    if (millis() - lastHello > 3000) { lastHello = millis(); printHello(); }
    return;
  }
  PorpoiseNet.loop();
  static unsigned long lastRoster = 0;
  if (millis() - lastRoster >= ROSTER_EVERY_MS) {
    lastRoster = millis();
    printRosterJson();
  }
}
