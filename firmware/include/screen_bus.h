#pragma once

#include <esp_err.h>
#include <esp_lvgl_port.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_system.h>

#include "styles.h"
#include "utiles.h"

void ui_load_screen_bus(lv_obj_t *screen);
void screen_bus_destroy_timer(void);
