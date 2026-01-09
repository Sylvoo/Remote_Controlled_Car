#include "motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_now_receiver.h"
#include "blinkers.h"


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