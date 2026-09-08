/* =====================================================================
 * PorpoiseNet.h  --  one shared two-way ESP-NOW network for every
 *                    Porpoise Robotics project
 * ---------------------------------------------------------------------
 * WHAT THIS FILE IS
 *   A single header file that turns any ESP32 into a node on a small
 *   two-way radio network. Every board runs the SAME networking code and
 *   differs only in its NODE ID and its ROLE. That is the whole idea:
 *
 *     - Find the Target (game)  : rovers + a base station. A rover that
 *                                 spots the target tells EVERY other
 *                                 rover, and the base can talk back.
 *     - Homeland Security Camera: motion sensors + a camera + a base.
 *                                 A sensor that trips tells the camera,
 *                                 the camera photographs it and tells
 *                                 everyone what it saved.
 *
 *   Same header, same packets, same channel rules. A rover and a camera
 *   are the same kind of node as far as the radio is concerned.
 *
 * WHY ESP-NOW AND NOT WIFI
 *   ESP-NOW is a peer-to-peer mode built into the ESP32 radio. There is
 *   no router, no password, no IP address, and no "join the network"
 *   step. Two boards with power can talk. That matters here because:
 *     - a school field has no WiFi coverage,
 *     - it works the instant both boards boot (~ms, not ~10s),
 *     - an ESP32 costs a few dollars, so a whole fleet is affordable.
 *
 * WHAT IT COSTS YOU
 *   - A packet is at most 250 bytes. No video, no audio, no file
 *     transfer. Ever. (See "the camera bridge" note below.)
 *   - Nothing is secret. Anyone nearby with an ESP32 can hear every
 *     message. Read the SECURITY section before the camera project.
 *   - Range outdoors, line of sight, is roughly 100-200 m at the power
 *     settings used here. Buildings and bodies cut that hard.
 *
 * ---------------------------------------------------------------------
 * HOW TO ACTUALLY CONNECT THE NETWORK  (read this part twice)
 * ---------------------------------------------------------------------
 * STEP 1 - GIVE EVERY BOARD A DIFFERENT NODE ID
 *   At the top of each sketch there is a line like
 *       #define MY_NODE_ID  11
 *   Node IDs are just numbers you choose. They must be UNIQUE inside one
 *   network. Two boards with the same ID will confuse the roster and the
 *   duplicate filter. The convention used across this repo:
 *       1        base station
 *       10-19    ground vehicles / rovers
 *       20-29    cameras
 *       30-39    fixed sensors (motion, door, etc.)
 *   ID 0 is reserved: it means "everybody" (broadcast).
 *
 * STEP 2 - PUT EVERY BOARD ON THE SAME NETWORK ID
 *       #define MY_NET_ID  7
 *   This is NOT security. It is a label carried in every packet so that
 *   two teams working in the same gym do not receive each other's
 *   messages. Different net ID = ignored. Pick one number per field, per
 *   team, per class period. Same on every board or nothing works.
 *
 * STEP 3 - PUT EVERY BOARD ON THE SAME WIFI CHANNEL
 *       #define MY_CHANNEL  1
 *   This is the #1 reason an ESP-NOW network "does not work" while every
 *   board looks fine. Radios on different channels are deaf to each
 *   other, and NOTHING prints an error - you just get silence.
 *   Channel 1, 6 or 11 are the sane choices (they do not overlap).
 *   *** If any node also joins a WiFi router, read STEP 6. ***
 *
 * STEP 4 - THERE IS NO STEP 4. TURN THEM ON.
 *   You do NOT have to collect MAC addresses and paste them into each
 *   other's sketches. PorpoiseNet talks to the broadcast address
 *   FF:FF:FF:FF:FF:FF, which every ESP-NOW radio listens to, and it
 *   LEARNS each node's MAC the first time it hears from it. Adding a
 *   fourth rover mid-game needs no edits to the other three.
 *   (The older ESPNow_Sender_v3 sketch in this repo does the paste-the-
 *   MAC dance. That is why it only ever talks to one board.)
 *
 * STEP 5 - CHECK THE ROSTER, NOT THE VIBES
 *   Every node prints a roster every few seconds:
 *       ROSTER  id=1 base  rssi=-42dBm  2s ago
 *               id=12 vehicle rssi=-71dBm  1s ago
 *   If a board is not in the roster, it is not on the network. Work the
 *   list: powered? same MY_NET_ID? same MY_CHANNEL? in range?
 *   RSSI is signal strength in dBm, always negative, closer to 0 is
 *   better. -30 is next to you, -70 is fine, -85 is about to fail.
 *
 * STEP 6 - THE CHANNEL TRAP, WHEN A NODE ALSO USES REAL WIFI
 *   An ESP32 has ONE radio. It cannot be on channel 1 for ESP-NOW and
 *   channel 6 for a router at the same time. The moment a board joins a
 *   router, the ROUTER decides its channel, and its ESP-NOW link to
 *   everyone else silently dies.
 *   This matters for the camera node, which wants both.
 *   You have three ways out, in order of preference:
 *
 *     (a) LOCK THE ROUTER TO ONE CHANNEL. In the router's admin page set
 *         the 2.4 GHz channel to a fixed 1 (not "Auto"), then set
 *         MY_CHANNEL 1 on every board. Simple, stable, recommended.
 *
 *     (b) LET THE CAMERA ANNOUNCE THE CHANNEL. Pass PN_CHANNEL_FOLLOW_WIFI
 *         on the board that joins the router. PorpoiseNet reads whatever
 *         channel the router handed it and prints, in capital letters,
 *         the number to put in MY_CHANNEL on every other board. Works,
 *         but you must re-check after any router change.
 *
 *     (c) DO NOT PUT WIFI AND ESP-NOW ON THE SAME BOARD. Use two cheap
 *         boards: one ESP-NOW node, one WiFi uploader, joined by two
 *         wires (serial). Costs $5 and removes the whole problem class.
 *
 *   Also: a board joined to a router will power-save its radio and drop
 *   ESP-NOW packets. PorpoiseNet calls WiFi.setSleep(false) for you, and
 *   your sketch must not turn it back on.
 *
 * STEP 7 - RANGE, IF THE FIELD IS BIGGER THAN THE RADIO
 *   Two options, and they stack:
 *     - RELAY: any node can repeat a message it hears (TTL, see below),
 *       so a rover in the middle extends the reach of one at the far
 *       end. On by default, costs nothing but airtime.
 *     - LONG RANGE MODE: call PorpoiseNet.enableLongRange() on EVERY
 *       node. Espressif's LR mode trades data rate for roughly 2-4x the
 *       distance. Two rules: every node must enable it (LR and normal
 *       radios cannot hear each other), and an LR node CANNOT join a
 *       WiFi router. So: fine for rovers, never on the camera bridge.
 *
 * ---------------------------------------------------------------------
 * HOW A MESSAGE TRAVELS  (what the code below is actually doing)
 * ---------------------------------------------------------------------
 *   1. A node calls e.g. reportTarget(). PorpoiseNet fills in a
 *      PnMessage: who sent it (origin), a counter (seq), what kind it is
 *      (type), how many hops it may still take (ttl), and the payload.
 *   2. It goes out to FF:FF:FF:FF:FF:FF - every node in range hears it.
 *   3. Each receiver checks magic + protocol + net ID and throws away
 *      anything that is not ours. Then it checks whether it has already
 *      seen (origin, seq). Radios repeat; this is what stops the same
 *      find being announced four times.
 *   4. If the message was addressed to everyone, or to this node, the
 *      sketch's onMessage() handler runs.
 *   5. If ttl is still above 1, the node decrements it and rebroadcasts,
 *      after a short random delay so two relays do not talk over each
 *      other. That is the whole mesh: no routing table, no addresses,
 *      just "repeat it once, count the hops".
 *   6. If the sender asked for an acknowledgement, the node whose job it
 *      is to answer (normally the base station) sends a PN_ACK back. The
 *      sender retries until it gets one or gives up, and tells you which.
 *
 * ---------------------------------------------------------------------
 * SECURITY - READ BEFORE THE CAMERA PROJECT
 * ---------------------------------------------------------------------
 *   ESP-NOW frames here are UNENCRYPTED. Assume anyone within a few
 *   hundred metres with a $5 board can read every alert and can forge
 *   one. MY_NET_ID keeps two classes from crossing wires; it stops
 *   nobody who is trying.
 *
 *   What follows from that, for a system that will run at a school:
 *     - Never put anything in a payload you would not shout across the
 *       quad: no names, no room numbers, no door codes.
 *     - Never let an ESP-NOW message alone unlock, disarm, or disable
 *       anything physical. Treat every packet as a hint from a stranger.
 *     - Camera images NEVER travel over ESP-NOW - the protocol cannot
 *       carry them and should not. ESP-NOW carries the one-line event
 *       ("zone 3 tripped"); the picture goes to an SD card or over real
 *       WiFi to a server you control.
 *     - The privacy questions in homeland-security-camera/README.md
 *       (who owns the footage, how long it is kept, what the school has
 *       approved in writing) are not answered by any of this code and
 *       must be settled before anything is mounted on a wall.
 *   ESP-NOW does support real encryption, but only for one-to-one peers,
 *   not for the broadcast this design is built on. Adding it would mean
 *   giving up the "turn it on and it works" property in STEP 4. That
 *   trade is deliberate and is the right one for a teaching network -
 *   and the wrong one for anything that actually needs to be secure.
 *
 * ---------------------------------------------------------------------
 * INSTALLING THIS FILE
 * ---------------------------------------------------------------------
 *   Either way works; pick one and tell the whole team which.
 *   (a) As a library: copy the folder shared/PorpoiseNet into
 *       Documents/Arduino/libraries/  then restart the Arduino IDE.
 *       #include <PorpoiseNet.h>
 *   (b) As a plain file: copy PorpoiseNet.h into the sketch folder,
 *       next to the .ino.   #include "PorpoiseNet.h"
 *   Header-only on purpose: there is no .cpp to forget to copy.
 *
 * COMPATIBILITY
 *   Arduino-ESP32 core 2.x and 3.x. Espressif changed the ESP-NOW
 *   callback signatures twice; the shims below handle all three shapes,
 *   which is why the same file compiles on a laptop that has not been
 *   updated since 2023 and one installed last week.
 *
 * NOTE ON HEADER-ONLY: the state below is file-scope `static`, which is
 * correct for an Arduino sketch (one compilation unit). If you ever add
 * your own .cpp file to a sketch, include this header in ONE of them
 * only, or each file gets its own private, silent copy of the network.
 * ===================================================================== */

