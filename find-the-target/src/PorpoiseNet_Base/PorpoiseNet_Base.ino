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
 *
 * ---------------------------------------------------------------------
 * *** THIS BOARD JOINS WIFI - AND THAT DECIDES THE WHOLE FLEET'S CHANNEL
 * ---------------------------------------------------------------------
 *   With USE_WIFI 1 this board connects to a router, so it can be reached
 *   from a browser and can push events to a server. That is useful, and
 *   it comes with one consequence that is easy to miss and impossible to
 *   debug by staring at the code:
 *
 *     An ESP32 has ONE radio and can be on ONE channel. When this board
 *     joins a router, the ROUTER decides its channel. Every other
 *     PorpoiseNet board is still sitting on whatever MY_CHANNEL says, and
 *     if those numbers differ the fleet and the base station are deaf to
 *     each other. Nothing prints an error. The roster is simply empty.
 *
 *   So this sketch connects to WiFi FIRST, then starts PorpoiseNet with
 *   PN_CHANNEL_FOLLOW_WIFI, reads back the channel the router actually
 *   handed out, and prints it in capital letters. Two ways to act on it:
 *
 *     BEST   Lock the router's 2.4 GHz channel to a fixed number (1, 6 or
 *            11 - not "Auto") and set MY_CHANNEL to that number on EVERY
 *            board. Then nothing moves and nothing surprises you.
 *
 *     OK     Read the channel this board prints at boot and set
 *            MY_CHANNEL to it on every other board. You must re-check
 *            after any router change, including ones nobody told you
 *            about, because "Auto" routers move channels on their own.
 *
 *   If WiFi fails to connect, this sketch falls back to MY_CHANNEL and
 *   carries on over USB, so a bad password does not take the game down.
 *
 * WHAT THE WIFI IS ACTUALLY FOR
 *   - http://<base-ip>/          a live status page: the roster, the last
 *                                find, the last alert. No laptop needed to
 *                                see whether the fleet is up.
 *   - http://<base-ip>/status.json   the same thing as JSON, for
 *                                base_station.py or anything else to poll.
 *   The USB serial output is UNCHANGED and still works exactly as before.
 *   WiFi is an addition, not a replacement.
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
#define MY_CHANNEL    1   // SAME on every board on this field.
                          // With USE_WIFI 1 this is only a FALLBACK for
                          // when WiFi does not connect - the router
                          // overrides it. Read the channel note above.
// ======================================================================

// Join a WiFi router? Set to 0 to go back to a USB-only base station.
#define USE_WIFI          1

#define ROSTER_EVERY_MS   10000

// Emit a JSON line for events (finds, alerts) as well as telemetry?
// Leave 0 unless base_station.py has been taught to tell them apart:
// today it treats every JSON line as a telemetry packet, so event lines
// would overwrite the vehicle's readings on the dashboard.
#define EMIT_EVENT_JSON   0

#if USE_WIFI
  #include <WiFi.h>
  #include <WebServer.h>
  // Real values go in secrets.h, which .gitignore blocks from ever being
  // committed. Copy secrets.example.h to secrets.h and fill it in.
  // THIS REPOSITORY IS PUBLIC: a password typed straight into this file
  // would be readable by everyone, forever, even after it is deleted.
  #include "secrets.h"
  WebServer webServer(80);
  bool wifiUp = false;
#endif

unsigned long lastRoster = 0;
uint32_t      findCount  = 0;
uint32_t      alertCount = 0;

// Last event seen, kept so the web page has something to show. Deliberately
// small and deliberately overwritten - this is a status light, not a log.
uint16_t      lastFindNode  = 0;
double        lastFindLat   = 0.0, lastFindLon = 0.0;
unsigned long lastFindMs    = 0;
uint16_t      lastAlertNode = 0, lastAlertZone = 0;
unsigned long lastAlertMs   = 0;

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
      lastFindNode = msg.origin;
      lastFindLat  = r.latitude;
      lastFindLon  = r.longitude;
      lastFindMs   = millis();
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
      lastAlertNode = msg.origin;
      lastAlertZone = a.zone;
      lastAlertMs   = millis();
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

// ============================== web pages =============================
#if USE_WIFI

