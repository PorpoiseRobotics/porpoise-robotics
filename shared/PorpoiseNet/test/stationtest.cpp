/* =====================================================================
 * stationtest.cpp  --  several Find the Target stations on one field
 * ---------------------------------------------------------------------
 * THIS IS NOT FIRMWARE. It runs on a laptop, like meshtest.cpp.
 *
 * Six boards share one fake radio: three station radios (node 1, 2, 3,
 * the boards on each laptop's USB port) and their three vehicles (node
 * 11, 12, 13). Unlike meshtest, every board here has its own MAC
 * address, and a frame sent to one MAC only reaches that board - so a
 * test can check what happens when a learned address goes stale.
 *
 * Run it with ./run_tests.sh
 * ===================================================================== */
#define PN_ROLE_VEHICLE 2
#define PN_ROLE_BASE 1
#include "radio.h"
#include <cstdio>
#include <cstdint>
#include <cstring>

#define NODE_DECLS(N) \
  void N##_begin2(uint16_t,uint8_t); void N##_loop(); \
  void N##_feedFrom(const uint8_t*,const uint8_t*,int); \
  bool N##_chat(uint16_t,const char*,bool); bool N##_tfull(); bool N##_mark(uint16_t); \
  bool N##_markList(int); void N##_suppress(bool); \
  uint32_t N##_relayed(); uint32_t N##_skipped(); int N##_pending(); \
  int N##_peers(); int N##_canUnicast(uint16_t); int N##_fakeHello(uint16_t,uint32_t,uint8_t*); \
  extern int N##_DELIVERED, N##_CHATS, N##_TFULL, N##_MARKS, \
             N##_LIST_COUNT, N##_CONFIRMED, N##_FAILED; \
  extern char N##_LAST_CHAT[];
NODE_DECLS(A) NODE_DECLS(B) NODE_DECLS(C) NODE_DECLS(D) NODE_DECLS(E) NODE_DECLS(F)

// One table so the test can loop over boards. Index 1..6 = g_sender.
struct Board {
  const char *name; uint16_t id;
  void (*loop)(); void (*feed)(const uint8_t*, const uint8_t*, int);
  uint32_t (*relayed)(); uint32_t (*skipped)(); void (*suppress)(bool);
};
#define B_(N, ID) { #N, ID, N##_loop, N##_feedFrom, N##_relayed, N##_skipped, N##_suppress }
Board boards[7] = { {}, B_(A, 1), B_(B, 2), B_(C, 3), B_(D, 11), B_(E, 12), B_(F, 13) };
uint8_t mac[7][6];          // each board's MAC address
bool hears[7][7];           // hears[r][s]: board r can hear board s

int failures = 0;
void check(const char *what, bool ok) {
  printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) failures++;
}
void allInRange(bool on) {
  for (int r = 1; r <= 6; r++)
    for (int s = 1; s <= 6; s++) hears[r][s] = on && r != s;
}
// Deliver what was sent, then let every board run loop() once, 60 ms later.
void step() {
  Frame air[64]; int n = g_airCount;
  for (int i = 0; i < n; i++) air[i] = g_air[i];
  clearAir();
  for (int i = 0; i < n; i++) {
    int s = air[i].from;
    for (int r = 1; r <= 6; r++) {
      if (!hears[r][s]) continue;
      if (!air[i].bcast && memcmp(air[i].to, mac[r], 6) != 0) continue;  // unicast for someone else
      g_sender = r; boards[r].feed(mac[s], air[i].data, air[i].len);
    }
  }
  g_now += 60;
  for (int r = 1; r <= 6; r++) { g_sender = r; boards[r].loop(); }
}
void run(int steps) { for (int i = 0; i < steps; i++) step(); }
uint32_t totalRelayed() { uint32_t t = 0; for (int r = 1; r <= 6; r++) t += boards[r].relayed(); return t; }
uint32_t totalSkipped() { uint32_t t = 0; for (int r = 1; r <= 6; r++) t += boards[r].skipped(); return t; }