#ifndef PORPOISENET_H
#define PORPOISENET_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "esp_mac.h"

// ============================================================= shims ==
// Espressif changed these callback signatures between core versions.
// Older cores do not even define ESP_ARDUINO_VERSION, so assume 2.0.0.
#ifndef ESP_ARDUINO_VERSION_VAL
  #define ESP_ARDUINO_VERSION_VAL(major, minor, patch) \
          ((major) * 10000 + (minor) * 100 + (patch))
#endif
#ifndef ESP_ARDUINO_VERSION
  #define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(2, 0, 0)
#endif

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  // core 3.x: receive callback carries an info struct (MAC + RSSI)
  #define PN_RECV_ARGS  const esp_now_recv_info_t *pnInfo, \
                        const uint8_t *pnData, int pnLen
  #define PN_RECV_SRC   ((pnInfo) ? pnInfo->src_addr : nullptr)
  #define PN_RECV_RSSI  (((pnInfo) && (pnInfo)->rx_ctrl) \
                          ? (int8_t)(pnInfo)->rx_ctrl->rssi : (int8_t)0)
#else
  // core 2.x: just the sender's MAC, and no RSSI at all
  #define PN_RECV_ARGS  const uint8_t *pnSrc, const uint8_t *pnData, int pnLen
  #define PN_RECV_SRC   (pnSrc)
  #define PN_RECV_RSSI  ((int8_t)0)
#endif

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 1, 0)
  // core 3.1+: first argument became a transmit-info struct. We only ever
  // use the status, so the pointer is deliberately ignored.
  #define PN_SEND_ARGS  const wifi_tx_info_t *pnTx, esp_now_send_status_t pnStatus
#else
  #define PN_SEND_ARGS  const uint8_t *pnDst, esp_now_send_status_t pnStatus
#endif

// ========================================================== protocol ==
#define PN_MAGIC             0xB7    // "is this even one of ours?" byte
#define PN_PROTOCOL_VERSION  1       // bump if the header layout changes
#define PN_MAX_PAYLOAD       200     // 250-byte ESP-NOW limit minus header
#define PN_BROADCAST_ID      0       // dest = 0 means "everyone"
#define PN_DEFAULT_TTL       2       // hops a message may take (1 = no relay)

// ---- Message types -------------------------------------------------
// Add new ones at the END and never renumber: an old board on a shelf
// will still be speaking the old numbers next season.
#define PN_HELLO      1   // "I exist" heartbeat, builds the roster
#define PN_TELEMETRY  2   // routine sensor readings
#define PN_TARGET     3   // THE GAME EVENT: a vehicle found the target
#define PN_ALERT      4   // THE SECURITY EVENT: a sensor tripped
#define PN_COMMAND    5   // base -> nodes: start, stop, arm, recall...
#define PN_ACK        6   // "I received your message", sent automatically
#define PN_EVIDENCE   7   // camera -> everyone: "I photographed that"
#define PN_TEXT       8   // free-form text, for debugging and messing about

// ---- Roles ---------------------------------------------------------
#define PN_ROLE_UNKNOWN  0
#define PN_ROLE_BASE     1   // the board on the laptop's USB port
#define PN_ROLE_VEHICLE  2   // a rover
#define PN_ROLE_CAMERA   3   // a camera node
#define PN_ROLE_SENSOR   4   // a fixed motion / door / vibration sensor

