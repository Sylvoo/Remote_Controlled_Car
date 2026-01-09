#ifndef BLINKERS_H
#define BLINKERS_H

#include <stdint.h>
#include "esp_timer.h"
#include <stdbool.h>

#define TOGGLE_PERIOD 250000
#define BLINKER_LEFT    GPIO_NUM_13
#define BLINKER_RIGHT   GPIO_NUM_12
#define BLINKERS        ((1ULL<<BLINKER_LEFT) | (1ULL<<BLINKER_RIGHT))

extern bool blinker_left_on;
extern bool blinker_right_on;
extern bool blinker_left_high;
extern bool blinker_right_high;

extern esp_timer_handle_t led_timer;

void blinker_cb(void* arg);


void blinkers_gpio_init(void);

void blink_start(uint64_t us, uint8_t num);
// {
//     gpio_set_level(BLINKER_LEFT, 0);
//     gpio_set_level(BLINKER_RIGHT, 0);
//     blinker_right_on = false;
//     blinker_left_on = false;
//     switch(num)
//     {
//         case 1:
//             blinker_right_on = false;
//             blinker_left_on = true;
//             break;
//         case 2:
//             blinker_left_on = false;
//             blinker_right_on = true;
//             break;
//         default:
//             break;
//     }
//     esp_timer_stop(led_timer);
//     ESP_ERROR_CHECK(esp_timer_start_periodic(led_timer, us));
// }

void blink_stop(void);
// {
//     blinker_left_on = false;
//     blinker_right_on = false;
//     esp_timer_stop(led_timer);
//     gpio_set_level(BLINKER_LEFT, 0);
//     gpio_set_level(BLINKER_RIGHT, 0);
// }

#endif // BLINKERS_H