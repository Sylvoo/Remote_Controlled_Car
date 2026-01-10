
#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>


#define PWM_FREQ   20000      
#define PWM_RES    LEDC_TIMER_8_BIT

#define PWM_CH_A   LEDC_CHANNEL_0
#define PWM_CH_B   LEDC_CHANNEL_1

#define PWM_GPIO_A 14   // ENA
#define PWM_GPIO_B 32   // ENB

static const char *TAG4 = "ESP_NOW_RECEIVER";

void motor_pwm_init(void);

#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33

void motor_gpio_init(void);

void motor_forward(void);

void motor_backward(void);

void motor_left(void);

void motor_right(void);

void motor_stop(void);

void motor_set_speed(uint8_t speed); // 0–255

extern volatile uint8_t speed;
bool is_beetween(int curr, int num, int num2);

uint8_t calcSpeed(int speed);

extern volatile uint8_t honk_on; 

void honk_action(void);

void gpio_honk_init();

void drive_actions(void);

#endif // MOTOR_H