#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <math.h>
#include <stdio.h>

#include <esp_err.h>
#include <esp_lvgl_port.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_system.h>

#include "styles.h"
#include "screen_boot.h"
#include "screen_wifi.h"
#include "screen_clock.h"
#include "utiles.h"


void screen_manager_init(lv_obj_t *screen);
void ui_load_screen(lv_obj_t *screen);
