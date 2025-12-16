#ifndef ESP_NOW_SENDER_H
#define ESP_NOW_SENDER_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_now.h"
#include "esp_wifi_types.h"

// typedef struct __attribute__((packed)) --> eliminuje padding, wysylamy mniej bitow niepotrzebnych
typedef struct __attribute__((packed)){
    int16_t x_v;   
    int16_t y_v;   
    uint8_t btn;   
    uint32_t seq;   
} espnow_payload_t;

void espnow_send_cb(const wifi_tx_info_t *tx_info, esp_now_send_status_t status);

esp_err_t espnow_init_peer(void);

#endif // ESP_NOW_SENDER_H