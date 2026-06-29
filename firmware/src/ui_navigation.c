#include "ui_navigation.h"
#include "images/arrow_right.c"
#include "images/arrow_left.c"

const char * TAGG = "ui_navigation";

void ui_event_arrow_left(lv_event_t *e){
    if (present_screen_type == SCREEN_CLOCK){
        next_screen_type = SCREEN_NUMBER - 1;
        ESP_LOGI(TAGG, "screen number: %d", next_screen_type);
    } else{
        next_screen_type = present_screen_type - 1;
        ESP_LOGI(TAGG, "screen number: %d", next_screen_type);
    }
}

void ui_event_arrow_right(lv_event_t *e){
    next_screen_type = (present_screen_type + 1) % SCREEN_NUMBER;
}


void ui_load_arrows_btn(lv_obj_t *screen){

    /* btn arrow right */
    lv_obj_t *btn_arrow_right = lv_button_create(screen);

    lv_obj_add_style(btn_arrow_right, &style_btn, 0);
    lv_obj_add_style(btn_arrow_right, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn_arrow_right, &style_btn, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_arrow_right, &style_btn, LV_STATE_DISABLED);

    lv_obj_align(btn_arrow_right, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_size(btn_arrow_right, 24, 51);

    lv_obj_t *img_arrow_right = lv_img_create(btn_arrow_right);
    lv_img_set_src(img_arrow_right, &arrow_right);
    lv_obj_center(img_arrow_right);

    lv_obj_add_event_cb(btn_arrow_right, ui_event_arrow_right, LV_EVENT_CLICKED, NULL);

    /* btn arrow left */
    lv_obj_t *btn_arrow_left = lv_button_create(screen);
    lv_obj_add_style(btn_arrow_left, &style_btn, 0);
    lv_obj_add_style(btn_arrow_left, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn_arrow_left, &style_btn, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_arrow_left, &style_btn, LV_STATE_DISABLED);

    lv_obj_align(btn_arrow_left, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_size(btn_arrow_left, 24, 51);

    lv_obj_t *img_arrow_left = lv_img_create(btn_arrow_left);
    lv_img_set_src(img_arrow_left, &arrow_left);
    lv_obj_center(img_arrow_left);

    lv_obj_add_event_cb(btn_arrow_left, ui_event_arrow_left, LV_EVENT_CLICKED, NULL);
}