int main() {
  g_now = 1000;
  for (int r = 1; r <= 6; r++) { uint8_t m[6] = {0x02, 0, 0, 0, 0, (uint8_t)r}; memcpy(mac[r], m, 6); }
  g_sender = 1; A_begin2(1,  PN_ROLE_BASE);
  g_sender = 2; B_begin2(2,  PN_ROLE_BASE);
  g_sender = 3; C_begin2(3,  PN_ROLE_BASE);
  g_sender = 4; D_begin2(11, PN_ROLE_VEHICLE);
  g_sender = 5; E_begin2(12, PN_ROLE_VEHICLE);
  g_sender = 6; F_begin2(13, PN_ROLE_VEHICLE);
  // A different relay delay on each board, like the real random stagger.
  // Spaced wider than one test step (60 ms) so the order is clear.
  for (int r = 1; r <= 6; r++) g_rand[r] = 70 * (r - 1);
  allInRange(true);
  run(3);   // everyone hears everyone's first HELLO and learns their MACs

  printf("\nTEST 1  a chat to everyone reaches every other station once\n");
  int b0 = B_CHATS, c0 = C_CHATS;
  g_sender = 1; check("send accepted", A_chat(0, "Found the cone by the goal!", false));
  run(4);
  check("station 2 got it once", B_CHATS - b0 == 1);
  check("station 3 got it once", C_CHATS - c0 == 1);
  check("text arrived intact", strcmp(C_LAST_CHAT, "Found the cone by the goal!") == 0);
  check("chat to everyone asks for no ack", A_pending() == 0);

  printf("\nTEST 2  a chat to ONE station is shown only there, and confirmed\n");
  b0 = B_CHATS; c0 = C_CHATS; int conf0 = A_CONFIRMED;
  g_sender = 1; A_chat(3, "Station 3, come help", true);
  run(6);
  check("station 3 got it", C_CHATS - c0 == 1);
  check("station 2 did not show it", B_CHATS - b0 == 0);
  check("station 1 was told it arrived", A_CONFIRMED - conf0 == 1 && A_pending() == 0);

  printf("\nTEST 3  three confirmed chats in a row all arrive, in order\n");
  c0 = C_CHATS; conf0 = A_CONFIRMED;
  g_sender = 1; A_chat(3, "one", true); A_chat(3, "two", true); A_chat(3, "three", true);
  check("all three queued (none abandoned)", A_pending() == 3);
  run(12);
  check("station 3 got all three", C_CHATS - c0 == 3);
  check("the last one shown is the last one sent", strcmp(C_LAST_CHAT, "three") == 0);
  check("all three confirmed", A_CONFIRMED - conf0 == 3);

  printf("\nTEST 4  the confirm queue says no when full, and gives up cleanly\n");
  allInRange(false);
  int fail0 = A_FAILED;
  g_sender = 1;
  bool ok4 = A_chat(2, "a", true) && A_chat(2, "b", true) &&
             A_chat(2, "c", true) && A_chat(2, "d", true);
  check("four fit in the queue", ok4);
  check("a fifth is refused instead of dropping an older one", !A_chat(2, "e", true));
  run(200);
  check("all four reported as not confirmed", A_FAILED - fail0 == 4 && A_pending() == 0);
  allInRange(true);

  printf("\nTEST 5  routine telemetry: one repeat is enough (relay suppression)\n");
  run(2); clearAir();
  uint32_t r0 = totalRelayed(), s0 = totalSkipped();
  int t2 = B_TFULL, t3 = C_TFULL;
  g_sender = 4; D_tfull();
  run(8);
  check("every station radio got the vehicle's telemetry",
        B_TFULL - t2 == 1 && C_TFULL - t3 == 1);
  printf("        (repeats sent: %u, repeats skipped: %u)\n",
         (unsigned)(totalRelayed() - r0), (unsigned)(totalSkipped() - s0));
  check("only one board repeated it", totalRelayed() - r0 == 1);
  check("the other four stood down", totalSkipped() - s0 == 4);

  for (int r = 1; r <= 6; r++) { g_sender = r; boards[r].suppress(false); }
  r0 = totalRelayed();
  g_sender = 4; D_tfull();
  run(8);
  check("with suppression off, all five repeat it (the old behaviour)", totalRelayed() - r0 == 5);
  for (int r = 1; r <= 6; r++) { g_sender = r; boards[r].suppress(true); }

  r0 = totalRelayed();
  g_sender = 1; A_chat(0, "chat is always repeated", false);
  run(8);
  check("chat is still repeated by every board", totalRelayed() - r0 == 5);

  printf("\nTEST 6  a station's board was swapped: the stale MAC does not strand it\n");
  // Station 3 gets a new radio board with a new MAC. Station 1 has not
  // heard from it since, so it still has the OLD MAC on file.
  mac[3][0] = 0x06;
  c0 = C_CHATS; conf0 = A_CONFIRMED;
  g_sender = 1; A_chat(3, "are you there?", true);
  run(40);
  check("station 3 got it (a broadcast retry reached the new board)", C_CHATS - c0 == 1);
  check("and station 1 was told it arrived", A_CONFIRMED - conf0 == 1);

  printf("\nTEST 7  marks, mark lists and long chats survive the trip\n");
  int m2 = B_MARKS;
  B_LIST_COUNT = -1;
  g_sender = 1; A_mark(7); A_markList(3);
  run(4);
  check("station 2 got the mark", B_MARKS - m2 == 1);
  check("station 2 got the mark list with 3 ids", B_LIST_COUNT == 3);
  char longText[200]; memset(longText, 'x', 150); longText[150] = '\0';
  g_sender = 1; A_chat(0, longText, false);
  run(4);
  check("a 150-letter chat is cut to 120, never overflows", strlen(B_LAST_CHAT) == 120);

  printf("\nTEST 8  more boards than the radio's peer list: fall back to broadcast\n");
  // Feed station 2 HELLOs from 20 more boards, each with its own MAC.
  for (int i = 0; i < 20; i++) {
    uint8_t f[64]; uint8_t m[6] = {0x0E, 0, 0, 0, 1, (uint8_t)i};
    g_sender = 2; int n = B_fakeHello((uint16_t)(100 + i), 1, f);
    B_feedFrom(m, f, n);
    B_loop();   // the receive queue holds 16 messages; let loop() empty it
  }
  check("the roster holds 20 boards", B_peers() == 20);
  check("an early board can be sent to directly", B_canUnicast(100) == 1);
  check("a board past the radio's 20-peer limit is marked broadcast-only", B_canUnicast(119) == 0);

  printf("\n%s  (%d failure%s)\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED",
         failures, failures == 1 ? "" : "s");
  return failures;
}
