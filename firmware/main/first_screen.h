#pragma once

#include <esp_err.h>
#include <lvgl.h>
#include <esp_lvgl_port.h>

void draw_first_screen(lv_obj_t *screen);
void draw_wifi_screen(lv_obj_t *screen);
void draw_clock_screen(lv_obj_t *screen);
void styles_init();
