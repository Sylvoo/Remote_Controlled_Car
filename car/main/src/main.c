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
#include <stdint.h>

#include "esp_now.h"
#include "esp_wifi_types.h"  

#include "inttypes.h"
#include "math.h"
#include "esp_err.h"
#include "driver/ledc.h"

#include "wifi.h"
#include "esp_now_receiver.h"
#include "blinkers.h"
#include "motor.h"

// static const char *TAG = "Wifi station";
// static const char *TAG4 = "ESP_NOW_RECEIVER";


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
    espnow_receiver_store_init();
    espnow_receiver_init();
    motor_gpio_init();
    blinkers_gpio_init();
    motor_pwm_init();
    
   
    while(1)
    {
        drive_actions();
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
