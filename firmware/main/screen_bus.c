#include "screen_bus.h"

void ui_load_screen_bus(lv_obj_t *screen){
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "Busses");
    lv_obj_add_style(label, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}