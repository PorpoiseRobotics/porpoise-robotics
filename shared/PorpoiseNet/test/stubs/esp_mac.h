#pragma once
#include <esp_wifi.h>
enum esp_mac_type_t { ESP_MAC_WIFI_STA };
esp_err_t esp_read_mac(uint8_t*, esp_mac_type_t);
