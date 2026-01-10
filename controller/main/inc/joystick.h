#ifndef JOYSTICK_H
#define JOYSTICK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "esp_err.h"
#include "hal/adc_types.h"   // adc_unit_t, adc_atten_t, adc_channel_t

/* Kanały ADC dla ESP32:
   GPIO34 -> ADC1_CH6, GPIO35 -> ADC1_CH7 */
#define JOY_X_CH   ADC_CHANNEL_6
#define JOY_Y_CH   ADC_CHANNEL_7

typedef struct {
    int x;
    int y;
    esp_err_t err_x;
    esp_err_t err_y;
} joystick_adc_t;

extern uint8_t x_ValueNormalized;
extern uint8_t y_ValueNormalized;
extern volatile int joystickButtonState;

void joystickButton_cb(void* arg);

void joystickButton_init(void);

void adc_joystick_init(void);

void adc_deinit(void);

joystick_adc_t joystick_read(void);

#ifdef __cplusplus
}
#endif

#endif /* JOYSTICK_H */
