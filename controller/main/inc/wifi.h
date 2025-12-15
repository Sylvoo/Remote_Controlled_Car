#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* zmienne globalne używane w main.c */
extern EventGroupHandle_t s_wifi_event_group;
extern int is_connected;

/* funkcje */
void wifi_init_sta(void);
void send_to_ThingSpeak(int tempValue, int pressureValue);

#endif
