#ifndef ESP_NOW_RECEIVER
#define ESP_NOW_RECEIVER

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

void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len);
void espnow_receiver_init(void);

void espnow_receiver_store_init(void);
bool espnow_receiver_get_latest(espnow_payload_t *out);


#endif // ESP_NOW_RECEIVER