// Build the roster as a JSON array. Shared by the page and the endpoint so
// there is only one place where the shape of this data is decided.
String rosterJson() {
  String out = "[";
  bool first = true;
  for (int i = 0; i < PN_MAX_PEERS; i++) {
    const PnPeer *p = PorpoiseNet.peer(i);
    if (!p) continue;
    if (!first) out += ",";
    first = false;
    out += "{\"id\":";        out += p->id;
    out += ",\"role\":\"";   out += pnRoleName(p->role);
    out += "\",\"rssi\":";   out += p->rssi;
    out += ",\"heard_s\":";  out += (millis() - p->lastHeardMs) / 1000;
    out += ",\"received\":"; out += p->received;
    out += ",\"lost\":";     out += p->missed;
    out += "}";
  }
  out += "]";
  return out;
}

void handleStatusJson() {
  String j = "{\"base\":";        j += MY_NODE_ID;
  j += ",\"net_id\":";            j += MY_NET_ID;
  j += ",\"channel\":";           j += PorpoiseNet.channel();
  j += ",\"uptime_s\":";          j += millis() / 1000;
  j += ",\"finds\":";             j += findCount;
  j += ",\"alerts\":";            j += alertCount;
  j += ",\"last_find_node\":";    j += lastFindNode;
  j += ",\"last_find_lat\":";     j += String(lastFindLat, 6);
  j += ",\"last_find_lon\":";     j += String(lastFindLon, 6);
  j += ",\"last_alert_node\":";   j += lastAlertNode;
  j += ",\"last_alert_zone\":";   j += lastAlertZone;
  j += ",\"nodes\":";             j += rosterJson();
  j += "}";
  webServer.send(200, "application/json", j);
}

// A deliberately plain page. It refreshes itself every 2 seconds rather
// than using JavaScript, because the point is to glance at it from a phone
// on the field and see whether the fleet is up.
void handleRoot() {
  String h = "<!doctype html><html><head><meta charset='utf-8'>";
  h += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  h += "<meta http-equiv='refresh' content='2'><title>PorpoiseNet base</title>";
  h += "<style>body{font-family:system-ui,sans-serif;background:#252525;";
  h += "color:#eee;margin:0;padding:16px}h1{font-size:18px;margin:0 0 4px}";
  h += "table{border-collapse:collapse;width:100%;max-width:640px;margin-top:12px}";
  h += "th,td{text-align:left;padding:6px 10px;border-bottom:1px solid #444;";
  h += "font-variant-numeric:tabular-nums}th{color:#9ab;font-weight:600}";
  h += ".sub{color:#999;font-size:13px}.big{font-size:15px;margin-top:14px}";
  h += ".stale{color:#e88}</style></head><body>";

  h += "<h1>PorpoiseNet base station</h1><div class='sub'>net ";
  h += MY_NET_ID; h += " &middot; channel "; h += PorpoiseNet.channel();
  h += " &middot; up "; h += millis() / 1000; h += "s</div>";

  h += "<div class='big'>finds <b>"; h += findCount;
  h += "</b> &nbsp; alerts <b>";     h += alertCount; h += "</b></div>";

  if (lastFindNode) {
    h += "<div class='sub'>last find: vehicle "; h += lastFindNode;
    h += ", "; h += (millis() - lastFindMs) / 1000; h += "s ago</div>";
  }
  if (lastAlertNode) {
    h += "<div class='sub'>last alert: node "; h += lastAlertNode;
    h += ", zone "; h += lastAlertZone;
    h += ", "; h += (millis() - lastAlertMs) / 1000; h += "s ago</div>";
  }

  h += "<table><tr><th>node</th><th>role</th><th>signal</th><th>heard</th>";
  h += "<th>received</th><th>lost</th></tr>";
  int shown = 0;
  for (int i = 0; i < PN_MAX_PEERS; i++) {
    const PnPeer *p = PorpoiseNet.peer(i);
    if (!p) continue;
    shown++;
    unsigned long age = (millis() - p->lastHeardMs) / 1000;
    h += "<tr><td>";  h += p->id;
    h += "</td><td>"; h += pnRoleName(p->role);
    h += "</td><td>"; h += p->rssi; h += " dBm";
    h += "</td><td";  if (age > 10) h += " class='stale'";
    h += ">";         h += age; h += "s";
    h += "</td><td>"; h += p->received;
    h += "</td><td>"; h += p->missed;
    h += "</td></tr>";
  }
  if (!shown) {
    h += "<tr><td colspan='6' class='stale'>no nodes heard - check that ";
    h += "every board uses channel "; h += PorpoiseNet.channel();
    h += " and net id "; h += MY_NET_ID; h += "</td></tr>";
  }
  h += "</table></body></html>";
  webServer.send(200, "text/html", h);
}

