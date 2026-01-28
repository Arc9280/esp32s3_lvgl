idf_component_register(SRC_DIRS "mpu6050" "rgb" "myiic" "wifi_station"
                    INCLUDE_DIRS "mpu6050" "rgb" "myiic" "wifi_station"
                    REQUIRES driver esp_lcd esp_wifi nvs_flash)