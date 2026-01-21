#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/i2c_master.h"
#include "rgb_init.h"


static const char *TAG = "RGB_RAW";

void app_main(void)
{
    // 1. 初始化屏幕驱动
    esp_lcd_panel_handle_t panel_handle = waveshare_esp32_s3_rgb_lcd_init();

    // 2. 获取 FrameBuffer 指针
    // 如果没有LVGL，你需要手动向这块内存写入颜色数据来显示内容
    uint16_t *buffer = NULL;
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, (void **)&buffer, NULL)); // 2 表示获取当前 buffer

    if (buffer) {
        ESP_LOGI(TAG, "Filling screen with RED color...");
        // 将屏幕填充满红色 (RGB565格式，红色=0xF800)
        // 分辨率 H * V
        for (int i = 0; i < 800 * 480; i++) {
            buffer[i] = 0xF800;
        }
    } else {
        ESP_LOGE(TAG, "Failed to get frame buffer");
    }

    // 3. 打开背光 (必须在填充数据后打开，否则可能看到花屏)
    ESP_LOGI(TAG, "Turning on Backlight...");
    wavesahre_rgb_lcd_bl_on();
    vTaskDelay(pdMS_TO_TICKS(1000));


    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "LCD is ON (Raw Mode)");
    }
}