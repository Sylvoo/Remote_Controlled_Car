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

#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


static inline int64_t now_us(void)
{
    return esp_timer_get_time();
}


#define JOY_X_CH   ADC_CHANNEL_6
#define JOY_Y_CH   ADC_CHANNEL_7

static const char *TAG = "Wifi station";
static const char *TAG2 = "Remote Controller";
static const char *TAG3 = "ADC";



int joystickButtonState = 0;
const gpio_num_t joystick_SW = GPIO_NUM_25;
const gpio_num_t xValuePin = GPIO_NUM_34;
const gpio_num_t yValuePin = GPIO_NUM_35;


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

   
    while(1)
    {
        joystick_adc_t xyData = joystick_read();
        ESP_LOGI(TAG3, "X: %dmV | Y: %dmV", xyData.x, xyData.y);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    adc_deinit();

}
