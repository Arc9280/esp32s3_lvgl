#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_port.h"
#include "mpu6050.h"
#include "iic_init.h"
#include "wifi_init.h"
#include "sntp.h"
#include "nvs_flash.h"
#include "gui_guider.h"
#include "custom.h"

// static const char *TAG = "RGB_RAW";
lv_ui guider_ui;
static char buf_pitch[16];
static char buf_roll[16];
static char buf_yaw[16];

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //初始化默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    i2c_bus_init();
    ESP_ERROR_CHECK(MPU6050_Init());
    DMP_Init();
    ESP_ERROR_CHECK(app_lvgl_init());
    lvgl_port_lock(0);
    setup_ui(&guider_ui);
    //custom_init(&guider_ui);
    lvgl_port_unlock();

    time_init();
    wifi_init_sta();

    float pitch, roll, yaw;

    while (1)
    {
        MPU6050_DMP_Get_Data(&pitch, &roll, &yaw);
        printf("Pitch: %.2f, Roll: %.2f, Yaw: %.2f\n", pitch, roll, yaw);
        snprintf(buf_pitch, sizeof(buf_pitch), "%.1f°", pitch);
        snprintf(buf_roll, sizeof(buf_roll), "%.1f°", roll);
        snprintf(buf_yaw, sizeof(buf_yaw), "%.1f°", yaw);
        lvgl_port_lock(0);
        lv_label_set_text(guider_ui.screen_pitch_data, buf_pitch);
        lv_label_set_text(guider_ui.screen_roll_data, buf_roll);
        lv_label_set_text(guider_ui.screen_yaw_data, buf_yaw);
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}