#ifndef _LV_PORT_H_
#define _LV_PORT_H_

#include "rgb_init.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lv_demos.h"
#include "esp_err.h"
#include "esp_check.h"

#define EXAMPLE_LCD_LVGL_DIRECT_MODE            (1)
#define EXAMPLE_LCD_LVGL_AVOID_TEAR             (1)
#define EXAMPLE_LCD_RGB_BOUNCE_BUFFER_MODE      (1)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE            (0)
#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT            (100)


esp_err_t app_lvgl_init(void);

#endif