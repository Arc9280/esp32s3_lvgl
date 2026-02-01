/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/


/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "misc/lv_timer.h"
#include "mpu6050.h"
#include "time.h"
#include "sys/time.h"
#include "wifi_init.h"  // 获取事件定义
#include "lv_port.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static float pitch = 0.0f, roll = 0.0f, yaw = 0.0f;
static char buf[32];
extern lv_ui guider_ui;
/**
 * Create a demo application
 */

void mpu6050_data_update(lv_ui *ui)
{
    MPU6050_DMP_Get_Data(&pitch, &roll, &yaw);
    
    snprintf(buf, sizeof(buf), "%.1f°", pitch);
    lv_label_set_text(ui->screen_pitch_data, buf);
    
    snprintf(buf, sizeof(buf), "%.1f°", roll);
    lv_label_set_text(ui->screen_roll_data, buf);
    
    snprintf(buf, sizeof(buf), "%.1f°", yaw);
    lv_label_set_text(ui->screen_yaw_data, buf);
}

static void mpu6050_update_timer_callback(lv_timer_t * timer)
{
    mpu6050_data_update(&guider_ui);
}

// 更新时间显示函数
void update_time_display(void)
{
    time_t now;
    struct tm timeinfo;
    char time_str[32];
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    if (timeinfo.tm_year >= (2016 - 1900)) {
        // 格式化时间
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", 
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    lvgl_port_lock(0);
    lv_label_set_text(guider_ui.screen_time, time_str);
    lvgl_port_unlock();
    }
}

// SNTP同步事件处理函数
static void sntp_sync_event_handler(void* arg, esp_event_base_t event_base,
                                   int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENTS && event_id == SNTP_SYNC_COMPLETED) {
        ESP_LOGI("CUSTOM", "SNTP sync completed, updating time display");
        update_time_display();
    }
}

static void time_update_timer_callback(lv_timer_t *timer)
{
    update_time_display();
}

void custom_init(lv_ui *ui)
{
    lv_timer_t *my_timer = lv_timer_create(mpu6050_update_timer_callback, 100, ui);
    

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENTS,
        SNTP_SYNC_COMPLETED,
        &sntp_sync_event_handler,
        NULL,
        NULL));
    
    lv_timer_t *time_timer = lv_timer_create(time_update_timer_callback, 1000, NULL);
}