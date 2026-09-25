#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
typedef bool boolean;
#define F(x) (x)
#define PROGMEM
unsigned long millis();
void delay(unsigned long);
long random(long, long);
void randomSeed(unsigned long);
class Stream {
 public:
  void print(const char*); void print(int); void print(unsigned int);
  void print(long); void print(unsigned long); void print(double, int = 2);
  void print(float, int); void print(char);
  void println(const char*); void println(int); void println(unsigned int);
  void println(long); void println(unsigned long); void println();
  void println(double, int = 2);
  void print(unsigned char, int);
  void print(int, int);
  void println(int, int);
  void begin(unsigned long);
  int available(); int read();
};
extern Stream Serial;
typedef struct { int owner; } portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED {0}
void portENTER_CRITICAL(portMUX_TYPE*);
void portEXIT_CRITICAL(portMUX_TYPE*);
void portENTER_CRITICAL_ISR(portMUX_TYPE*);
void portEXIT_CRITICAL_ISR(portMUX_TYPE*);
uint32_t esp_random();
// ---- extra Arduino surface used by the sketches ----
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define LED_BUILTIN 2
#define DEC 10
#define HEX 16
#define SERIAL_8N1 0x800001c
void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int  digitalRead(uint8_t);
void delayMicroseconds(unsigned int);
unsigned long pulseIn(uint8_t, uint8_t, unsigned long);
bool psramFound();
class String {
 public:
  String(); String(const char*); String(int); String(unsigned int);
  void trim(); void toLowerCase(); unsigned length() const;
  int indexOf(char) const; String substring(int) const; String substring(int,int) const;
  long toInt() const; const char* c_str() const;
  String& operator+=(char); bool operator==(const char*) const;
  operator const char*() const;
};
class HardwareSerial : public Stream {
 public:
  HardwareSerial(int);
  void begin(unsigned long, uint32_t = SERIAL_8N1, int8_t = -1, int8_t = -1);
  int available(); int read();
};
