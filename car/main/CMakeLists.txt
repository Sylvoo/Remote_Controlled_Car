file(GLOB SRC_FILES "src/*.c")

idf_component_register(
                    SRCS ${SRC_FILES}
                    INCLUDE_DIRS "." "inc"
                    PRIV_REQUIRES esp_wifi nvs_flash esp_http_client esp_driver_gpio esp_timer esp_adc nvs_flash esp_driver_ledc mqtt)
                    
