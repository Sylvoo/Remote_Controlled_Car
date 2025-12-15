/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_http_client.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/gpio.h"
#include "esp_rom_sys.h" 
#include "esp_timer.h"
#include <stdio.h>
#include "freertos/portmacro.h"


/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define ESP_WIFI_SSID      "iPhone"
#define ESP_WIFI_PASS      "123456789"
#define ESP_MAXIMUM_RETRY  5
#define THINGSPEAK_APIKEY  "PYOQZ55HNNW1TX6Q"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";
static const char *TAG2 = "DHT11";

static portMUX_TYPE dht_mux = {0};



static int s_retry_num = 0;

int is_connected = 0;
const gpio_num_t DHT_GPIO = GPIO_NUM_4;
double RH = 0;
double TEMP = 0;
int bit = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
        is_connected = 0;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        is_connected = 1; 
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        is_connected = 1;
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        is_connected = 0;
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        is_connected = 0;
    }
}

void send_to_ThingSpeak(int tempValue, int pressureValue)
{
    // to operate in http client mode
    char url[256];
    sprintf(url, "http://api.thingspeak.com/update?api_key=%s&field1=%d&field2=%d",
                THINGSPEAK_APIKEY, tempValue, pressureValue);

    esp_http_client_config_t config = {
    .url = url,
    .method = HTTP_METHOD_GET,
    };
    ESP_LOGI(TAG, "HTTP request with url =>");
    esp_http_client_handle_t client = esp_http_client_init(&config);    
    
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

static inline int64_t now_us(void)
{
    return esp_timer_get_time();
}

static int wait_for_level(gpio_num_t gpio_pin, int level, int timeout)
{
    int64_t t0 = now_us();
    while(gpio_get_level(gpio_pin) != level)
    {
        if(now_us() - t0 > timeout) return -1;
    }
    return (int)(now_us() - t0); // zwracamy czas oczekiwania na ustalenie poziomu
}

typedef struct {
    int humidity_int;
    int humidity_dec;
    int temp_int;
    int temp_dec;
} dht11_data_t;

dht11_data_t data;

esp_err_t dht11_Read(gpio_num_t gpio_pin, dht11_data_t *out)
{
    uint8_t data[5];
    memset(data, 0, sizeof(data));
    // send a start signal from ESP32
    // portENTER_CRITICAL(&dht_mux);

    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(gpio_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(gpio_pin, 1);
    esp_rom_delay_us(30);    

    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT);

    // sequence of answer from dht11
    if(wait_for_level(gpio_pin, 0, 300) < 0) return ESP_ERR_TIMEOUT;
    if(wait_for_level(gpio_pin, 1, 300) < 0) return ESP_ERR_TIMEOUT;
    if(wait_for_level(gpio_pin, 0, 300) <0 ) return ESP_ERR_TIMEOUT;

    for(int i = 0; i < 40; i++)
    {
        if(wait_for_level(gpio_pin, 1, 200) < 0) return ESP_ERR_TIMEOUT;
        int64_t high_lvl = now_us();
        if(wait_for_level(gpio_pin, 0, 200) < 0) return ESP_ERR_TIMEOUT;
        int high_lvl_time = (int)(now_us() - high_lvl);

        if(high_lvl_time > 50) bit = 1; 
        else bit = 0;

        data[i/8] <<= 1;
        data[i/8] |= bit; 
    }
    // portEXIT_CRITICAL(&dht_mux);

    uint8_t sum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if(data[4] != sum) return ESP_ERR_INVALID_CRC;

    out->humidity_int = data[0];
    out->humidity_dec = data[1];
    out->temp_int     = data[2];
    out->temp_dec     = data[3];

    return ESP_OK;
}

void set_gpio_DHT11(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << DHT_GPIO,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
    gpio_set_level(DHT_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(1100)); 
}
static void dht_task(void *args)
{
    while (1) {
        esp_err_t err = dht11_Read(DHT_GPIO, &data);
        if (err == ESP_OK) {
            RH   = data.humidity_int + ((double)data.humidity_dec / 10.0);
            TEMP = data.temp_int     + ((double)data.temp_dec     / 10.0);
            ESP_LOGI(TAG2, "RH: %d.%d TEMP: %d.%d",
                     data.humidity_int, data.humidity_dec,
                     data.temp_int, data.temp_dec);
        } else {
            ESP_LOGE(TAG2, "DHT11 ERROR: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
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
        /* If you only want to open more logs in the wifi module, you need to make the max level greater than the default level,
         * and call esp_log_level_set() before esp_wifi_init() to improve the log level of the wifi module. */
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    set_gpio_DHT11();

    while(1)
    {
        xTaskCreatePinnedToCore(dht_task, "sdas", 4096, NULL, 5, NULL, 1);
        if(is_connected == 1)
        {
          
            ESP_LOGI(TAG2, "Sending data... RH: %d.%d, TEMP: %d.%d", data.humidity_dec, data.humidity_int, data.temp_int, data.temp_dec);
            send_to_ThingSpeak(RH, TEMP);
            vTaskDelay(pdMS_TO_TICKS(15000)); // wymagane na tej stronce 
        }

    }
}
