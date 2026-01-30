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
#include "mpu6050.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void my_timer_callback(lv_timer_t * timer);
/**********************
 *  STATIC VARIABLES
 **********************/
static float pitch = 0.0f, roll = 0.0f, yaw = 0.0f;
static char buf[32];
/**
 * Create a demo application
 */

void custom_init(lv_ui *ui)
{
    lv_timer_t *my_timer = lv_timer_create(my_timer_callback, 100, ui);
}

static void my_timer_callback(lv_timer_t * timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    
    snprintf(buf, sizeof(buf), "%.1f°", pitch);
    lv_label_set_text(ui->screen_pitch_data, buf);
    
    snprintf(buf, sizeof(buf), "%.1f°", roll);
    lv_label_set_text(ui->screen_roll_data, buf);
    
    snprintf(buf, sizeof(buf), "%.1f°", yaw);
    lv_label_set_text(ui->screen_yaw_data, buf);
}