#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rgb_init.h"
#include "lv_port.h"
#include "lv_demos.h"
#include "mpu6050.h"
#include "iic_init.h"
#include "wifi_init.h"

//static const char *TAG = "RGB_RAW";

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    i2c_bus_init();
    ESP_ERROR_CHECK(MPU6050_Init());
    ESP_ERROR_CHECK(app_lvgl_init());
    DMP_Init();
    wifi_init_sta();

    float pitch, roll, yaw;

    lvgl_port_lock(0);
    lv_demo_widgets();
    lvgl_port_unlock();

    while (1)
    {
        MPU6050_DMP_Get_Data(&pitch, &roll, &yaw);
        printf("Pitch: %.2f, Roll: %.2f, Yaw: %.2f\n", pitch, roll, yaw);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}