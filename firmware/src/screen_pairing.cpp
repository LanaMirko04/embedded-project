#include "screen_pairing.h"
#include "api_task.h"
#include "config.h"
#include "utiles.h"

static lv_timer_t *s_poll_timer = nullptr;

static void persist_and_go(void) {
    Config::get_instance().store();
    api_task_start();
    next_screen_type = SCREEN_CLOCK;
}

static void pairing_poll_cb(lv_timer_t *) {
    BusModel tmp;
    if (api_task_copy_bus_model(&tmp) && tmp.count > 0)
        persist_and_go();
}

static void pairing_done_cb(lv_event_t *) {
    persist_and_go();
}

extern "C" {

void screen_pairing_destroy_timer(void) {
    if (s_poll_timer) {
        lv_timer_delete(s_poll_timer);
        s_poll_timer = nullptr;
    }
}

void ui_load_screen_pairing(lv_obj_t *screen) {
    screen_pairing_destroy_timer();

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Pair with companion app");
    lv_obj_add_style(title, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -65);

    lv_obj_t *hint = lv_label_create(screen);
    lv_label_set_text(hint, "Enter this token in the app:");
    lv_obj_add_style(hint, &style_par, LV_STATE_DEFAULT);
    lv_obj_set_width(hint, 280);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(hint, LV_ALIGN_CENTER, 0, -35);

    const char *token = Config::get_instance().get_device_token();
    lv_obj_t *token_lbl = lv_label_create(screen);
    lv_label_set_text(token_lbl, token);
    lv_obj_add_style(token_lbl, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_set_width(token_lbl, 300);
    lv_obj_set_style_text_align(token_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(token_lbl, LV_ALIGN_CENTER, 0, -5);

    lv_obj_t *waiting = lv_label_create(screen);
    lv_label_set_text(waiting, "Waiting for pairing...");
    lv_obj_add_style(waiting, &style_par, LV_STATE_DEFAULT);
    lv_obj_align(waiting, LV_ALIGN_CENTER, 0, 25);

    lv_obj_t *btn = lv_button_create(screen);
    lv_obj_add_style(btn, &style1_btn, LV_STATE_DEFAULT);
    lv_obj_add_style(btn, &style1_btn_pressed, LV_STATE_PRESSED);
    lv_obj_set_size(btn, 110, 34);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -12);
    lv_obj_add_event_cb(btn, pairing_done_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, "Done");
    lv_obj_add_style(btn_lbl, &style_par, LV_STATE_DEFAULT);
    lv_obj_center(btn_lbl);

    s_poll_timer = lv_timer_create(pairing_poll_cb, 3000, nullptr);
}

} /* extern "C" */
