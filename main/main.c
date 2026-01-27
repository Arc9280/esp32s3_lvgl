#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rgb_init.h"
#include "lv_port.h"
#include "lv_demos.h"
#include "mpu6050.h"
#include "iic_init.h"


//static const char *TAG = "RGB_RAW";

void app_main(void)
{
    i2c_bus_init();
    ESP_ERROR_CHECK(MPU6050_Init());
    ESP_ERROR_CHECK(app_lvgl_init());
    DMP_Init();

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