#ifndef _WIFI_INIT_H_
#define _WIFI_INIT_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID      "TP_LINK_B304"
#define EXAMPLE_ESP_WIFI_PASS      "LCUB304HAG"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

void wifi_init_sta(void);

#endif