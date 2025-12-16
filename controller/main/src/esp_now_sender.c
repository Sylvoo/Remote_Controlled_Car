#include "esp_now_sender.h"

#include <string.h>
#include "esp_log.h"

static const char *TAG4 = "ESP_NOW_SENDER";

extern const uint8_t PEER_MAC[ESP_NOW_ETH_ALEN] = { 0x44, 0x1D, 0x64, 0xF8, 0xFE, 0x5C }; // 44:1D:64:F8:FE:5C

void espnow_send_cb(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    if (!tx_info) return;

    const uint8_t *mac = tx_info->des_addr;   // jeśli kompilator krzyczy, patrz niżej

    ESP_LOGI(TAG4, "ESP-NOW send -> %02X:%02X:%02X:%02X:%02X:%02X : %s",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
             (status == ESP_NOW_SEND_SUCCESS) ? "OK" : "FAIL");
}


esp_err_t espnow_init_peer(void)
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));

    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, PEER_MAC, ESP_NOW_ETH_ALEN);
    peer.ifidx = ESP_IF_WIFI_STA;
    peer.encrypt = false;
    peer.channel = 0; // czyli ten aktualnie uzywany

    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
    ESP_LOGI(TAG4, "ESP-NOW sender initialized");
    return ESP_OK;
}
