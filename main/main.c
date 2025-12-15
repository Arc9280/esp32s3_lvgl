#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
//#include "driver/i2c.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/i2c_master.h"

static const char *TAG = "RGB_RAW";

//==============================================================================
// 1. 硬件引脚配置
//==============================================================================

// I2C 配置 (必须保留，用于背光控制)
#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TIMEOUT_MS       1000

// 屏幕分辨率 (如果没有LVGL，需要手动指定)
#define EXAMPLE_LCD_H_RES               800
#define EXAMPLE_LCD_V_RES               480

// RGB 屏参数
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ      (16 * 1000 * 1000)
#define EXAMPLE_LCD_BIT_PER_PIXEL       (16)
#define EXAMPLE_RGB_DATA_WIDTH          (16)
#define EXAMPLE_RGB_BOUNCE_BUFFER_SIZE  (EXAMPLE_LCD_H_RES * 10) // 10 lines buffer

// RGB 引脚定义
#define EXAMPLE_LCD_IO_RGB_DISP         (-1)
#define EXAMPLE_LCD_IO_RGB_VSYNC        (GPIO_NUM_3)
#define EXAMPLE_LCD_IO_RGB_HSYNC        (GPIO_NUM_46)
#define EXAMPLE_LCD_IO_RGB_DE           (GPIO_NUM_5)
#define EXAMPLE_LCD_IO_RGB_PCLK         (GPIO_NUM_7)
#define EXAMPLE_LCD_IO_RGB_DATA0        (GPIO_NUM_14)
#define EXAMPLE_LCD_IO_RGB_DATA1        (GPIO_NUM_38)
#define EXAMPLE_LCD_IO_RGB_DATA2        (GPIO_NUM_18)
#define EXAMPLE_LCD_IO_RGB_DATA3        (GPIO_NUM_17)
#define EXAMPLE_LCD_IO_RGB_DATA4        (GPIO_NUM_10)
#define EXAMPLE_LCD_IO_RGB_DATA5        (GPIO_NUM_39)
#define EXAMPLE_LCD_IO_RGB_DATA6        (GPIO_NUM_0)
#define EXAMPLE_LCD_IO_RGB_DATA7        (GPIO_NUM_45)
#define EXAMPLE_LCD_IO_RGB_DATA8        (GPIO_NUM_48)
#define EXAMPLE_LCD_IO_RGB_DATA9        (GPIO_NUM_47)
#define EXAMPLE_LCD_IO_RGB_DATA10       (GPIO_NUM_21)
#define EXAMPLE_LCD_IO_RGB_DATA11       (GPIO_NUM_1)
#define EXAMPLE_LCD_IO_RGB_DATA12       (GPIO_NUM_2)
#define EXAMPLE_LCD_IO_RGB_DATA13       (GPIO_NUM_42)
#define EXAMPLE_LCD_IO_RGB_DATA14       (GPIO_NUM_41)
#define EXAMPLE_LCD_IO_RGB_DATA15       (GPIO_NUM_40)

// IO扩展器相关引脚 (用于触摸复位和背光控制)
#define GPIO_INPUT_IO_4     4
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_4)

//==============================================================================
// 2. I2C 和 辅助功能实现
//==============================================================================

/**
 * @brief I2C 主机初始化 (控制背光必须)
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0,
    };
    return i2c_param_config(i2c_master_port, &i2c_conf) ||
           i2c_driver_install(i2c_master_port, i2c_conf.mode, 0, 0, 0);
}



/**
 * @brief 配置 IO 扩展芯片 (CH422G)
 * 这一步非常重要，不配置这个，背光无法打开
 */
void io_expander_init(void)
{
    // 配置 GPIO4 (Touch Reset usually)
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    // 配置 CH422G 芯片
    // 0x24 Reg: 设置为输出模式
    uint8_t write_buf = 0x01;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    // 复位时序 (虽然不用触摸，但建议保持芯片处于已知状态)
    write_buf = 0x2C;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    esp_rom_delay_us(100 * 1000);
    gpio_set_level(GPIO_INPUT_IO_4, 0);
    esp_rom_delay_us(100 * 1000);
    write_buf = 0x2E; // 恢复
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    esp_rom_delay_us(200 * 1000);
}

/**
 * @brief 打开背光
 */
