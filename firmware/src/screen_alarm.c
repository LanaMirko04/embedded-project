#include "screen_alarm.h"

static const char *TAG = "screen_alarm";



void create_rect_bg(lv_obj_t * rect){
    lv_obj_set_size(rect, 51, 78);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0xe6dcd2), 0);
    lv_obj_set_style_radius(rect, 6, 0);
    lv_obj_set_style_pad_all(rect, 4, 0);
    lv_obj_set_style_border_width(rect, 0, 0);
}

void ui_load_screen_alarm(lv_obj_t *screen) {

    //rectangle for hours 1
    lv_obj_t *rect_h1 = lv_obj_create(screen);

    create_rect_bg(rect_h1);
    lv_obj_align(rect_h1, LV_ALIGN_TOP_LEFT, 39, 49);

    //rectangle for hours 2
    lv_obj_t *rect_h2 = lv_obj_create(screen);

    create_rect_bg(rect_h2);
    lv_obj_align(rect_h2, LV_ALIGN_TOP_LEFT, 95, 49);


    lv_obj_t *separator = lv_label_create(screen);
    lv_label_set_text(separator, ":");
    lv_obj_add_style(separator, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(separator, LV_ALIGN_TOP_LEFT, 155, 58);


    //rectangle for minutes 1
    lv_obj_t *rect_m1 = lv_obj_create(screen);

    create_rect_bg(rect_m1);
    lv_obj_align(rect_m1, LV_ALIGN_TOP_LEFT, 174, 49);


    //rectangle for minutes 2
    lv_obj_t *rect_m2 = lv_obj_create(screen);

    create_rect_bg(rect_m2);
    lv_obj_align(rect_m2, LV_ALIGN_TOP_LEFT, 230, 49);

    
    
}
