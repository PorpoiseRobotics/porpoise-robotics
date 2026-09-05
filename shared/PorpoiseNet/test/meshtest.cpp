/* =====================================================================
 * meshtest.cpp  --  does the network logic actually work?
 * ---------------------------------------------------------------------
 * THIS IS NOT FIRMWARE. You never flash this file. It runs on a laptop.
 *
 * It builds three PorpoiseNet nodes inside one program - rover 11, the
 * base station, and rover 12 in the middle - and connects them through a
 * fake radio whose "who can hear whom" table this file controls. That
 * makes it possible to test the parts that are miserable to test in a
 * field: what happens when the base is out of range, when a frame
 * arrives four times, or when nobody answers at all.
 *
 * Run it with ./run_tests.sh
 *
 * Where the real hardware is faked: stubs/. Those files exist only to
 * satisfy the compiler and do nothing an ESP32 would do.
 * ===================================================================== */
// Node roles and types, repeated here so this file does not have to pull
// in the whole header (each node .cpp has its own copy of it).
#define PN_ROLE_VEHICLE 2
#define PN_ROLE_BASE 1
#define PN_TARGET 3
#include "radio.h"
#include <cstdio>
#include <cstdint>
// A = rover 11, B = base 1, C = rover 12 (the middle relay)
void A_begin(uint16_t,uint8_t,bool); void A_loop(); void A_feed(const uint8_t*,int);
bool A_waitingAck(); uint32_t A_dups(); bool A_target();
extern int A_DELIVERED, A_LAST_TYPE, A_LAST_FROM;
void B_begin(uint16_t,uint8_t,bool); void B_loop(); void B_feed(const uint8_t*,int);
bool B_waitingAck(); uint32_t B_dups(); bool B_target();
extern int B_DELIVERED, B_LAST_TYPE, B_LAST_FROM;
void C_begin(uint16_t,uint8_t,bool); void C_loop(); void C_feed(const uint8_t*,int);
bool C_waitingAck(); uint32_t C_dups(); bool C_target();
extern int C_DELIVERED, C_LAST_TYPE, C_LAST_FROM;

int failures = 0;
void check(const char *what, bool ok) {
  printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) failures++;
}
// Run every node's loop() and deliver whatever they transmit, according
// to a "who can hear whom" table. 0 = out of range.
void step(bool aHearsB, bool aHearsC, bool bHearsA, bool bHearsC, bool cHearsA, bool cHearsB) {
  Frame air[64]; int n = g_airCount;
  for (int i = 0; i < n; i++) air[i] = g_air[i];
  clearAir();
  for (int i = 0; i < n; i++) {
    int f = air[i].from;
    if (f != 1 && ((f == 2 && aHearsB) || (f == 3 && aHearsC))) { g_sender = 1; A_feed(air[i].data, air[i].len); }
    if (f != 2 && ((f == 1 && bHearsA) || (f == 3 && bHearsC))) { g_sender = 2; B_feed(air[i].data, air[i].len); }
    if (f != 3 && ((f == 1 && cHearsA) || (f == 2 && cHearsB))) { g_sender = 3; C_feed(air[i].data, air[i].len); }
  }
  g_now += 60;
  g_sender = 1; A_loop();
  g_sender = 2; B_loop();
  g_sender = 3; C_loop();
}

int main() {
  g_now = 1000;
  g_sender = 1; A_begin(11, PN_ROLE_VEHICLE, true);
  g_sender = 2; B_begin(1,  PN_ROLE_BASE,    true);
  g_sender = 3; C_begin(12, PN_ROLE_VEHICLE, true);
  clearAir();

  printf("\nTEST 1  everyone in range: a find reaches the base and is acknowledged\n");
  g_sender = 1; A_target();
  int before = B_DELIVERED;
  for (int i = 0; i < 8; i++) step(1,1,1,1,1,1);
  check("base received exactly one TARGET", B_DELIVERED - before == 1);
  check("base saw it came from rover 11", B_LAST_FROM == 11 && B_LAST_TYPE == PN_TARGET);
  check("rover 12 also heard the find directly", C_LAST_FROM == 11 && C_LAST_TYPE == PN_TARGET);
  check("rover 11 got its acknowledgement", !A_waitingAck());
  check("nobody delivered a duplicate to the sketch",
        B_DELIVERED - before == 1);
  printf("        (duplicate frames silently dropped: A=%u B=%u C=%u)\n",
         A_dups(), B_dups(), C_dups());

  printf("\nTEST 2  base out of range: rover 12 relays the find across\n");
  int bBefore = B_DELIVERED, cBefore = C_DELIVERED;
  g_sender = 1; A_target();
  //        aHearsB aHearsC bHearsA bHearsC cHearsA cHearsB
  for (int i = 0; i < 10; i++) step(0, 1, 0, 1, 1, 0);
  check("rover 12 heard the find directly", C_DELIVERED - cBefore == 1);
  check("base received it anyway, via the relay", B_DELIVERED - bBefore == 1);
  check("base saw the ORIGIN as rover 11, not the relay", B_LAST_FROM == 11);

  printf("\nTEST 3  no base at all: sender retries, then reports failure\n");
  g_sender = 1; A_target();
  for (int i = 0; i < 3; i++) step(0,0,0,0,0,0);
  check("still chasing an acknowledgement", A_waitingAck());
  for (int i = 0; i < 40; i++) step(0,0,0,0,0,0);
  check("gave up after its retries instead of hanging", !A_waitingAck());

  printf("\nTEST 4  a repeated frame is only delivered to the sketch once\n");
  clearAir();
  g_sender = 3; C_target();
  Frame copy = g_air[0];
  int b2 = B_DELIVERED;
  for (int r = 0; r < 4; r++) { g_sender = 2; B_feed(copy.data, copy.len); }
  g_now += 60; g_sender = 2; B_loop();
  check("four identical frames -> one delivery", B_DELIVERED - b2 == 1);

  printf("\n%s  (%d failure%s)\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED",
         failures, failures == 1 ? "" : "s");
  return failures;
}
