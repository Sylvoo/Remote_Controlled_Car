#include "esp_now_receiver.h"

#include <string.h>
#include "esp_log.h"

static const char *TAG4 = "ESP_NOW_SENDER";

extern const uint8_t PEER_MAC[ESP_NOW_ETH_ALEN]; // 44:1D:64:F8:FE:5C

void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    const uint8_t *src = recv_info->src_addr;

    ESP_LOGI(TAG4, "RX from %02X:%02X:%02X:%02X:%02X:%02X len=%d",
             src[0], src[1], src[2], src[3], src[4], src[5], data_len);

    if (data_len == sizeof(espnow_payload_t))
    {
        espnow_payload_t p;
        memcpy(&p, data, sizeof(p));

        ESP_LOGI(TAG4, "payload: x=%d y=%d btn=%u seq=%d",
                 p.x_v, p.y_v, p.btn, p.seq);
    }
}

void espnow_receiver_init(void)
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    ESP_LOGI(TAG4, "ESP-NOW receiver initialized");
}