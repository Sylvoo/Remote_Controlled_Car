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

const uint8_t PEER_MAC[ESP_NOW_ETH_ALEN] = { 0x44, 0x1D, 0x64, 0xF8, 0xFE, 0x5C }; // 44:1D:64:F8:FE:5C

#define HONK GPIO_NUM_5

void honk_cb(void* arg)
{
    if(honk_on)
    {
        gpio_set_level(HONK, 1);
    }
}

void gpio_honk_init()
{
    gpio_config_t honk_io = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1<<HONK) 
    };
    gpio_config(&honk_io);

    const esp_timer_create_args_t honk_args = 
    {
        .callback = &honk_cb,
    };
    ESP_ERROR_CHECK(esp_timer_create(&honk_args, &led_timer_honk));
}

void honk_start(uint64_t us)
{
    honk_on = 1;
    esp_timer_stop(led_timer_honk);
    ESP_ERROR_CHECK(esp_timer_start_periodic(led_timer_honk, us));
}



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
