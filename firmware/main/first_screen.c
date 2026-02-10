#include "first_screen.h"
#include "images/clock_screen.c"
#include "images/timer.c"
static lv_style_t subtitle_style;

static lv_style_t title_style;

static lv_style_t par_style;

static lv_style_t btn_style;

static lv_style_t btn_style_pressed;

void styles_init() {
    lv_style_init(&subtitle_style);
    lv_style_set_text_color(&subtitle_style, lv_color_make(0x3E, 0x15, 0x00));
    lv_style_set_text_font(&subtitle_style, &lv_font_montserrat_32);

    lv_style_init(&title_style);
    lv_style_set_text_color(&title_style, lv_color_make(0x3E, 0x15, 0x00));
    lv_style_set_text_font(&title_style, &lv_font_montserrat_48);

    lv_style_init(&par_style);
    lv_style_set_text_color(&par_style, lv_color_make(0x3E, 0x15, 0x00));
    lv_style_set_text_font(&par_style, &lv_font_montserrat_14);

    //lv_style_set_shadow_width(&btn_style, 10);
    //lv_style_set_shadow_color(&btn_style, lv_color_black());
    //lv_style_set_shadow_opa(&btn_style, LV_OPA_50);
    //lv_style_set_shadow_offset_x(&btn_style, 0);
    //lv_style_set_shadow_offset_y(&btn_style, 3);
    
    // Make background fully transparent
    lv_style_init(&btn_style);
    lv_style_set_bg_opa(&btn_style, LV_OPA_TRANSP);
    lv_style_set_border_opa(&btn_style, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&btn_style, LV_OPA_TRANSP);

    // Keep pressed visual: soft overlay
    lv_style_init(&btn_style_pressed);
    lv_style_set_bg_color(&btn_style_pressed, lv_color_black());
    lv_style_set_bg_opa(&btn_style_pressed, LV_OPA_30);
    
}

void draw_first_screen(lv_obj_t *screen) {

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "Sdrumo");
    lv_obj_add_style(label, &title_style, LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void draw_wifi_screen(lv_obj_t *screen) {

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "WiFi not found!");
    lv_obj_add_style(label, &subtitle_style, LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label1 = lv_label_create(screen);
    lv_label_set_text(label1, "Open EspTouch to configure WiFi");
    lv_obj_add_style(label1, &par_style, LV_STATE_DEFAULT);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, +30);
}

void ui_event_clock(lv_event_t *e){
    static uint8_t counter = 0;
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED){
        counter ++;
    }
}


void draw_clock_screen(lv_obj_t *screen){
    //background
    lv_obj_t *bg = lv_img_create(screen);
    lv_img_set_src(bg, &clock_screen);

    lv_obj_set_size(bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(bg, 0, 0);

    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);

    //timer button
    lv_obj_t *btn_timer = lv_button_create(screen);
    lv_obj_add_style(btn_timer, &btn_style, 0);
    lv_obj_add_style(btn_timer, &btn_style_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn_timer, &btn_style, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_timer, &btn_style, LV_STATE_DISABLED);
    //lv_obj_remove_style_all(btn_timer);

    lv_obj_align(btn_timer, LV_ALIGN_BOTTOM_LEFT, 65, -10);
    lv_obj_set_size(btn_timer, 28, 28);
    
    
    lv_obj_t *img_timer = lv_img_create(btn_timer);
    lv_img_set_src(img_timer, &timer);
    lv_obj_center(img_timer);
    //lv_obj_clear_flag(img_timer, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_add_event_cb(btn_timer, ui_event_clock, LV_EVENT_CLICKED, NULL);


}