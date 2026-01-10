#include "blinkers.h"
#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_err.h"


bool blinker_left_on = false;
bool blinker_right_on = false;
bool blinker_left_high = false;
bool blinker_right_high = false;

#define BLINKER_BUZZER GPIO_NUM_18
esp_timer_handle_t led_timer = NULL;

void blinker_cb(void* arg)
{
    if(blinker_left_on)
    {
        int nextState= (blinker_left_high == 0) ? 1 : 0;
        gpio_set_level(BLINKER_LEFT, nextState);
        gpio_set_level(BLINKER_BUZZER, nextState);
        blinker_left_high = nextState;
    }
    if(blinker_right_on)
    {
        int nextState = (blinker_right_high == 0) ? 1 : 0;
        gpio_set_level(BLINKER_RIGHT, nextState);
        gpio_set_level(BLINKER_BUZZER, nextState);
        blinker_right_high = nextState;
    }
}

void blinkers_gpio_init(void)
{
    gpio_config_t blinkers_io = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BLINKERS | (1ULL<<BLINKER_BUZZER),
    };
    gpio_config(&blinkers_io);

    const esp_timer_create_args_t toggle_args = 
    {
        .callback = &blinker_cb,
    };
    ESP_ERROR_CHECK(esp_timer_create(&toggle_args, &led_timer));
}

void blink_start(uint64_t us, uint8_t num)
{
    gpio_set_level(BLINKER_LEFT, 0);
    gpio_set_level(BLINKER_RIGHT, 0);
    gpio_set_level(BLINKER_BUZZER, 0);
    blinker_right_on = false;
    blinker_left_on = false;
    switch(num)
    {
        case 1:
            blinker_right_on = false;
            blinker_left_on = true;
            break;
        case 2:
            blinker_left_on = false;
            blinker_right_on = true;
            break;
        default:
            break;
    }
    esp_timer_stop(led_timer);
    ESP_ERROR_CHECK(esp_timer_start_periodic(led_timer, us));
}

void blink_stop(void)
{
    blinker_left_on = false;
    blinker_right_on = false;
    esp_timer_stop(led_timer);
    gpio_set_level(BLINKER_LEFT, 0);
    gpio_set_level(BLINKER_RIGHT, 0);
    gpio_set_level(BLINKER_BUZZER, 0);
}