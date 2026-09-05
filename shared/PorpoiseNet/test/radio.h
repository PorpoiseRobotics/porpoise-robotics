#pragma once
#include <cstdint>
#include <cstddef>
struct Frame { int from; uint8_t data[260]; int len; };
extern Frame g_air[64];
extern int   g_airCount;
extern int   g_sender;
extern unsigned long g_now;
void clearAir();
