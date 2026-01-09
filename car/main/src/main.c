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
#include "esp_now_receiver.h"

#include "esp_now.h"
#include "esp_wifi_types.h"  

#include "inttypes.h"
#include "math.h"
#include "esp_err.h"
#include "driver/ledc.h"

// static const char *TAG = "Wifi station";
static const char *TAG4 = "ESP_NOW_RECEIVER";

const uint8_t PEER_MAC[ESP_NOW_ETH_ALEN] = { 0x44, 0x1D, 0x64, 0xF8, 0xFE, 0x5C }; // 44:1D:64:F8:FE:5C




#define PWM_FREQ   20000      
#define PWM_RES    LEDC_TIMER_8_BIT

#define PWM_CH_A   LEDC_CHANNEL_0
#define PWM_CH_B   LEDC_CHANNEL_1

#define PWM_GPIO_A 14   // ENA
#define PWM_GPIO_B 32   // ENB

#define TOGGLE_PERIOD 250000

void motor_pwm_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = PWM_FREQ,
        .duty_resolution  = PWM_RES,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch_a = {
        .gpio_num   = PWM_GPIO_A,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = PWM_CH_A,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0
    };
    ledc_channel_config(&ch_a);

    ledc_channel_config_t ch_b = ch_a;
    ch_b.gpio_num = PWM_GPIO_B;
    ch_b.channel  = PWM_CH_B;
    ledc_channel_config(&ch_b);
}

#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33

#define BLINKER_LEFT    GPIO_NUM_13
#define BLINKER_RIGHT   GPIO_NUM_12
#define BLINKERS        ((1ULL<<BLINKER_LEFT) | (1ULL<<BLINKER_RIGHT))



esp_timer_handle_t led_timer;
volatile uint8_t blinker_left_on = 0;
volatile uint8_t blinker_right_on = 0;
volatile uint8_t blinker_left_high = 0;
volatile uint8_t blinker_right_high = 0;

// void gpio_toggle(gpio_num_t pinNumber)
// {
//     // int level = gpio_get_level(pinNumber);
    
// }

void blinker_cb(void* arg)
{
    if(blinker_left_on)
    {
        int nextState= (blinker_left_high == 0) ? 1 : 0;
        gpio_set_level(BLINKER_LEFT, nextState);
        blinker_left_high = nextState;
    }
    if(blinker_right_on)
    {
        int nextState = (blinker_right_high == 0) ? 1 : 0;
        gpio_set_level(BLINKER_RIGHT, nextState);
        blinker_right_high = nextState;
    }
}

void motor_gpio_init(void)
{
    gpio_config_t io = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask =
            (1ULL<<IN1) | (1ULL<<IN2) |
            (1ULL<<IN3) | (1ULL<<IN4)
    };
    gpio_config(&io);
}

void blinkers_gpio_init(void)
{
    gpio_config_t blinkers_io = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BLINKERS
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
}

void motor_forward(void)
{
    gpio_set_level(IN1, 1);
    gpio_set_level(IN2, 0);

    gpio_set_level(IN3, 1);
    gpio_set_level(IN4, 0);
    ESP_LOGI(TAG4, "CAR FORWARD");
    }

void motor_backward(void)
{
    gpio_set_level(IN1, 0);
    gpio_set_level(IN2, 1);

    gpio_set_level(IN3, 0);
    gpio_set_level(IN4, 1);
    ESP_LOGI(TAG4, "CAR BACKWARD");
}
void motor_left(void)
{
    gpio_set_level(IN1, 0);
    gpio_set_level(IN2, 1);

    gpio_set_level(IN3, 1);
    gpio_set_level(IN4, 0);
    ESP_LOGI(TAG4, "CAR LEFT");
}

void motor_right(void)
{
    gpio_set_level(IN1, 1);
    gpio_set_level(IN2, 0);

    gpio_set_level(IN3, 0);
    gpio_set_level(IN4, 1);
    ESP_LOGI(TAG4, "CAR RIGHT");
}

void motor_stop(void)
{
    gpio_set_level(IN1, 0);
    gpio_set_level(IN2, 0);
    gpio_set_level(IN3, 0);
    gpio_set_level(IN4, 0);
    ESP_LOGI(TAG4, "CAR STOP");
}

void motor_set_speed(uint8_t speed) // 0–255
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CH_A, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CH_A);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CH_B, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CH_B);
}
// typedef struct __attribute__((packed)){
//     int16_t x_v;   
//     int16_t y_v;   
//     uint8_t btn;   
//     uint32_t seq;   
// } espnow_payload_t;


static volatile uint8_t speed = 0;
bool is_beetween(int curr, int num, int num2)
{
    return (curr >= num && curr <= num2) ? true : false;
}

uint8_t calcSpeed(int speed)
{
    if(is_beetween(speed, 170, 200)) return 0;
    else return 255;
}

void drive_actions(void)
{

    static volatile uint8_t action = 0;
    static volatile uint8_t prev_action = 255;
    espnow_payload_t current;

    if (espnow_receiver_get_latest(&current)) {
        
        if(is_beetween(current.y_v, 0, 120) && is_beetween(current.x_v, 120, 190))
        {
            action = 1;
            motor_set_speed(200);
        }
        if(is_beetween(current.y_v, 240, 255) && is_beetween(current.x_v, 180, 215))
        {
            action = 2;
            motor_set_speed(200);
        }
        if(is_beetween(current.x_v, 0, 120) && is_beetween(current.y_v, 160, 190))
        {
            action = 4;
            motor_set_speed(180);
        }
        if(is_beetween(current.x_v, 240, 255) && is_beetween(current.y_v, 170, 215))
        {
            action = 3;
            motor_set_speed(180);
        }
        if(is_beetween(current.y_v, 170, 200) && is_beetween(current.x_v, 170, 200))
        {
            action = 0;
            motor_set_speed(0);
        } 
    }
    
    if(action != prev_action)
    {
        switch(action)
        {
            case 0:
                motor_stop();
                blink_stop();
            break;
            case 1:
                motor_forward();
                blink_stop();
            break;
            case 2:
                motor_backward();
                blink_stop();
            break;
            case 3:
                motor_left();
                blink_start(TOGGLE_PERIOD, 1);
            break;
            case 4:
                motor_right();
                blink_start(TOGGLE_PERIOD, 2);
            break;
            default:
                ESP_LOGE(TAG4, "Something went wrong with, switch{case} actions");
            break;
        }
        prev_action = action;
    }
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
