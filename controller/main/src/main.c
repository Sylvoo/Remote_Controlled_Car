#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"


#include "esp_http_client.h"

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include <stdio.h>
#include "freertos/portmacro.h"

#include "wifi.h"
#include "joystick.h" 
#include "esp_now_sender.h"


#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_now.h"
#include "esp_wifi_types.h"  

#include "inttypes.h"
#include "math.h"
#include "esp_err.h"


// static inline int64_t now_us(void)
// {
//     return esp_timer_get_time();
// }


#define JOY_X_CH   ADC_CHANNEL_6
#define JOY_Y_CH   ADC_CHANNEL_7

static const char *TAG = "Wifi station";
static const char *TAG2 = "Remote Controller";
static const char *TAG3 = "ADC";
static const char *TAG4 = "ESP_NOW_SENDER";

int joystickButtonState = 0;
const gpio_num_t joystick_SW = GPIO_NUM_25;
const gpio_num_t xValuePin = GPIO_NUM_34;
const gpio_num_t yValuePin = GPIO_NUM_35;

static const uint8_t PEER_MAC[ESP_NOW_ETH_ALEN] = { 0x44, 0x1D, 0x64, 0xF8, 0xFE, 0x5C }; // 44:1D:64:F8:FE:5C
static uint32_t g_seq = 0;

uint8_t conversion(int value)
{
    const int maxInValue = 4095;
    const int maxOutValue = 255;

    double normalize = ((double)value / (double)maxInValue) * (double)maxOutValue;

    
    uint8_t normalized = (uint8_t)round(normalize);
    return normalized;
}

uint8_t x_ValueNormalized = 0;
uint8_t y_ValueNormalized = 0;

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }
    wifi_init_sta();
    adc_joystick_init();
    espnow_init_peer();

   
    while(1)
    {
        joystick_adc_t xyData = joystick_read();
        ESP_LOGI(TAG3, "X: %dmV | Y: %dmV", xyData.x, xyData.y);
        x_ValueNormalized = conversion(xyData.x);
        y_ValueNormalized = conversion(xyData.y);

        espnow_payload_t payload = {
            .x_v = x_ValueNormalized,
            .y_v = y_ValueNormalized,
            .btn = joystickButtonState,
            .seq = g_seq++,
        };

        esp_err_t err = esp_now_send(PEER_MAC, (const uint8_t*)&payload, sizeof(payload));
        if(err != ESP_OK)
        {
            ESP_LOGE(TAG4, "esp_now_send failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    adc_deinit();

}
