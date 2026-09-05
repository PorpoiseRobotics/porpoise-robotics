#pragma once
#include <Arduino.h>
enum wifi_mode_t { WIFI_OFF, WIFI_STA, WIFI_AP, WIFI_AP_STA };
class WiFiClass {
 public:
  void mode(wifi_mode_t); void disconnect(); void setSleep(bool);
  const char* macAddress();
  int status(); void begin(const char*, const char*);
  const char* localIP();
};
#define WL_CONNECTED 3
extern WiFiClass WiFi;
