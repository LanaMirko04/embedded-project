#include "styles.h"

lv_style_t style_title;
lv_style_t style_subtitle;
lv_style_t style_par;
lv_style_t style_btn;
lv_style_t style_btn_pressed;
lv_style_t style_label_16;


void styles_init() {

    /* style of title */
    lv_style_init(&style_title);
    lv_style_set_text_color(&style_title, lv_color_make(0x3E, 0x15, 0x00));
    lv_style_set_text_font(&style_title, &lv_font_montserrat_48);

    /* style of subtitle */
    lv_style_init(&style_subtitle);
    lv_style_set_text_color(&style_subtitle, lv_color_make(0x3E, 0x15, 0x00));
    lv_style_set_text_font(&style_subtitle, &lv_font_montserrat_32);

    /* style of paragraph */
    lv_style_init(&style_par);
    lv_style_set_text_color(&style_par, lv_color_make(0x3E, 0x15, 0x00));
    lv_style_set_text_font(&style_par, &lv_font_montserrat_14);

    /* style of button */
    // Make background fully transparent
    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_btn, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_btn, LV_OPA_TRANSP);

    /* style of button pressed */
    // Keep pressed visual: soft overlay
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, lv_color_black());
    lv_style_set_bg_opa(&style_btn_pressed, LV_OPA_30);

    /* style of label_16 */
    lv_style_init(&style_label_16);
    lv_style_set_text_color(&style_label_16, lv_color_make(0x3E, 0x15, 0x00));
    lv_style_set_text_font(&style_label_16, &lv_font_montserrat_16);
}
