/**
 * @file sdrumo_opening_screen_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "sdrumo_opening_screen_gen.h"
#include "ui_1.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * sdrumo_opening_screen_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_main;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_main);
        lv_style_set_bg_color(&style_main, lv_color_hex(0xf1ece6));

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_name_static(lv_obj_0, "sdrumo_opening_screen_#");

    lv_obj_add_style(lv_obj_0, &style_main, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_0, "Sdrumo");
    lv_obj_set_style_text_font(lv_label_0, Poppins_medium_48, 0);
    lv_obj_set_style_bg_color(lv_label_0, lv_color_hex(0xf1ece6), 0);
    lv_obj_set_style_align(lv_label_0, LV_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lv_label_0, lv_color_hex(0x3e3121), 0);

    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

