#include "screen_clock.h"
#include "images/clock_screen.c"
#include "images/timer.c"

void ui_event_clock(lv_event_t *e){
    static uint8_t counter = 0;
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED){
        counter ++;
    }
}


void ui_load_screen_clock(lv_obj_t *screen){
    //background
    lv_obj_t *bg = lv_img_create(screen);
    lv_img_set_src(bg, &clock_screen);

    lv_obj_set_size(bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(bg, 0, 0);

    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);

    //timer button
    lv_obj_t *btn_timer = lv_button_create(screen);
    lv_obj_add_style(btn_timer, &style_btn, 0);
    lv_obj_add_style(btn_timer, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn_timer, &style_btn, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_timer, &style_btn, LV_STATE_DISABLED);
    //lv_obj_remove_style_all(btn_timer);

    lv_obj_align(btn_timer, LV_ALIGN_BOTTOM_LEFT, 65, -10);
    lv_obj_set_size(btn_timer, 28, 28);
    
    
    lv_obj_t *img_timer = lv_img_create(btn_timer);
    lv_img_set_src(img_timer, &timer);
    lv_obj_center(img_timer);
    //lv_obj_clear_flag(img_timer, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_add_event_cb(btn_timer, ui_event_clock, LV_EVENT_CLICKED, NULL);


}