// ---- Header flags --------------------------------------------------
#define PN_FLAG_NEEDS_ACK  0x01  // sender wants confirmation
#define PN_FLAG_RELAYED    0x02  // this copy came via a repeater, not direct

// ---- Commands (payload of PN_COMMAND) ------------------------------
#define PN_CMD_PING        1   // "answer me", the network's ping
#define PN_CMD_START       2   // game: begin the hunt
#define PN_CMD_STOP        3   // game: everybody stop
#define PN_CMD_RECALL      4   // game: come home
#define PN_CMD_RESET       5   // game: clear the found-target state
#define PN_CMD_ARM         6   // security: sensors start reporting
#define PN_CMD_DISARM      7   // security: sensors stay quiet
#define PN_CMD_SNAPSHOT    8   // security: camera, take a picture now
#define PN_CMD_IDENTIFY    9   // flash your LEDs so I can see which board you are

// ---- Sensor kinds (payload of PN_ALERT) ----------------------------
#define PN_SENSOR_MOTION     1
#define PN_SENSOR_DOOR       2
#define PN_SENSOR_VIBRATION  3
#define PN_SENSOR_RANGE      4
#define PN_SENSOR_MANUAL     5   // a human pressed the button

// ---- Channel selection ---------------------------------------------
// Pass this instead of a channel number on a board that also joins a
// WiFi router; see STEP 6 above.
#define PN_CHANNEL_FOLLOW_WIFI  0

// ---- Tuning --------------------------------------------------------
#define PN_MAX_PEERS       12   // roster size (ESP-NOW itself allows 20)
#define PN_SEEN_HISTORY    24   // duplicate-filter memory, in messages
#define PN_RX_QUEUE        8    // messages buffered between radio and loop()
#define PN_ACK_RETRIES     4    // resends before giving up on an ack
#define PN_ACK_TIMEOUT_MS  400  // wait before resending
#define PN_HELLO_EVERY_MS  3000 // heartbeat interval
#define PN_PEER_STALE_MS   10000// no word for this long = "lost"

// ================================================== message layout ====
// __attribute__((packed)) means "no invisible padding" - every board
// must agree byte for byte, so we forbid the compiler from being clever.
typedef struct __attribute__((packed)) {
  uint8_t  magic;       // PN_MAGIC, cheap "not for us" filter
  uint8_t  protocol;    // PN_PROTOCOL_VERSION
  uint8_t  netId;       // MY_NET_ID: which field / team / class period
  uint8_t  type;        // PN_HELLO, PN_TARGET, ...
  uint8_t  role;        // sender's role, so the roster can label it
  uint8_t  flags;       // PN_FLAG_*
  uint8_t  ttl;         // hops remaining; a relay decrements it
  uint8_t  payloadLen;  // bytes used in payload[]
  uint16_t origin;      // node that CREATED this (relays never change it)
  uint16_t dest;        // 0 = everyone, else a node id
  uint32_t seq;         // per-origin counter. (origin,seq) = unique id
  uint32_t originMs;    // millis() on the origin - lets you see age
  uint8_t  payload[PN_MAX_PAYLOAD];
} PnMessage;

#define PN_HEADER_BYTES  (sizeof(PnMessage) - PN_MAX_PAYLOAD)

// The radio's hard limit. If someone enlarges PN_MAX_PAYLOAD or adds a
// header field past this point, the compiler stops them here rather than
// letting every send() fail silently at 2am the night before a demo.
static_assert(sizeof(PnMessage) <= 250,
              "PnMessage is bigger than an ESP-NOW frame can carry (250 bytes)");

// ------------------------- payload structures -----------------------
// One struct per message type. Same packed rule applies.

// PN_TARGET - the Find the Target game event.
typedef struct __attribute__((packed)) {
  uint8_t  contactType;    // free for the team to define: 1=cone, 2=flag...
  uint8_t  confidence;     // 0-100, how sure the rover is
  float    rangeInches;    // how far the rover was from it
  float    fieldX_ft;      // position on the field map, feet from the
  float    fieldY_ft;      //   origin corner (same axes as the dashboard)
  double   latitude;       // GPS, 0 if no fix
  double   longitude;
  float    headingDeg;     // which way the rover was pointing
  char     note[24];       // short human label, always NUL-terminated
} PnTargetReport;

// PN_ALERT - the security event.
typedef struct __attribute__((packed)) {
  uint8_t  sensor;         // PN_SENSOR_*
  uint8_t  severity;       // 1 = note, 2 = attention, 3 = urgent
  uint16_t zone;           // a NUMBER, never a room name (see SECURITY)
  uint32_t count;          // how many times this sensor has tripped
  float    value;          // sensor-specific: inches, seconds, whatever
  char     note[24];
} PnAlert;

// PN_COMMAND - base station to everyone or to one node.
typedef struct __attribute__((packed)) {
  uint8_t  command;        // PN_CMD_*
  uint8_t  arg;            // meaning depends on the command
  char     note[24];
} PnCommand;

// PN_EVIDENCE - camera reporting what it did about an event.
typedef struct __attribute__((packed)) {
  uint16_t aboutOrigin;    // whose event this was
  uint32_t aboutSeq;       //   (origin,seq) of the message that triggered it
  uint8_t  result;         // 0 failed, 1 saved to SD, 2 uploaded, 3 both
  uint32_t bytes;          // size of the image
  char     name[28];       // file name only, never a full path
} PnEvidence;

// PN_ACK - generated for you; you rarely touch this.
typedef struct __attribute__((packed)) {
  uint16_t ackOrigin;
  uint32_t ackSeq;
} PnAck;

// PN_TELEMETRY - routine readings. Trimmed to what both projects use.
typedef struct __attribute__((packed)) {
  uint8_t  health;         // bit0 range ok, bit1 gps fix, bit2 battery ok
  uint8_t  battPercent;    // 0-100, 255 = not measured
  float    rangeInches;
  float    tempF;
  float    fieldX_ft;
  float    fieldY_ft;
  double   latitude;
  double   longitude;
  float    speedMph;
  float    headingDeg;
  uint32_t satellites;
} PnTelemetry;

// Every payload has to fit in the space a message leaves for it.
static_assert(sizeof(PnTargetReport) <= PN_MAX_PAYLOAD, "PnTargetReport too big");
static_assert(sizeof(PnAlert)        <= PN_MAX_PAYLOAD, "PnAlert too big");
static_assert(sizeof(PnCommand)      <= PN_MAX_PAYLOAD, "PnCommand too big");
static_assert(sizeof(PnEvidence)     <= PN_MAX_PAYLOAD, "PnEvidence too big");
static_assert(sizeof(PnAck)          <= PN_MAX_PAYLOAD, "PnAck too big");
static_assert(sizeof(PnTelemetry)    <= PN_MAX_PAYLOAD, "PnTelemetry too big");

#define PN_HEALTH_RANGE_OK  0x01
#define PN_HEALTH_GPS_FIX   0x02
#define PN_HEALTH_BATT_OK   0x04

