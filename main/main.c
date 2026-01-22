#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rgb_init.h"
#include "lv_port.h"
#include "lv_demos.h"

//static const char *TAG = "RGB_RAW";

void app_main(void)
{
    ESP_ERROR_CHECK(app_lvgl_init());

    lvgl_port_lock(0);
    lv_demo_widgets();
    lvgl_port_unlock();
}