#include "screen_wifi.h"


void ui_load_screen_wifi (lv_obj_t *screen) {

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "WiFi not found!");
    lv_obj_add_style(label, &style_subtitle, LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label1 = lv_label_create(screen);
    lv_label_set_text(label1, "Open EspTouch to configure WiFi");
    lv_obj_add_style(label1, &style_par, LV_STATE_DEFAULT);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, +30);
}