esp_err_t wavesahre_rgb_lcd_bl_on()
{
    // 确保 CH422G 处于输出模式
    uint8_t write_buf = 0x01;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    // 拉高背光引脚 (0x1E command to IO expander at 0x38)
    write_buf = 0x1E;
    return i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief 关闭背光
 */
esp_err_t wavesahre_rgb_lcd_bl_off()
{
    uint8_t write_buf = 0x01;
    i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    
    // 拉低背光引脚
    write_buf = 0x1A;
    return i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}


//==============================================================================
// 3. RGB LCD 初始化核心逻辑
//==============================================================================

/**
 * @brief 初始化 RGB 屏幕
 * @return esp_lcd_panel_handle_t 句柄，用于后续画图操作
 */
esp_lcd_panel_handle_t waveshare_esp32_s3_rgb_lcd_init()
{
    ESP_LOGI(TAG, "Initializing I2C Bus...");
    ESP_ERROR_CHECK(i2c_master_init());

    ESP_LOGI(TAG, "Initializing IO Expander (for Backlight)...");
    io_expander_init();

    ESP_LOGI(TAG, "Install RGB LCD panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;

    esp_lcd_rgb_panel_config_t panel_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings =  {
            .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
            .h_res = EXAMPLE_LCD_H_RES,
            .v_res = EXAMPLE_LCD_V_RES,
            // 以下时序参数可能需要根据具体数据手册微调，这里使用通用值
            .hsync_pulse_width = 4,
            .hsync_back_porch = 8,
            .hsync_front_porch = 8,
            .vsync_pulse_width = 4,
            .vsync_back_porch = 8,
            .vsync_front_porch = 8,
            .flags = {
                .pclk_active_neg = 1,
            },
        },
        .data_width = EXAMPLE_RGB_DATA_WIDTH,
        .bits_per_pixel = 16, // RGB565
        .num_fbs = 2,         // 不使用LVGL时，通常分配1个全屏FrameBuffer即可，或者使用2个做双缓冲
        .bounce_buffer_size_px = EXAMPLE_RGB_BOUNCE_BUFFER_SIZE,
        .sram_trans_align = 4,
        .psram_trans_align = 64,
        .hsync_gpio_num = EXAMPLE_LCD_IO_RGB_HSYNC,
        .vsync_gpio_num = EXAMPLE_LCD_IO_RGB_VSYNC,
        .de_gpio_num = EXAMPLE_LCD_IO_RGB_DE,
        .pclk_gpio_num = EXAMPLE_LCD_IO_RGB_PCLK,
        .disp_gpio_num = EXAMPLE_LCD_IO_RGB_DISP,
        .data_gpio_nums = {
            EXAMPLE_LCD_IO_RGB_DATA0, EXAMPLE_LCD_IO_RGB_DATA1, EXAMPLE_LCD_IO_RGB_DATA2, EXAMPLE_LCD_IO_RGB_DATA3,
            EXAMPLE_LCD_IO_RGB_DATA4, EXAMPLE_LCD_IO_RGB_DATA5, EXAMPLE_LCD_IO_RGB_DATA6, EXAMPLE_LCD_IO_RGB_DATA7,
            EXAMPLE_LCD_IO_RGB_DATA8, EXAMPLE_LCD_IO_RGB_DATA9, EXAMPLE_LCD_IO_RGB_DATA10, EXAMPLE_LCD_IO_RGB_DATA11,
            EXAMPLE_LCD_IO_RGB_DATA12, EXAMPLE_LCD_IO_RGB_DATA13, EXAMPLE_LCD_IO_RGB_DATA14, EXAMPLE_LCD_IO_RGB_DATA15,
        },
        .flags = {
            .fb_in_psram = 1, // 必须使用 PSRAM，因为FrameBuffer很大
        },
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));
    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    return panel_handle;
}

//==============================================================================
// 4. 主程序示例
//==============================================================================

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
        for (int i = 0; i < EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES; i++) {
            buffer[i] = 0xFFFF; 
        }
    } else {
        ESP_LOGE(TAG, "Failed to get frame buffer");
    }

    // 3. 打开背光 (必须在填充数据后打开，否则可能看到花屏)
    ESP_LOGI(TAG, "Turning on Backlight...");
    wavesahre_rgb_lcd_bl_on();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "LCD is ON (Raw Mode)");
    }
}