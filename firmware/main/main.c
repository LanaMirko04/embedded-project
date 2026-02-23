/*!
 * \file            main.h
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           CYD main program.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <math.h>
#include <stdio.h>

#include <esp_check.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_lvgl_port.h>
#include <esp_system.h>
#include <lvgl.h>

#include "lcd.h"
#include "touch.h"

#include "styles.h"
//#include "screen_boot.h"
//#include "screen_wifi.h"
//#include "screen_clock.h"
#include "screen_manager.h"

// Generated UI header (from XML)
//#include "ui_1.h"

static const char *TAG = "demo";

static esp_err_t app_lvgl_main(void) {
    lv_obj_t *scr = lv_scr_act();
    
    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(scr, lv_color_make(0xF1, 0xEC, 0xE6), LV_STATE_DEFAULT);
    styles_init();

    screen_manager_init(scr);

    lvgl_port_unlock();

    return ESP_OK;
}

//static void lv_tick_task(void *arg) {
//    (void)arg;
//    while (1) {
//        lv_tick_inc(1);
//        vTaskDelay(pdMS_TO_TICKS(1));
//    }
//}
//
//static void lvgl_task(void *arg) {
//    (void)arg;
//    while (1) {
//        lv_timer_handler();
//        vTaskDelay(pdMS_TO_TICKS(5));
//    }
//}

void app_main(void) {

    esp_lcd_panel_io_handle_t lcd_io;
    esp_lcd_panel_handle_t lcd_panel;
    esp_lcd_touch_handle_t tp;
    lvgl_port_touch_cfg_t touch_cfg = { 0 };
    lv_display_t *lvgl_display = NULL;
    char buf[16];
    uint16_t n = 0;

    ESP_ERROR_CHECK(lcd_display_brightness_init());

    ESP_ERROR_CHECK(app_lcd_init(&lcd_io, &lcd_panel));
    lvgl_display = app_lvgl_init(lcd_io, lcd_panel);
    if (lvgl_display == NULL) {
        ESP_LOGI(TAG, "fatal error in app_lvgl_init");
        esp_restart();
    }

    ESP_ERROR_CHECK(touch_init(&tp));
    touch_cfg.disp = lvgl_display;
    touch_cfg.handle = tp;
    lvgl_port_add_touch(&touch_cfg);

    ESP_ERROR_CHECK(lcd_display_brightness_set(75));
    ESP_ERROR_CHECK(lcd_display_rotate(lvgl_display, LV_DISPLAY_ROTATION_180));
    ESP_ERROR_CHECK(app_lvgl_main());
}
