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
#include <nvs.h>

#include "api_client.h"
#include "api_task.h"
#include "config.h"
#include "fsm.h"
#include "net.h"
#include "nvs_flash.h"
#include "result.h"

#include "lcd.h"
#include "touch.h"
#include "styles.h"
#include "screen_manager.h"
#include "utiles.h"

static constexpr char MAIN[] = "MAIN";

static bool lvgl_initialized = false;
static bool s_first_boot     = false;

static esp_lcd_panel_io_handle_t s_lcd_io;
static esp_lcd_panel_handle_t s_lcd_panel;
static esp_lcd_touch_handle_t s_tp;
static lv_display_t *s_lvgl_display = NULL;

static esp_err_t app_lvgl_main(void) {
    lv_obj_t *scr = lv_scr_act();

    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(scr, lv_color_make(0xF1, 0xEC, 0xE6), LV_STATE_DEFAULT);
    styles_init();
    screen_manager_init(scr);
    lvgl_port_unlock();

    return ESP_OK;
}

static Result init_action(void) {
    ESP_LOGD(MAIN, "Running %s", __func__);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(MAIN, "NVS needs erase (%s); erasing", esp_err_to_name(err));
        esp_err_t e2 = nvs_flash_erase();
        if (e2 != ESP_OK) {
            ESP_LOGE(MAIN, "NVS erase failed: %s", esp_err_to_name(e2));
            return Result::UNKNOWN_ERROR;
        }
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "NVS init failed: %s", esp_err_to_name(err));
        return Result::UNKNOWN_ERROR;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "Event loop create failed: %s", esp_err_to_name(err));
        nvs_flash_deinit();
        return Result::UNKNOWN_ERROR;
    }

    err = lcd_display_brightness_init();
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "Brightness init failed: %s", esp_err_to_name(err));
        return Result::UNKNOWN_ERROR;
    }

    err = app_lcd_init(&s_lcd_io, &s_lcd_panel);
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "LCD init failed: %s", esp_err_to_name(err));
        return Result::UNKNOWN_ERROR;
    }

    s_lvgl_display = app_lvgl_init(s_lcd_io, s_lcd_panel);
    if (s_lvgl_display == NULL) {
        ESP_LOGE(MAIN, "LVGL init failed");
        return Result::UNKNOWN_ERROR;
    }

    err = touch_init(&s_tp);
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "Touch init failed: %s", esp_err_to_name(err));
        return Result::UNKNOWN_ERROR;
    }

    lvgl_port_touch_cfg_t touch_cfg = {};
    touch_cfg.disp = s_lvgl_display;
    touch_cfg.handle = s_tp;
    lvgl_port_add_touch(&touch_cfg);

    err = lcd_display_brightness_set(75);
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "Brightness set failed: %s", esp_err_to_name(err));
        return Result::UNKNOWN_ERROR;
    }

    err = lcd_display_rotate(s_lvgl_display, LV_DISPLAY_ROTATION_180);
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "Display rotate failed: %s", esp_err_to_name(err));
        return Result::UNKNOWN_ERROR;
    }

    present_screen_type = SCREEN_BOOT;
    next_screen_type = SCREEN_BOOT;

    err = app_lvgl_main();
    if (err != ESP_OK) {
        ESP_LOGE(MAIN, "LVGL main init failed");
        return Result::UNKNOWN_ERROR;
    }

    lvgl_initialized = true;
    return Result::SUCCESS;
}

static Result wifi_connection_action(void) {
    ESP_LOGD(MAIN, "Running %s", __func__);

    NetHandler &net = NetHandler::get_instance();
    net.init_connection();

    while (!net.connected) {
        if (net.smartconfig_running) {
            next_screen_type = SCREEN_WIFI;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    next_screen_type = SCREEN_BOOT;

    Result res = net.sync_time();
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "Time sync failed: %s - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    return Result::SUCCESS;
}

static Result fetch_config_action(void) {
    ESP_LOGD(MAIN, "Running %s", __func__);

    Config    &cfg = Config::get_instance();
    ApiClient &api = ApiClient::get_instance();

    if (cfg.get_device_token()[0] == '\0') {
        ESP_LOGI(MAIN, "No device token — registering device");
        Result r = api.register_device();
        if (r != Result::SUCCESS) {
            result_set_err_msg("Device registration failed: %s", result_to_str(r));
            return r;
        }
        ESP_LOGI(MAIN, "Registered: token=%s", cfg.get_device_token());
        s_first_boot = true;
    } else {
        ESP_LOGI(MAIN, "Device token present: %s", cfg.get_device_token());
    }

    if (!s_first_boot) {
        api_task_start();
    }
    return Result::SUCCESS;
}

static Result update_view_action(void) {
    ESP_LOGD(MAIN, "Running %s", __func__);
    next_screen_type = s_first_boot ? SCREEN_PAIRING : SCREEN_CLOCK;
    return Result::SUCCESS;
}

static Result error_action(void) {
    ESP_LOGE(MAIN, "FSM ERROR state: %s", result_get_err_msg());
    if (lvgl_initialized) {
        next_screen_type = SCREEN_WIFI;
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
    esp_restart();
    return Result::SUCCESS;
}

static Result setup_fsm(void) {
    ESP_LOGD(MAIN, "Running %s", __func__);
    Fsm &fsm = Fsm::get_instance();
    Result res;

    res = fsm.register_action(Fsm::State::INIT, init_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "Failed to register INIT action: %s - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::WIFI_CONNECTION, wifi_connection_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "Failed to register WIFI_CONNECTION action: %s - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::FETCH_CONFIG, fetch_config_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "Failed to register FETCH_CONFIG action: %s - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::UPDATE_VIEW, update_view_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "Failed to register UPDATE_VIEW action: %s - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::ERROR, error_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "Failed to register ERROR action: %s - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    return Result::SUCCESS;
}

extern "C" void app_main(void) {
    assert(setup_fsm() == Result::SUCCESS);

    Fsm &fsm = Fsm::get_instance();
    Result res;

    res = fsm.transition_to(Fsm::State::INIT);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "INIT failed: %s - %s", result_to_str(res), result_get_err_msg());
        esp_restart();
    }

    res = fsm.transition_to(Fsm::State::WIFI_CONNECTION);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "WIFI_CONNECTION failed: %s - %s", result_to_str(res), result_get_err_msg());
        fsm.transition_to(Fsm::State::ERROR);
    }

    res = fsm.transition_to(Fsm::State::FETCH_CONFIG);
    if (res != Result::SUCCESS) {
        ESP_LOGE(MAIN, "FETCH_CONFIG failed: %s - %s", result_to_str(res), result_get_err_msg());
        fsm.transition_to(Fsm::State::ERROR);
    }

    fsm.transition_to(Fsm::State::UPDATE_VIEW);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
