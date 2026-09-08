#pragma once
#include <Arduino.h>
typedef int esp_err_t;
#define ESP_OK 0
enum wifi_second_chan_t { WIFI_SECOND_CHAN_NONE };
enum wifi_interface_t { WIFI_IF_STA, WIFI_IF_AP };
#define WIFI_PROTOCOL_11B 1
#define WIFI_PROTOCOL_11G 2
#define WIFI_PROTOCOL_11N 4
#define WIFI_PROTOCOL_LR  8
esp_err_t esp_wifi_set_channel(uint8_t, wifi_second_chan_t);
esp_err_t esp_wifi_get_channel(uint8_t*, wifi_second_chan_t*);
esp_err_t esp_wifi_set_protocol(wifi_interface_t, uint8_t);
typedef struct { signed rssi : 8; } wifi_pkt_rx_ctrl_t;
typedef struct { const uint8_t *des_addr; } wifi_tx_info_t;