// Connect to the router, then report what channel we ended up on. Returns
// the channel argument to hand to PorpoiseNet.begin().
uint8_t connectWifi() {
  Serial.print(F("# joining WiFi "));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(400);
    Serial.print(F("."));
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    // Not fatal. The game runs fine on USB alone, and a base station that
    // dies because of a typo'd password would be a poor trade.
    Serial.println(F("# WiFi did NOT connect. Check the SSID and password in"));
    Serial.println(F("# secrets.h, and that the network is 2.4 GHz - an ESP32"));
    Serial.println(F("# cannot see 5 GHz networks at all."));
    Serial.print  (F("# Falling back to channel ")); Serial.print(MY_CHANNEL);
    Serial.println(F("; serial output is unaffected."));
    WiFi.disconnect();
    wifiUp = false;
    return MY_CHANNEL;
  }

  wifiUp = true;
  Serial.print(F("# WiFi connected, status page at http://"));
  Serial.println(WiFi.localIP());
  return PN_CHANNEL_FOLLOW_WIFI;   // adopt whatever channel the router gave us
}

// Say the channel out loud, and say it again if it disagrees with what the
// rest of the fleet has been told. This is the one mismatch that produces
// no error anywhere else in the system.
void announceChannel() {
  uint8_t ch = PorpoiseNet.channel();
  Serial.println(F("#"));
  Serial.print  (F("# >>> THE FLEET MUST USE CHANNEL ")); Serial.print(ch);
  Serial.println(F(" <<<"));
  if (ch != MY_CHANNEL) {
    Serial.println(F("#"));
    Serial.println(F("# ***********************************************"));
    Serial.print  (F("# * The router put this board on channel ")); Serial.println(ch);
    Serial.print  (F("# * but MY_CHANNEL on the other boards says "));
    Serial.println(MY_CHANNEL);
    Serial.println(F("# * They CANNOT hear each other. The roster will"));
    Serial.println(F("# * stay empty and nothing will report an error."));
    Serial.println(F("# *"));
    Serial.println(F("# * Fix it either way:"));
    Serial.print  (F("# *   set MY_CHANNEL to ")); Serial.print(ch);
    Serial.println(F(" on every other board, or"));
    Serial.print  (F("# *   lock the router to channel ")); Serial.print(MY_CHANNEL);
    Serial.println(F(" (not 'Auto')"));
    Serial.println(F("# ***********************************************"));
  }
  Serial.println(F("#"));
}
#endif  // USE_WIFI

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
  Serial.println(F("#   wifi         show the IP address and radio channel"));
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
  else if (word == "wifi") {
#if USE_WIFI
    if (wifiUp) {
      Serial.print(F("# WiFi connected, status page at http://"));
      Serial.println(WiFi.localIP());
    } else {
      Serial.println(F("# WiFi is not connected; running on USB only."));
    }
    announceChannel();
#else
    Serial.print(F("# built with USE_WIFI 0 - USB only, channel "));
    Serial.println(PorpoiseNet.channel());
#endif
    return;
  }

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

  // ORDER MATTERS. Join the router FIRST, then start PorpoiseNet on
  // whatever channel the router put us on. Starting the radio first and
  // connecting afterwards moves this board's channel out from under
  // ESP-NOW, and the mesh goes quiet with no error printed anywhere.
  uint8_t channel = MY_CHANNEL;
#if USE_WIFI
  channel = connectWifi();
#endif

  if (!PorpoiseNet.begin(MY_NODE_ID, PN_ROLE_BASE, channel, MY_NET_ID)) {
    Serial.println(F("# Radio failed to start - nothing will be received."));
    while (true) delay(1000);
  }
  PorpoiseNet.onMessage(onNetworkMessage);

#if USE_WIFI
  announceChannel();
  if (wifiUp) {
    webServer.on("/", handleRoot);
    webServer.on("/status.json", handleStatusJson);
    webServer.begin();
    Serial.println(F("# web status page running on port 80"));
  }
#endif

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

#if USE_WIFI
  // Cheap when nobody is connected, and it must run often: a browser that
  // waits too long for a reply gives up and shows an error page.
  if (wifiUp) webServer.handleClient();
#endif

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
