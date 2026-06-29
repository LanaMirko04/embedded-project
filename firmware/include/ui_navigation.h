#pragma once

#include <esp_err.h>
#include <esp_lvgl_port.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_system.h>

#include "styles.h"
#include "utiles.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_load_arrows_btn(lv_obj_t *screen);

#ifdef __cplusplus
}
#endif