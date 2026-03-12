/*!
 * \file            main.cpp
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           CYD main program.
 */

#include <assert.h>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_lvgl_port.h>
#include <esp_system.h>
#include <lvgl.h>
#include <cstring>

#include "fsm.h"
#include "net.h"
#include "result.h"
#include "config.h"

extern "C" {
#include "lcd.h"
#include "touch.h"
#include "styles.h"
#include "screen_manager.h"
}

static constexpr char TAG[] = "MAIN";

static Result dummy_action(void);
static Result setup_fsm(void);

// extern "C" void app_main(void) {
//     assert(setup_fsm() == Result::SUCCESS);

//     while (true) { // main loop
//     }
// }

static esp_err_t app_lvgl_main(void) {
    lv_obj_t *scr = lv_scr_act();

    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(scr, lv_color_make(0xF1, 0xEC, 0xE6), LV_STATE_DEFAULT);
    styles_init();

    screen_manager_init(scr);

    lvgl_port_unlock();

    return ESP_OK;
}

extern "C" void app_main(void) {
    Config &cfg = Config::get_instance();
    cfg.set_ssid("niggatron", std::strlen("niggatron"));
    cfg.set_password("parmiagiana", std::strlen("parmigiana"));

    // NetHandler::get_instance().init_connection();

    extern Screens present_screen_type;
    extern Screens next_screen_type;

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

static Result dummy_action(void) {
    ESP_LOGW(TAG, "Running %s", __func__);
    return Result::SUCCESS;
}

static Result setup_fsm(void) {
    ESP_LOGW(TAG, "Running %s", __func__);
    Fsm &fsm = Fsm::get_instance();

    Result res = fsm.register_action(Fsm::State::INIT, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting INIT action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::WIFI_CONNECTION, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting WIFI_CONNECTION action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::FETCH_CONFIG, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting FETCH_CONFIG action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::UPDATE_VIEW, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting UPDATE_VIEW action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::ERROR, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting ERROR action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    return Result::SUCCESS;
}
