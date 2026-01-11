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
#include "mqtt_client.h"

// static const char *TAG = "Wifi station";
// static const char *TAG4 = "ESP_NOW_RECEIVER";
static const char *TAG100 = "MQTT";
const uint8_t PEER_MAC[ESP_NOW_ETH_ALEN] = { 0x44, 0x1D, 0x64, 0xF8, 0xFE, 0x5C }; // 44:1D:64:F8:FE:5C

#define BROKER_URL "mqtt://test.mosquitto.org:1883"
esp_mqtt_client_handle_t mqtt_client = NULL;
uint8_t mqtt_connected = 0;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG100, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG100, "MQTT_CONNECTED, Active session with broker");
            mqtt_connected = 1;

            msg_id = esp_mqtt_client_publish(client, "car/actions", "dataACTIONS", 0, 1, 0);
            ESP_LOGI(TAG100, "sent publish success, msg_id=%d",  msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "car/phone", 0);
            ESP_LOGI(TAG100, "sent subscribe success, msg_id=%d", msg_id);
        break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG100, "MQTT_EVENT_DISCONNECTED");
            mqtt_connected = 0;
        break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG100, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG100, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

        case MQTT_EVENT_PUBLISHED:
         ESP_LOGI(TAG100, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG100, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            // parsujemy nasze dane, trzeba znac strukture jaka bedzie przychodzic
        break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG100, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG100, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

            }
        break;

        default:
             ESP_LOGI(TAG100, "Other event id:%d", event->event_id);
        break;
        
    }
}

void mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = 
    {
        .broker.address.uri = BROKER_URL,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void mqtt_publish_if_connected(const char* topic, const char* data)
{
    if(mqtt_connected)
    {
        esp_mqtt_client_publish(mqtt_client, topic, data, 0, 0, 0);
    }
}

static actions_t car_action;
const char* carStatusTopic = "car/actions";
static volatile char* data;

void carDataParse(actions_t action)
{
    switch(action)
    {
        case CAR_STOP:
            data = "stop";
            break;
        case CAR_FORWARD:
            data = "forward";
            break;
        case CAR_BACKWARD:
            data = "backward";
            break;
        case CAR_LEFT:
            data = "left";
            break;
        case CAR_RIGHT:
            data = "right";
            break;
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
    gpio_honk_init();
    mqtt_start();
    
    while(1)
    {
        car_action = drive_actions();
        carDataParse(car_action);
        mqtt_publish_if_connected(carStatusTopic, data);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
