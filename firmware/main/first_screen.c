#include "first_screen.h"
#include "images/clock_screen.c"
static lv_style_t subtitle_style;

static lv_style_t title_style;

static lv_style_t par_style;

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

void draw_clock_screen(lv_obj_t *screen){
    lv_obj_t *bg = lv_img_create(screen);
    lv_img_set_src(bg, &clock_screen);

    lv_obj_set_size(bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(bg, 0, 0);

    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
}