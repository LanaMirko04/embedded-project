#pragma once

#include <esp_err.h>
#include <esp_lvgl_port.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_system.h>
#include <math.h>
#include <time.h>

#include "styles.h"
#include "utiles.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_load_screen_clock(lv_obj_t *screen);
void screen_clock_destroy(void);

#ifdef __cplusplus
}
#endif