// --------------------------- roster entry ---------------------------
typedef struct {
  uint16_t      id;
  uint8_t       role;
  uint8_t       mac[6];
  int8_t        rssi;         // signal strength of the last DIRECT packet
  bool          macKnown;     // false until we hear from it without a relay
  unsigned long lastHeardMs;
  uint32_t      received;     // messages counted from this node
  uint32_t      missed;       // sequence gaps: messages that never arrived
  uint32_t      lastSeq;
} PnPeer;

// ====================================================== small helpers ==
inline const char *pnRoleName(uint8_t role) {
  switch (role) {
    case PN_ROLE_BASE:    return "base";
    case PN_ROLE_VEHICLE: return "vehicle";
    case PN_ROLE_CAMERA:  return "camera";
    case PN_ROLE_SENSOR:  return "sensor";
    default:              return "unknown";
  }
}

inline const char *pnTypeName(uint8_t type) {
  switch (type) {
    case PN_HELLO:     return "HELLO";
    case PN_TELEMETRY: return "TELEMETRY";
    case PN_TARGET:    return "TARGET";
    case PN_ALERT:     return "ALERT";
    case PN_COMMAND:   return "COMMAND";
    case PN_ACK:       return "ACK";
    case PN_EVIDENCE:  return "EVIDENCE";
    case PN_TEXT:      return "TEXT";
    default:           return "?";
  }
}

