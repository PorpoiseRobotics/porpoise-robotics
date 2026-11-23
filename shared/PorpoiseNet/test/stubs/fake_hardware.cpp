#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "esp_mac.h"
Stream Serial; WiFiClass WiFi;
#include "radio.h"
Frame g_air[64]; int g_airCount=0; int g_sender=0; unsigned long g_now=0;
long g_rand[9]={0}; int g_peerLimit=20;
static uint8_t s_peers[9][32][6]; static int s_npeers[9]={0};
void clearAir(){g_airCount=0;}
unsigned long millis(){return g_now;} void delay(unsigned long){}
long random(long,long){return (g_sender>=0&&g_sender<9)?g_rand[g_sender]:0;} void randomSeed(unsigned long){}
void Stream::print(const char*){} void Stream::print(int){} void Stream::print(unsigned int){}
void Stream::print(long){} void Stream::print(unsigned long){} void Stream::print(double,int){}
void Stream::print(float,int){} void Stream::print(char){} void Stream::print(unsigned char,int){}
void Stream::print(int,int){} void Stream::println(int,int){}
void Stream::println(const char*){} void Stream::println(int){} void Stream::println(unsigned int){}
void Stream::println(long){} void Stream::println(unsigned long){} void Stream::println(){}
void Stream::println(double,int){} void Stream::begin(unsigned long){}
int Stream::available(){return 0;} int Stream::read(){return -1;}
void portENTER_CRITICAL(portMUX_TYPE*){} void portEXIT_CRITICAL(portMUX_TYPE*){}
void portENTER_CRITICAL_ISR(portMUX_TYPE*){} void portEXIT_CRITICAL_ISR(portMUX_TYPE*){}
uint32_t esp_random(){return 0;}
void WiFiClass::mode(wifi_mode_t){} void WiFiClass::disconnect(){} void WiFiClass::setSleep(bool){}
const char* WiFiClass::macAddress(){return "";}
esp_err_t esp_wifi_set_channel(uint8_t, wifi_second_chan_t){return 0;}
esp_err_t esp_wifi_get_channel(uint8_t* p, wifi_second_chan_t*){*p=1;return 0;}
esp_err_t esp_wifi_set_protocol(wifi_interface_t, uint8_t){return 0;}
esp_err_t esp_read_mac(uint8_t*, esp_mac_type_t){return 0;}
esp_err_t esp_now_init(){return 0;}
// A small per-board peer list, so a test can fill it up like the real radio's.
esp_err_t esp_now_add_peer(const esp_now_peer_info_t* p){
  int s = (g_sender>=0&&g_sender<9)?g_sender:0;
  if (s_npeers[s] >= g_peerLimit) return 0x306A;          // ESP_ERR_ESPNOW_FULL
  memcpy(s_peers[s][s_npeers[s]++], p->peer_addr, 6); return 0; }
bool esp_now_is_peer_exist(const uint8_t* m){
  int s = (g_sender>=0&&g_sender<9)?g_sender:0;
  for (int i=0;i<s_npeers[s];i++) if (!memcmp(s_peers[s][i],m,6)) return true;
  return false; }
esp_err_t esp_now_send(const uint8_t* to, const uint8_t* d, size_t n){
  if (g_airCount < 64) { g_air[g_airCount].from=g_sender; g_air[g_airCount].len=(int)n;
    static const uint8_t bc[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    g_air[g_airCount].bcast = !to || !memcmp(to,bc,6);
    if (to) memcpy(g_air[g_airCount].to,to,6);
    for(size_t i=0;i<n && i<260;i++) g_air[g_airCount].data[i]=d[i];
    g_airCount++; }
  return 0; }
esp_err_t esp_now_register_recv_cb(void(*)(const esp_now_recv_info_t*, const uint8_t*, int)){return 0;}
esp_err_t esp_now_register_send_cb(void(*)(const uint8_t*, esp_now_send_status_t)){return 0;}
