#define ESP_ARDUINO_VERSION_VAL(a,b,c) ((a)*10000+(b)*100+(c))
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(3,0,7)
#include "PorpoiseNet.h"
#include "radio.h"
#include <cstdio>
int  NODE_DELIVERED = 0;      // how many messages reached the sketch handler
int  NODE_LAST_TYPE = 0;
int  NODE_LAST_FROM = 0;
static void h(const PnMessage &m, int8_t) { NODE_DELIVERED++; NODE_LAST_TYPE=m.type; NODE_LAST_FROM=m.origin; }
void NODE_begin(uint16_t id, uint8_t role, bool relay) {
  PorpoiseNet.setVerbose(false);
  PorpoiseNet.begin(id, role, 1, 7);
  PorpoiseNet.onMessage(h);
  PorpoiseNet.setRelay(relay);
  PorpoiseNet.setHelloInterval(0);          // no heartbeats: keeps the test readable
}
void NODE_loop() { PorpoiseNet.loop(); }
void NODE_feed(const uint8_t *d, int n) { PorpoiseNet._handleRx((const uint8_t*)"\1\2\3\4\5\6", d, n, -50); }
bool NODE_waitingAck() { return PorpoiseNet.isAwaitingAck(); }
uint32_t NODE_dups() { return PorpoiseNet.duplicateCount(); }
bool NODE_target() { PnTargetReport r={}; r.confidence=99; pnCopyStr(r.note,sizeof(r.note),"cone"); return PorpoiseNet.reportTarget(r,true); }

// ---- extra helpers used by stationtest.cpp (several stations on one field) ----
char NODE_LAST_CHAT[PN_CHAT_MAX_TEXT + 1] = {0};
int  NODE_CHATS = 0, NODE_TFULL = 0, NODE_MARKS = 0, NODE_LIST_COUNT = -1;
int  NODE_CONFIRMED = 0, NODE_FAILED = 0;
static void hx(const PnMessage &m, int8_t r) {
  h(m, r);
  PnChat c; PnMark mk; PnMarkList l; PnTelemetryFull t;
  if (PorpoiseNet.asChat(m, c)) { NODE_CHATS++; strcpy(NODE_LAST_CHAT, c.text); }
  if (PorpoiseNet.asTelemetryFull(m, t)) NODE_TFULL++;
  if (PorpoiseNet.asMark(m, mk)) NODE_MARKS++;
  if (PorpoiseNet.asMarkList(m, l)) NODE_LIST_COUNT = l.count;
}
static void rx(uint16_t, uint32_t, bool ok) { if (ok) NODE_CONFIRMED++; else NODE_FAILED++; }
void NODE_begin2(uint16_t id, uint8_t role) {
  NODE_begin(id, role, true);
  PorpoiseNet.onMessage(hx);
  PorpoiseNet.onSendResult(rx);
}
void NODE_feedFrom(const uint8_t *mac, const uint8_t *d, int n) { PorpoiseNet._handleRx(mac, d, n, -50); }
bool NODE_chat(uint16_t dest, const char *text, bool ack) { return PorpoiseNet.sendChat(text, dest, ack); }
bool NODE_tfull() { PnTelemetryFull t = {}; t.latitude = 32.7; t.headingDeg = 90; return PorpoiseNet.sendTelemetryFull(t); }
bool NODE_mark(uint16_t id) {
  PnMark m = {}; m.action = PN_MARK_ADD; m.kind = PN_MARK_KIND_HAZARD; m.markId = id;
  pnCopyStr(m.label, sizeof(m.label), "big rock");
  return PorpoiseNet.sendMark(m);
}
bool NODE_markList(int n) {
  PnMarkList l = {}; l.count = (uint8_t)n;
  for (int i = 0; i < n; i++) l.ids[i] = (uint16_t)(i + 1);
  return PorpoiseNet.sendMarkList(l);
}
void NODE_suppress(bool on) { PorpoiseNet.setRelaySuppression(on); }
uint32_t NODE_relayed() { return PorpoiseNet.relayedCount(); }
uint32_t NODE_skipped() { return PorpoiseNet.relaySkippedCount(); }
int  NODE_pending() { return PorpoiseNet.pendingAckCount(); }
int  NODE_peers() { return PorpoiseNet.peerCount(); }
int  NODE_canUnicast(uint16_t id) {
  const PnPeer *p = PorpoiseNet.peerById(id);
  return p ? (p->canUnicast ? 1 : 0) : -1;
}
// Build a HELLO frame as if it came from another board, for filling the roster.
int NODE_fakeHello(uint16_t origin, uint32_t seq, uint8_t *out) {
  PnMessage m = {};
  m.magic = PN_MAGIC; m.protocol = PN_PROTOCOL_VERSION; m.netId = PorpoiseNet.netId();
  m.type = PN_HELLO; m.role = PN_ROLE_VEHICLE; m.ttl = 1; m.origin = origin; m.seq = seq;
  memcpy(out, &m, PN_HEADER_BYTES);
  return (int)PN_HEADER_BYTES;
}