// Write a MAC into a caller-supplied 18-byte buffer. Returns the buffer
// so it can be used straight inside a Serial.print().
inline const char *pnMacToStr(const uint8_t *mac, char *out18) {
  if (!mac) { strcpy(out18, "??:??:??:??:??:??"); return out18; }
  sprintf(out18, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return out18;
}

// Copy a C string into a fixed char[] field, always leaving it
// NUL-terminated. Payload strings cross between boards, so a missing
// terminator would print garbage on the other side.
inline void pnCopyStr(char *dst, size_t dstSize, const char *src) {
  if (dstSize == 0) return;
  if (!src) { dst[0] = '\0'; return; }
  strncpy(dst, src, dstSize - 1);
  dst[dstSize - 1] = '\0';
}

// ========================================================== the class ==
class PorpoiseNetClass {
 public:
  // Your sketch's handler: called from loop(), once per NEW message that
  // is addressed to this node (or to everyone). Duplicates and relayed
  // copies of things you already saw never reach it.
  typedef void (*MessageHandler)(const PnMessage &msg, int8_t rssi);

  // ------------------------------------------------------------------
  // begin() - call once in setup(), AFTER Serial.begin().
  //   nodeId  : this board's unique number (see STEP 1)
  //   role    : PN_ROLE_VEHICLE / _BASE / _CAMERA / _SENSOR
  //   channel : 1-13, all boards the same (STEP 3), or
  //             PN_CHANNEL_FOLLOW_WIFI on a board already joined to a
  //             router (STEP 6b) - call WiFi.begin() and wait for the
  //             connection BEFORE calling this.
  //   netId   : which network this board belongs to (STEP 2)
  // Returns false if the radio refused to start, in which case nothing
  // will ever be sent or received and the sketch should say so loudly.
  // ------------------------------------------------------------------
  bool begin(uint16_t nodeId, uint8_t role, uint8_t channel, uint8_t netId);

  // Call every time round loop(), as often as possible. This is where
  // received messages are dispatched, relays go out, acknowledgements
  // are chased and heartbeats are sent. Nothing happens without it.
  void loop();

  void onMessage(MessageHandler handler) { _handler = handler; }

  // ---------------------------- sending -----------------------------
  // The general one. dest = PN_BROADCAST_ID reaches every node.
  // needAck asks for confirmation and retries until it arrives; use it
  // for events that matter (a find, an intrusion), not for telemetry.
  bool send(uint8_t type, const void *payload, uint8_t payloadLen,
            uint16_t dest = PN_BROADCAST_ID, bool needAck = false);

  // Convenience wrappers - what the sketches actually call.
  bool reportTarget(const PnTargetReport &report, bool needAck = true) {
    return send(PN_TARGET, &report, sizeof(report), PN_BROADCAST_ID, needAck);
  }
  bool raiseAlert(const PnAlert &alert, bool needAck = true) {
    return send(PN_ALERT, &alert, sizeof(alert), PN_BROADCAST_ID, needAck);
  }
  bool sendTelemetry(const PnTelemetry &t, uint16_t dest = PN_BROADCAST_ID) {
    return send(PN_TELEMETRY, &t, sizeof(t), dest, false);
  }
  bool sendEvidence(const PnEvidence &e) {
    return send(PN_EVIDENCE, &e, sizeof(e), PN_BROADCAST_ID, false);
  }
  bool sendCommand(uint8_t command, uint8_t arg = 0,
                   uint16_t dest = PN_BROADCAST_ID, const char *note = "") {
    PnCommand c = {};
    c.command = command;
    c.arg     = arg;
    pnCopyStr(c.note, sizeof(c.note), note);
    return send(PN_COMMAND, &c, sizeof(c), dest, false);
  }
  bool say(const char *text, uint16_t dest = PN_BROADCAST_ID) {
    char buf[64];
    pnCopyStr(buf, sizeof(buf), text);
    return send(PN_TEXT, buf, (uint8_t)(strlen(buf) + 1), dest, false);
  }

  // ------------------------- payload unpacking ----------------------
  // Safe reads: they check the length first, so a corrupted or
  // wrong-version packet returns false instead of scribbling on memory.
  bool asTarget(const PnMessage &m, PnTargetReport &out) const {
    return _unpack(m, PN_TARGET, &out, sizeof(out));
  }
  bool asAlert(const PnMessage &m, PnAlert &out) const {
    return _unpack(m, PN_ALERT, &out, sizeof(out));
  }
  bool asCommand(const PnMessage &m, PnCommand &out) const {
    return _unpack(m, PN_COMMAND, &out, sizeof(out));
  }
  bool asEvidence(const PnMessage &m, PnEvidence &out) const {
    return _unpack(m, PN_EVIDENCE, &out, sizeof(out));
  }
  bool asTelemetry(const PnMessage &m, PnTelemetry &out) const {
    return _unpack(m, PN_TELEMETRY, &out, sizeof(out));
  }

  // ------------------------- status / roster ------------------------
  uint16_t id() const      { return _nodeId; }
  uint8_t  role() const    { return _role; }
  uint8_t  channel() const { return _channel; }
  uint8_t  netId() const   { return _netId; }
  const char *macStr() const { return _macStr; }

  int  peerCount() const;                   // nodes heard from recently
  const PnPeer *peer(int index) const;      // walk the roster, 0..PN_MAX_PEERS-1
  const PnPeer *peerById(uint16_t id) const;
  bool  isAwaitingAck() const { return _pending.active; }
  uint32_t sentCount() const     { return _txTotal; }
  uint32_t deliveredCount() const{ return _txDelivered; }
  uint32_t receivedCount() const { return _rxTotal; }
  uint32_t ignoredCount() const  { return _rxForeign; }  // wrong net/protocol
  uint32_t duplicateCount() const{ return _rxDuplicate; }

  // Print the roster and link statistics. Every line starts with "# "
  // so the base station's serial parser treats it as a comment.
  void printRoster(Stream &out = Serial) const;
  void printMessage(const PnMessage &m, int8_t rssi, Stream &out = Serial) const;

  // --------------------------- behaviour ----------------------------
  // Repeat messages meant for other nodes, extending range (STEP 7).
  // On by default. Turn it off on a node that must stay quiet.
  void setRelay(bool on)        { _relay = on; }
  // Whether this node answers PN_FLAG_NEEDS_ACK. Default: only the base.
  // If nothing answers, senders will retry and report NO-ACK, which is
  // honest but noisy; if everything answers, you get an ack storm.
  void setAckResponder(bool on) { _ackResponder = on; }
  void setHelloInterval(uint32_t ms) { _helloEveryMs = ms; }
  void setDefaultTtl(uint8_t ttl)    { _ttl = ttl ? ttl : 1; }
  void setVerbose(bool on)      { _verbose = on; }

  // Trade data rate for distance. EVERY node must call this, and a node
  // that calls it can no longer join a WiFi router (see STEP 7).
  bool enableLongRange();

  // ---- internal, called from the C callbacks. Not for sketches. ----
  void _handleRx(const uint8_t *src, const uint8_t *data, int len, int8_t rssi);
  void _handleTx(esp_now_send_status_t status);

 private:
  bool _unpack(const PnMessage &m, uint8_t type, void *out, size_t size) const {
    if (m.type != type || m.payloadLen < size) return false;
    memcpy(out, m.payload, size);
    return true;
  }
  bool _rawSend(const PnMessage &m);
  bool _ensurePeer(const uint8_t *mac);
  void _learnPeer(uint16_t id, uint8_t role, const uint8_t *mac,
                  int8_t rssi, uint32_t seq, bool direct);
  bool _seenBefore(uint16_t origin, uint32_t seq);
  void _process(const PnMessage &m, int8_t rssi, const uint8_t *mac);
  void _sendAck(const PnMessage &m);

  // identity
  uint16_t _nodeId       = 0;
  uint8_t  _role         = PN_ROLE_UNKNOWN;
  uint8_t  _channel      = 1;
  uint8_t  _netId        = 0;
  char     _macStr[18]   = {0};
  bool     _started      = false;

  // behaviour
  MessageHandler _handler = nullptr;
  bool     _relay        = true;
  bool     _ackResponder = false;
  bool     _verbose      = true;
  uint8_t  _ttl          = PN_DEFAULT_TTL;
  uint32_t _helloEveryMs = PN_HELLO_EVERY_MS;

  // counters
  uint32_t _seq          = 0;
  uint32_t _txTotal      = 0;
  uint32_t _txDelivered  = 0;   // the radio saw the frame leave successfully
  uint32_t _rxTotal      = 0;
  uint32_t _rxForeign    = 0;
  uint32_t _rxDuplicate  = 0;
  unsigned long _lastHello = 0;

  // roster
  PnPeer   _peers[PN_MAX_PEERS] = {};

  // duplicate filter: a ring of the last N (origin, seq) pairs we handled
  struct SeenEntry { uint16_t origin; uint32_t seq; bool used; };
  SeenEntry _seen[PN_SEEN_HISTORY] = {};
  uint8_t   _seenNext = 0;

  // one outstanding acknowledgement at a time - enough for events that
  // happen seconds apart, and it keeps the memory cost near zero
  struct Pending {
    bool          active;
    PnMessage     msg;
    uint8_t       triesLeft;
    unsigned long lastTryMs;
  } _pending = {};

  // relay queue: messages waiting to be repeated after a random stagger
  struct RelaySlot { bool used; PnMessage msg; unsigned long dueMs; };
  RelaySlot _relayQueue[4] = {};

  // Hand-off from the radio task to loop(). The callback writes at
  // _rxHead, loop() reads at _rxTail; the spinlock keeps the two from
  // tripping over each other on a dual-core chip. If loop() is too slow
  // and the ring fills, the OLDEST unread message is dropped and counted
  // - better to lose one late reading than to block the radio.
  struct RxSlot { PnMessage msg; int8_t rssi; uint8_t mac[6]; };
  RxSlot        _rxQueue[PN_RX_QUEUE] = {};
  volatile uint8_t _rxHead = 0;
  volatile uint8_t _rxTail = 0;
  uint32_t      _rxOverflow = 0;
  portMUX_TYPE  _rxLock = portMUX_INITIALIZER_UNLOCKED;
};

// The one and only network object. Sketches use it as `PorpoiseNet.xxx()`.
static PorpoiseNetClass PorpoiseNet;

// ================================================== radio callbacks ===
// These run inside the WiFi task, NOT in loop(). They must be short and
// must not print, allocate or block - so they only copy bytes and set a
// flag. Everything interesting happens later, in loop().
static void pnRecvCallback(PN_RECV_ARGS) {
  PorpoiseNet._handleRx(PN_RECV_SRC, pnData, pnLen, PN_RECV_RSSI);
}

static void pnSendCallback(PN_SEND_ARGS) {
  // The pointer argument (which peer) is deliberately unused: its type
  // changed between core versions and only the status matters here.
  PorpoiseNet._handleTx(pnStatus);
}

// ======================================================================
// IMPLEMENTATION
// Everything below is the machinery. A sketch never needs to read it,
// but it is commented so that a student who wants to know how a mesh
// works can find out without leaving the file.
// ======================================================================

static const uint8_t PN_BROADCAST_MAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// ------------------------------------------------------------- begin --
inline bool PorpoiseNetClass::begin(uint16_t nodeId, uint8_t role,
                                    uint8_t channel, uint8_t netId) {
  _nodeId = nodeId;
  _role   = role;
  _netId  = netId;
  // By default the base station is the one that answers acknowledgement
  // requests. If every node answered, one find would trigger a burst of
  // replies that drowns out the next message.
  _ackResponder = (role == PN_ROLE_BASE);

  if (channel == PN_CHANNEL_FOLLOW_WIFI) {
    // STEP 6b: this board is already joined to a router. Do not touch
    // the mode or the channel - just find out what the router picked.
    uint8_t primary = 0;
    wifi_second_chan_t second;
    if (esp_wifi_get_channel(&primary, &second) == ESP_OK && primary > 0) {
      _channel = primary;
    } else {
      _channel = 1;   // radio not started yet; caller connected too late
    }
  } else {
    // Normal case: no router involved. Station mode, associated with
    // nothing, parked on the channel the whole team agreed on.
    _channel = channel;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(50);
    esp_wifi_set_channel(_channel, WIFI_SECOND_CHAN_NONE);
  }

  // WiFi power save puts the radio to sleep between beacons, which makes
  // a node miss ESP-NOW packets that arrive while it naps. Off, always.
  WiFi.setSleep(false);

  // Read the MAC out of the chip's efuse - valid even before the radio
  // is fully up, unlike WiFi.macAddress() in some core versions.
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  pnMacToStr(mac, _macStr);

  if (esp_now_init() != ESP_OK) {
    Serial.println(F("# PorpoiseNet FATAL: esp_now_init() failed."));
    Serial.println(F("#   The radio did not start. Nothing will send or"));
    Serial.println(F("#   receive. Usually a bad board selection in the"));
    Serial.println(F("#   IDE, or a brownout from a weak USB port."));
    _started = false;
    return false;
  }

  esp_now_register_recv_cb(pnRecvCallback);
  esp_now_register_send_cb(pnSendCallback);

  // The broadcast address is a peer like any other and must be added, or
  // esp_now_send() to FF:FF:FF:FF:FF:FF returns ESP_ERR_ESPNOW_NOT_FOUND.
  // This one line is what makes STEP 4 ("there is no step 4") possible.
  if (!_ensurePeer(PN_BROADCAST_MAC)) {
    Serial.println(F("# PorpoiseNet FATAL: could not add broadcast peer."));
    _started = false;
    return false;
  }

  _started = true;
  randomSeed(esp_random());   // used to stagger relays

  if (_verbose) {
    Serial.println(F("# ============================================"));
    Serial.print  (F("#  PorpoiseNet up - node "));   Serial.print(_nodeId);
    Serial.print  (F(" ("));                          Serial.print(pnRoleName(_role));
    Serial.println(F(")"));
    Serial.print  (F("#  MAC          : ")); Serial.println(_macStr);
    Serial.print  (F("#  Network ID   : ")); Serial.print(_netId);
    Serial.println(F("   (must match every other board)"));
    Serial.print  (F("#  WiFi channel : ")); Serial.print(_channel);
    Serial.println(F("   (must match every other board)"));
    if (channel == PN_CHANNEL_FOLLOW_WIFI) {
      Serial.println(F("#"));
      Serial.print  (F("#  >>> THIS BOARD FOLLOWED THE ROUTER ONTO CHANNEL "));
      Serial.print(_channel);
      Serial.println(F(" <<<"));
      Serial.println(F("#  >>> SET MY_CHANNEL TO THAT NUMBER ON EVERY OTHER BOARD <<<"));
      Serial.println(F("#"));
    }
    Serial.print  (F("#  Relay        : ")); Serial.println(_relay ? F("on") : F("off"));
    Serial.print  (F("#  Answers acks : ")); Serial.println(_ackResponder ? F("yes") : F("no"));
    Serial.println(F("#  No MAC addresses to copy: this node broadcasts and"));
    Serial.println(F("#  learns the others as it hears them."));
    Serial.println(F("# ============================================"));
  }

  // Announce ourselves immediately so the rest of the fleet sees us in
  // their roster without waiting for the first heartbeat.
  PnCommand hello = {};
  send(PN_HELLO, &hello, 0, PN_BROADCAST_ID, false);
  _lastHello = millis();
  return true;
}

// -------------------------------------------------------- peer table --
inline bool PorpoiseNetClass::_ensurePeer(const uint8_t *mac) {
  if (!mac) return false;
  if (esp_now_is_peer_exist(mac)) return true;

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = 0;              // 0 = "use the interface's current channel"
  peer.encrypt = false;          // broadcast cannot be encrypted; see SECURITY
  peer.ifidx   = WIFI_IF_STA;    // must be explicit once AP+STA is possible
  return esp_now_add_peer(&peer) == ESP_OK;
}

inline void PorpoiseNetClass::_learnPeer(uint16_t id, uint8_t role,
                                         const uint8_t *mac, int8_t rssi,
                                         uint32_t seq, bool direct) {
  if (id == PN_BROADCAST_ID || id == _nodeId) return;

  int slot = -1;
  for (int i = 0; i < PN_MAX_PEERS; i++) {
    if (_peers[i].id == id) { slot = i; break; }
  }
  if (slot < 0) {
    for (int i = 0; i < PN_MAX_PEERS; i++) {
      if (_peers[i].id == 0) { slot = i; break; }
    }
  }
  if (slot < 0) {
    // Roster full: evict whoever has been silent longest. On a school
    // network this only happens if node IDs are being handed out twice.
    unsigned long oldest = ~0UL;
    for (int i = 0; i < PN_MAX_PEERS; i++) {
      if (_peers[i].lastHeardMs < oldest) { oldest = _peers[i].lastHeardMs; slot = i; }
    }
    _peers[slot] = PnPeer();
  }

  PnPeer &p = _peers[slot];
  bool isNew = (p.id != id);
  if (isNew) {
    p = PnPeer();
    p.id = id;
    if (_verbose) {
      Serial.print(F("# NEW NODE on the network: id ")); Serial.print(id);
      Serial.print(F(" ("));  Serial.print(pnRoleName(role)); Serial.println(F(")"));
    }
  }
  p.role        = role;
  p.lastHeardMs = millis();
  p.received++;

  // Only a message that reached us directly tells us anything true about
  // that node's MAC or signal strength - a relayed copy carries the
  // relay's radio characteristics, not the origin's.
  if (direct && mac) {
    memcpy(p.mac, mac, 6);
    p.macKnown = true;
    p.rssi     = rssi;
    _ensurePeer(mac);   // now we can send to it directly, not just broadcast
  }

  // Sequence gaps: the honest measure of how lossy the link is. "Link
  // up" and "no messages lost" are different questions.
  if (p.lastSeq != 0 && seq > p.lastSeq + 1) p.missed += (seq - p.lastSeq - 1);
  if (seq > p.lastSeq) p.lastSeq = seq;
}

inline int PorpoiseNetClass::peerCount() const {
  int n = 0;
  for (int i = 0; i < PN_MAX_PEERS; i++) if (_peers[i].id != 0) n++;
  return n;
}

inline const PnPeer *PorpoiseNetClass::peer(int index) const {
  if (index < 0 || index >= PN_MAX_PEERS) return nullptr;
  return _peers[index].id ? &_peers[index] : nullptr;
}

inline const PnPeer *PorpoiseNetClass::peerById(uint16_t id) const {
  for (int i = 0; i < PN_MAX_PEERS; i++) {
    if (_peers[i].id == id) return &_peers[i];
  }
  return nullptr;
}

// --------------------------------------------------- duplicate filter --
// Radios repeat themselves and relays repeat each other, so the same
// message arrives more than once. (origin, seq) identifies a message for
// life; remembering the last two dozen is enough to throw away the
// echoes without remembering anything for long.
inline bool PorpoiseNetClass::_seenBefore(uint16_t origin, uint32_t seq) {
  for (int i = 0; i < PN_SEEN_HISTORY; i++) {
    if (_seen[i].used && _seen[i].origin == origin && _seen[i].seq == seq) return true;
  }
  _seen[_seenNext].origin = origin;
  _seen[_seenNext].seq    = seq;
  _seen[_seenNext].used   = true;
  _seenNext = (_seenNext + 1) % PN_SEEN_HISTORY;
  return false;
}

// ------------------------------------------------------------ sending --
inline bool PorpoiseNetClass::_rawSend(const PnMessage &m) {
  // Send only the bytes actually used: header + payload. A short frame
  // is faster on air and leaves more room in the 250-byte budget.
  const uint8_t len = PN_HEADER_BYTES + m.payloadLen;

  const uint8_t *target = PN_BROADCAST_MAC;
  if (m.dest != PN_BROADCAST_ID) {
    const PnPeer *p = peerById(m.dest);
    // If we have not learned that node's MAC yet we fall back to
    // broadcasting: everyone hears it, but the dest field means only the
    // intended node acts on it. Slightly wasteful, always works.
    if (p && p->macKnown) target = p->mac;
  }

  _txTotal++;
  esp_err_t err = esp_now_send(target, (const uint8_t *)&m, len);
  if (err != ESP_OK && _verbose) {
    Serial.print(F("# send failed, esp_now_send() = ")); Serial.println((int)err);
  }
  return err == ESP_OK;
}

inline bool PorpoiseNetClass::send(uint8_t type, const void *payload,
                                   uint8_t payloadLen, uint16_t dest,
                                   bool needAck) {
  if (!_started) return false;
  if (payloadLen > PN_MAX_PAYLOAD) return false;

  PnMessage m = {};
  m.magic      = PN_MAGIC;
  m.protocol   = PN_PROTOCOL_VERSION;
  m.netId      = _netId;
  m.type       = type;
  m.role       = _role;
  m.ttl        = _ttl;
  m.flags      = needAck ? PN_FLAG_NEEDS_ACK : 0;
  m.origin     = _nodeId;
  m.dest       = dest;
  m.seq        = ++_seq;
  m.originMs   = millis();
  m.payloadLen = payloadLen;
  if (payload && payloadLen) memcpy(m.payload, payload, payloadLen);

  // Remember our own message so that when a relay bounces it back to us
  // we recognise it as ours and ignore it instead of relaying forever.
  _seenBefore(_nodeId, m.seq);

  if (needAck) {
    if (_pending.active && _verbose) {
      Serial.println(F("# NOTE: a previous message was still waiting for an"));
      Serial.println(F("#       ack and has been abandoned. Space out events"));
      Serial.println(F("#       that need confirmation."));
    }
    _pending.active    = true;
    _pending.msg       = m;
    _pending.triesLeft = PN_ACK_RETRIES;
    _pending.lastTryMs = millis();
  }

  return _rawSend(m);
}

inline void PorpoiseNetClass::_sendAck(const PnMessage &m) {
  PnAck a;
  a.ackOrigin = m.origin;
  a.ackSeq    = m.seq;
  send(PN_ACK, &a, sizeof(a), m.origin, false);
}

// ------------------------------------------------------- radio to us --
// Runs in the WiFi task. Validate cheaply, copy, get out. Anything that
// prints or takes time belongs in loop(), not here.
inline void PorpoiseNetClass::_handleRx(const uint8_t *src, const uint8_t *data,
                                        int len, int8_t rssi) {
  if (!data || len < (int)PN_HEADER_BYTES || len > (int)sizeof(PnMessage)) return;

  const PnMessage *in = (const PnMessage *)data;
  // Three cheap rejections, in order of how often they fire:
  //  - wrong magic     : someone else's ESP-NOW traffic entirely
  //  - wrong protocol  : one of our boards running older firmware
  //  - wrong net id    : the other team's field, on our channel
  if (in->magic != PN_MAGIC || in->protocol != PN_PROTOCOL_VERSION ||
      in->netId != _netId) {
    _rxForeign++;
    return;
  }
  // payloadLen must fit inside what actually arrived, or a corrupted
  // packet could make us read past the end of the buffer.
  if ((int)(PN_HEADER_BYTES + in->payloadLen) > len) { _rxForeign++; return; }

  portENTER_CRITICAL_ISR(&_rxLock);
  uint8_t next = (_rxHead + 1) % PN_RX_QUEUE;
  if (next == _rxTail) {
    // Ring full: drop the oldest so the newest still gets through.
    _rxTail = (_rxTail + 1) % PN_RX_QUEUE;
    _rxOverflow++;
  }
  memset(&_rxQueue[_rxHead].msg, 0, sizeof(PnMessage));
  memcpy(&_rxQueue[_rxHead].msg, data, len);
  _rxQueue[_rxHead].rssi = rssi;
  if (src) memcpy(_rxQueue[_rxHead].mac, src, 6);
  else     memset(_rxQueue[_rxHead].mac, 0, 6);
  _rxHead = next;
  portEXIT_CRITICAL_ISR(&_rxLock);
}

inline void PorpoiseNetClass::_handleTx(esp_now_send_status_t status) {
  // WARNING, and this trips people up every year:
  // For a BROADCAST there is no acknowledgement at the radio level, so
  // this callback reports SUCCESS as soon as the frame leaves the
  // antenna - even if nobody is listening, or nobody is switched on.
  // "Delivered" here means "transmitted", not "received". The only real
  // proof that a message arrived is a PN_ACK coming back.
  if (status == ESP_NOW_SEND_SUCCESS) _txDelivered++;
}

// ------------------------------------------------------- processing ---
inline void PorpoiseNetClass::_process(const PnMessage &m, int8_t rssi,
                                       const uint8_t *mac) {
  _rxTotal++;
  const bool direct = !(m.flags & PN_FLAG_RELAYED);
  const bool forMe  = (m.dest == PN_BROADCAST_ID || m.dest == _nodeId);
  const bool wantsAck = (m.flags & PN_FLAG_NEEDS_ACK) && m.type != PN_ACK;

  _learnPeer(m.origin, m.role, mac, rssi, m.seq, direct);

  if (_seenBefore(m.origin, m.seq)) {
    _rxDuplicate++;
    // One exception to "ignore duplicates": if the sender is repeating
    // because our acknowledgement got lost, staying silent would make it
    // retry until it gave up. Answer again, then drop it.
    if (forMe && wantsAck && _ackResponder && m.origin != _nodeId) _sendAck(m);
    return;
  }

  if (m.type == PN_ACK) {
    PnAck a;
    if (m.payloadLen >= sizeof(a)) {
      memcpy(&a, m.payload, sizeof(a));
      if (a.ackOrigin == _nodeId && _pending.active && a.ackSeq == _pending.msg.seq) {
        _pending.active = false;
        if (_verbose) {
          Serial.print(F("# ACK from node ")); Serial.print(m.origin);
          Serial.print(F(" for message #")); Serial.print(a.ackSeq);
          Serial.print(F(" after "));
          Serial.print(PN_ACK_RETRIES - _pending.triesLeft);
          Serial.println(F(" retr(y/ies) - confirmed received."));
        }
      }
    }
  } else if (forMe) {
    if (wantsAck && _ackResponder && m.origin != _nodeId) _sendAck(m);
    if (_handler) _handler(m, rssi);
  }

  // ---- relay ----
  // Repeat anything that still has hops left and was not addressed
  // privately to us. This is the entire routing algorithm: no tables, no
  // discovery, just "say it once more and count down". The duplicate
  // filter above is what stops it echoing around the field forever.
  if (_relay && m.ttl > 1 && m.dest != _nodeId && m.origin != _nodeId) {
    for (int i = 0; i < (int)(sizeof(_relayQueue) / sizeof(_relayQueue[0])); i++) {
      if (_relayQueue[i].used) continue;
      _relayQueue[i].used  = true;
      _relayQueue[i].msg   = m;
      _relayQueue[i].msg.ttl--;
      _relayQueue[i].msg.flags |= PN_FLAG_RELAYED;
      // Random stagger: if three rovers all hear the same message and
      // all repeat it at the same instant, the copies collide in the air
      // and nobody downstream gets any of them.
      _relayQueue[i].dueMs = millis() + random(8, 45);
      break;
    }
  }
}

// ------------------------------------------------------------- loop ---
inline void PorpoiseNetClass::loop() {
  if (!_started) return;

  // ---- 1. Hand everything the radio collected over to the sketch ----
  while (true) {
    RxSlot slot;
    portENTER_CRITICAL(&_rxLock);
    bool has = (_rxTail != _rxHead);
    if (has) {
      slot   = _rxQueue[_rxTail];
      _rxTail = (_rxTail + 1) % PN_RX_QUEUE;
    }
    portEXIT_CRITICAL(&_rxLock);
    if (!has) break;
    _process(slot.msg, slot.rssi, slot.mac);
  }

  const unsigned long now = millis();

  // ---- 2. Send any relays whose stagger has elapsed ----
  for (int i = 0; i < (int)(sizeof(_relayQueue) / sizeof(_relayQueue[0])); i++) {
    if (_relayQueue[i].used && (long)(now - _relayQueue[i].dueMs) >= 0) {
      _rawSend(_relayQueue[i].msg);
      _relayQueue[i].used = false;
    }
  }

  // ---- 3. Chase an unacknowledged message ----
  if (_pending.active && (now - _pending.lastTryMs) >= PN_ACK_TIMEOUT_MS) {
    if (_pending.triesLeft > 0) {
      _pending.triesLeft--;
      _pending.lastTryMs = now;
      if (_verbose) {
        Serial.print(F("# no ack yet for message #"));
        Serial.print(_pending.msg.seq);
        Serial.print(F(", resending ("));
        Serial.print(_pending.triesLeft);
        Serial.println(F(" tries left)"));
      }
      _rawSend(_pending.msg);
    } else {
      _pending.active = false;
      if (_verbose) {
        Serial.print(F("# NO ACK for message #"));
        Serial.print(_pending.msg.seq);
        Serial.println(F(" - it was broadcast, but nothing confirmed it."));
        Serial.println(F("#   Base station powered? Same channel and net id?"));
        Serial.println(F("#   In range? Check the roster."));
      }
    }
  }

  // ---- 4. Heartbeat, so everyone else's roster stays fresh ----
  if (_helloEveryMs && (now - _lastHello) >= _helloEveryMs) {
    _lastHello = now;
    send(PN_HELLO, nullptr, 0, PN_BROADCAST_ID, false);
  }
}

// ------------------------------------------------------- diagnostics --
inline void PorpoiseNetClass::printRoster(Stream &out) const {
  out.print(F("# ROSTER (node ")); out.print(_nodeId);
  out.print(F(" ")); out.print(pnRoleName(_role));
  out.print(F(", net ")); out.print(_netId);
  out.print(F(", ch ")); out.print(_channel);
  out.println(F(")"));

  int shown = 0;
  for (int i = 0; i < PN_MAX_PEERS; i++) {
    const PnPeer &p = _peers[i];
    if (!p.id) continue;
    shown++;
    unsigned long age = millis() - p.lastHeardMs;
    out.print(F("#   id "));       out.print(p.id);
    out.print(F("  "));            out.print(pnRoleName(p.role));
    out.print(F("  rssi "));
    if (p.macKnown) { out.print(p.rssi); out.print(F("dBm")); }
    else            { out.print(F("via relay")); }
    out.print(F("  heard "));      out.print(age / 1000);
    out.print(F("s ago  rx "));    out.print(p.received);
    out.print(F("  lost "));       out.print(p.missed);
    if (age > PN_PEER_STALE_MS) out.print(F("   <-- LOST?"));
    out.println();
  }
  if (shown == 0) {
    out.println(F("#   (nobody) - no other node has been heard yet."));
    out.println(F("#   Check, in this order: is the other board powered and"));
    out.println(F("#   running? Same MY_NET_ID? Same MY_CHANNEL? Within range?"));
  }
  out.print(F("#   links: sent "));      out.print(_txTotal);
  out.print(F("  received "));           out.print(_rxTotal);
  out.print(F("  duplicates ignored ")); out.print(_rxDuplicate);
  out.print(F("  not ours "));           out.print(_rxForeign);
  if (_rxOverflow) {
    out.print(F("  DROPPED (loop too slow) ")); out.print(_rxOverflow);
  }
  out.println();
}

inline void PorpoiseNetClass::printMessage(const PnMessage &m, int8_t rssi,
                                           Stream &out) const {
  out.print(F("# rx "));          out.print(pnTypeName(m.type));
  out.print(F(" from node "));    out.print(m.origin);
  out.print(F(" ("));             out.print(pnRoleName(m.role));
  out.print(F(")  #"));           out.print(m.seq);
  if (m.dest != PN_BROADCAST_ID) { out.print(F("  to ")); out.print(m.dest); }
  if (m.flags & PN_FLAG_RELAYED) out.print(F("  [relayed]"));
  else { out.print(F("  rssi ")); out.print(rssi); out.print(F("dBm")); }
  out.println();
}

// --------------------------------------------------------- long range --
inline bool PorpoiseNetClass::enableLongRange() {
  // Espressif's proprietary LR mode. Lower data rate, much better link
  // budget - roughly 2-4x the distance in the open. Two hard rules:
  //   1. EVERY node must call this. An LR radio and a normal radio
  //      cannot hear each other at all, and neither reports an error.
  //   2. A node in LR-only mode CANNOT join a WiFi router, because no
  //      router speaks LR. Never call this on the camera bridge.
  esp_err_t err = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  if (_verbose) {
    if (err == ESP_OK) {
      Serial.println(F("# LONG RANGE mode on. Every other node must also"));
      Serial.println(F("#   call enableLongRange() or it will hear nothing."));
    } else {
      Serial.print(F("# long range mode failed, error ")); Serial.println((int)err);
    }
  }
  return err == ESP_OK;
}

#endif  // PORPOISENET_H
