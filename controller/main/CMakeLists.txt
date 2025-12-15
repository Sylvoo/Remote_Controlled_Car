idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES esp_wifi nvs_flash esp_http_client esp_driver_gpio esp_timer
                    INCLUDE_DIRS ".")
