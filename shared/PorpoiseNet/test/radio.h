#pragma once
#include <cstdint>
#include <cstddef>
struct Frame { int from; uint8_t data[260]; int len; bool bcast; uint8_t to[6]; };
extern Frame g_air[64];
extern int   g_airCount;
extern int   g_sender;
extern unsigned long g_now;
void clearAir();
// Per-board stand-ins for hardware differences (index = g_sender, 1..8):
extern long g_rand[9];          // what random() returns on that board (relay stagger)
extern int  g_peerLimit;        // ESP-NOW peer list size (the real radio: 20)
