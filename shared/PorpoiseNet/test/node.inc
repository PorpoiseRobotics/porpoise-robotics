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
