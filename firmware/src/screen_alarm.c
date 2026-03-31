#include "screen_alarm.h"
#include "images/arrow_down.c"
#include "images/arrow_up.c"


static const char *TAG = "screen_alarm";

int h_1 = 0;
int h_2 = 0;
int m_1 = 0;
int m_2 = 0;



void create_rect_bg(lv_obj_t * rect){
    lv_obj_set_size(rect, 51, 78);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0xe6dcd2), 0);
    lv_obj_set_style_radius(rect, 6, 0);
    lv_obj_set_style_pad_all(rect, 4, 0);
    lv_obj_set_style_border_width(rect, 0, 0);
}

void set_button_style(lv_obj_t *btn){
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_btn, LV_STATE_FOCUSED);
    lv_obj_add_style(btn, &style_btn, LV_STATE_DISABLED);

    lv_obj_set_size(btn, 53, 16);
}

void ui_load_screen_alarm(lv_obj_t *screen) {

    char int_to_text[2];

//hours 1
    lv_obj_t *rect_h1 = lv_obj_create(screen);

    create_rect_bg(rect_h1);
    lv_obj_align(rect_h1, LV_ALIGN_TOP_LEFT, 39, 49);

    //label (number)
    lv_obj_t *lb_h1 = lv_label_create(rect_h1);
    snprintf(int_to_text, 2, "%d", h_1);
    lv_label_set_text(lb_h1, int_to_text);
    lv_obj_add_style(lb_h1, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(lb_h1, LV_ALIGN_CENTER, 0, 0);

    //button down
    lv_obj_t *btn_down_h1 = lv_button_create(screen);
    set_button_style(btn_down_h1);
    lv_obj_align(btn_down_h1, LV_ALIGN_TOP_LEFT, 38, 135);

    lv_obj_t *img_arrow_down_h1 = lv_img_create(btn_down_h1);
    lv_img_set_src(img_arrow_down_h1, &arrow_down);
    lv_obj_center(img_arrow_down_h1);

    //lv_obj_add_event_cb(btn_arrow_right, ui_event_arrow_right, LV_EVENT_CLICKED, NULL);


//hours 2
    lv_obj_t *rect_h2 = lv_obj_create(screen);

    create_rect_bg(rect_h2);
    lv_obj_align(rect_h2, LV_ALIGN_TOP_LEFT, 95, 49);

    //label (number)
    lv_obj_t *lb_h2 = lv_label_create(rect_h2);
    snprintf(int_to_text, 2, "%d", h_2);
    lv_label_set_text(lb_h2, int_to_text);
    lv_obj_add_style(lb_h2, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(lb_h2, LV_ALIGN_CENTER, 0, 0);


    //separator
    lv_obj_t *separator = lv_label_create(screen);
    lv_label_set_text(separator, ":");
    lv_obj_add_style(separator, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(separator, LV_ALIGN_TOP_LEFT, 155, 58);


//minutes 1
    lv_obj_t *rect_m1 = lv_obj_create(screen);

    create_rect_bg(rect_m1);
    lv_obj_align(rect_m1, LV_ALIGN_TOP_LEFT, 174, 49);

    //label (number)
    lv_obj_t *lb_m1 = lv_label_create(rect_m1);
    snprintf(int_to_text, 2, "%d", m_1);
    lv_label_set_text(lb_m1, int_to_text);
    lv_obj_add_style(lb_m1, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(lb_m1, LV_ALIGN_CENTER, 0, 0);


//minutes 2
    lv_obj_t *rect_m2 = lv_obj_create(screen);

    create_rect_bg(rect_m2);
    lv_obj_align(rect_m2, LV_ALIGN_TOP_LEFT, 230, 49);

    //label (number)
    lv_obj_t *lb_m2 = lv_label_create(rect_m2);
    snprintf(int_to_text, 2, "%d", m_2);
    lv_label_set_text(lb_m2, int_to_text);
    lv_obj_add_style(lb_m2, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(lb_m2, LV_ALIGN_CENTER, 0, 0);
    
    
}
