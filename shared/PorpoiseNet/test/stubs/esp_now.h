#pragma once
#include <esp_wifi.h>
#define ESP_NOW_ETH_ALEN 6
typedef enum { ESP_NOW_SEND_SUCCESS, ESP_NOW_SEND_FAIL } esp_now_send_status_t;
typedef struct {
  uint8_t peer_addr[6]; uint8_t lmk[16]; uint8_t channel;
  wifi_interface_t ifidx; bool encrypt; void *priv;
} esp_now_peer_info_t;
typedef struct {
  uint8_t *srcaddr; uint8_t *des_addr; wifi_pkt_rx_ctrl_t *rx_ctrl;
  uint8_t *src_addr;
} esp_now_recv_info_t;
esp_err_t esp_now_init();
esp_err_t esp_now_add_peer(const esp_now_peer_info_t*);
bool esp_now_is_peer_exist(const uint8_t*);
esp_err_t esp_now_send(const uint8_t*, const uint8_t*, size_t);
esp_err_t esp_now_register_recv_cb(void(*)(const esp_now_recv_info_t*, const uint8_t*, int));
esp_err_t esp_now_register_send_cb(void(*)(const uint8_t*, esp_now_send_status_t));
// extra overloads so the stub accepts the core-2.x and core-3.1+ shapes too
esp_err_t esp_now_register_recv_cb(void(*)(const uint8_t*, const uint8_t*, int));
esp_err_t esp_now_register_send_cb(void(*)(const wifi_tx_info_t*, esp_now_send_status_t));
