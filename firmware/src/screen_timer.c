#include "screen_alarm.h"
#include "core/lv_obj_event.h"
#include "core/lv_obj_pos.h"
#include "esp_log.h"
#include "images/arrow_down.h"
#include "images/arrow_up.h"
#include "misc/lv_area.h"
#include "misc/lv_event.h"
#include "misc/lv_types.h"
#include "styles.h"
#include "utiles.h"
#include "widgets/button/lv_button.h"
#include "widgets/label/lv_label.h"
#include <stdio.h>

#define MARGIN_Y 49
#define MARGIN_X 39
#define SPACE_BTN 5
#define WIDTH 51
#define HEIGHT 78

static const char *TAG = "screen_timer";

char timer_data[6] = "00:00";

typedef struct {
    unsigned char val;
    unsigned char max;
} btn_user_data_t;

btn_user_data_t s_1 = {.val = 0, .max = 59};
static btn_user_data_t m_1 = {.val = 0, .max = 59};


/*background box for minutes and hours*/
static void create_rect_bg(lv_obj_t *rect) {
    lv_obj_set_size(rect, WIDTH, HEIGHT);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0xe6dcd2), 0);
    lv_obj_set_style_radius(rect, 6, 0);
    lv_obj_set_style_pad_all(rect, 4, 0);
    lv_obj_set_style_border_width(rect, 0, 0);
}

/*style of the internal buttons*/
static void set_button_style(lv_obj_t *btn) {
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_btn, LV_STATE_FOCUSED);
    lv_obj_add_style(btn, &style_btn, LV_STATE_DISABLED);

    lv_obj_set_size(btn, 53, 16);
}

/* style of select and cancel buttons */
static void set_ext_button_style(lv_obj_t *btn){
    lv_obj_add_style(btn, &style1_btn, 0);
    lv_obj_add_style(btn, &style1_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style1_btn, LV_STATE_FOCUSED);
    lv_obj_add_style(btn, &style1_btn, LV_STATE_DISABLED);

    lv_obj_set_size(btn, 64, 28);
}



/*event button down*/
static void ui_event_btn_down(lv_event_t *e) {
    btn_user_data_t *data = (btn_user_data_t *)lv_event_get_user_data(e);
    if (data->val > 0) {
        (data->val)--;
        change_status = true;
       //ESP_LOGI(TAG, "change status: %d", change_status);
    }
    //ESP_LOGI(TAG, "h1: %d", data->val);
}

/*event button down*/
static void ui_event_btn_down_10(lv_event_t *e) {
    btn_user_data_t *data = (btn_user_data_t *)lv_event_get_user_data(e);
    if (data->val > 10) {
        (data->val)-=10;
        change_status = true;
       //ESP_LOGI(TAG, "change status: %d", change_status);
    }
    //ESP_LOGI(TAG, "h1: %d", data->val);
}

/* event button up */
static void ui_event_btn_up(lv_event_t *e) {
    btn_user_data_t *data = (btn_user_data_t *)lv_event_get_user_data(e);

    if (data->val < data->max) {
        (data->val)++;
        change_status = true;
       //ESP_LOGI(TAG, "change status: %d", change_status);
    }
    //ESP_LOGI(TAG, "h1: %d", data->val);
}

static void ui_event_btn_up_10(lv_event_t *e) {
    btn_user_data_t *data = (btn_user_data_t *)lv_event_get_user_data(e);

    if (data->val < (data->max - 10)) {
        (data->val)+= 10;
        change_status = true;
       //ESP_LOGI(TAG, "change status: %d", change_status);
    }
    //ESP_LOGI(TAG, "h1: %d", data->val);
}


static void ui_event_cancel(lv_event_t *e){
    next_screen_type = SCREEN_CLOCK;
}

static void ui_event_select(lv_event_t *e){
    snprintf(timer_data, sizeof(timer_data), "%02d:%02d", m_1.val%60, s_1.val%60);
    ESP_LOGI(TAG, "Alarm data: %s", timer_data);
    next_screen_type = SCREEN_CLOCK;
}



void ui_load_screen_timer(lv_obj_t *screen) {

    char int_to_text[2];

    //hours 1
    lv_obj_t *rect_h1 = lv_obj_create(screen);

    create_rect_bg(rect_h1);
    lv_obj_align(rect_h1, LV_ALIGN_TOP_LEFT, 39, MARGIN_Y);

    //label (number)
    lv_obj_t *lb_h1 = lv_label_create(rect_h1);
    snprintf(int_to_text, 2, "%d", (m_1.val / 10)%10);
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

    lv_obj_add_event_cb(btn_down_h1, ui_event_btn_down_10, LV_EVENT_CLICKED, &m_1);


    //button up
    lv_obj_t *btn_up_h1 = lv_button_create(screen);
    set_button_style(btn_up_h1);
    lv_obj_align(btn_up_h1, LV_ALIGN_TOP_LEFT, 38, MARGIN_Y-24);

    lv_obj_t *img_arrow_up_h1 = lv_img_create(btn_up_h1);
    lv_img_set_src(img_arrow_up_h1, &arrow_up);
    lv_obj_center(img_arrow_up_h1);
    lv_obj_add_event_cb(btn_up_h1, ui_event_btn_up_10, LV_EVENT_CLICKED, &m_1);


    //hours 2
    lv_obj_t *rect_h2 = lv_obj_create(screen);

    create_rect_bg(rect_h2);
    lv_obj_align(rect_h2, LV_ALIGN_TOP_LEFT, 95, MARGIN_Y);

    //label (number)
    lv_obj_t *lb_h2 = lv_label_create(rect_h2);
    snprintf(int_to_text, 2, "%d", m_1.val % 10);
    lv_label_set_text(lb_h2, int_to_text);
    lv_obj_add_style(lb_h2, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(lb_h2, LV_ALIGN_CENTER, 0, 0);

    //button down
    lv_obj_t *btn_down_h2 = lv_button_create(screen);
    set_button_style(btn_down_h2);
    lv_obj_align(btn_down_h2, LV_ALIGN_TOP_LEFT, 94, 135);

    lv_obj_t *img_arrow_down_h2 = lv_img_create(btn_down_h2);
    lv_img_set_src(img_arrow_down_h2, &arrow_down);
    lv_obj_center(img_arrow_down_h2);

    lv_obj_add_event_cb(btn_down_h2, ui_event_btn_down, LV_EVENT_CLICKED, &m_1);


    //button up
    lv_obj_t *btn_up_h2 = lv_button_create(screen);
    set_button_style(btn_up_h2);
    lv_obj_align(btn_up_h2, LV_ALIGN_TOP_LEFT, 94, MARGIN_Y-24);

    lv_obj_t *img_arrow_up_h2 = lv_img_create(btn_up_h2);
    lv_img_set_src(img_arrow_up_h2, &arrow_up);
    lv_obj_center(img_arrow_up_h2);
    lv_obj_add_event_cb(btn_up_h2, ui_event_btn_up, LV_EVENT_CLICKED, &m_1);




    //separator
    lv_obj_t *separator = lv_label_create(screen);
    lv_label_set_text(separator, ":");
    lv_obj_add_style(separator, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(separator, LV_ALIGN_TOP_LEFT, 155, 58);

    //minutes 1
    lv_obj_t *rect_m1 = lv_obj_create(screen);

    create_rect_bg(rect_m1);
    lv_obj_align(rect_m1, LV_ALIGN_TOP_LEFT, 174, MARGIN_Y);

    //label (number)
    lv_obj_t *lb_m1 = lv_label_create(rect_m1);
    snprintf(int_to_text, 2, "%d", (s_1.val / 10)%10);
    lv_label_set_text(lb_m1, int_to_text);
    lv_obj_add_style(lb_m1, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(lb_m1, LV_ALIGN_CENTER, 0, 0);

    //button down
    lv_obj_t *btn_down_m1 = lv_button_create(screen);
    set_button_style(btn_down_m1);
    lv_obj_align(btn_down_m1, LV_ALIGN_TOP_LEFT, 173, 135);

    lv_obj_t *img_arrow_down_m1 = lv_img_create(btn_down_m1);
    lv_img_set_src(img_arrow_down_m1, &arrow_down);
    lv_obj_center(img_arrow_down_m1);

    lv_obj_add_event_cb(btn_down_m1, ui_event_btn_down_10, LV_EVENT_CLICKED, &s_1);

    //button up
    lv_obj_t *btn_up_m1 = lv_button_create(screen);
    set_button_style(btn_up_m1);
    lv_obj_align(btn_up_m1, LV_ALIGN_TOP_LEFT, 173, MARGIN_Y-24);

    lv_obj_t *img_arrow_up_m1 = lv_img_create(btn_up_m1);
    lv_img_set_src(img_arrow_up_m1, &arrow_up);
    lv_obj_center(img_arrow_up_m1);
    lv_obj_add_event_cb(btn_up_m1, ui_event_btn_up_10, LV_EVENT_CLICKED, &s_1);



    //minutes 2
    lv_obj_t *rect_m2 = lv_obj_create(screen);

    create_rect_bg(rect_m2);
    lv_obj_align(rect_m2, LV_ALIGN_TOP_LEFT, 230, MARGIN_Y);

    //label (number)
    lv_obj_t *lb_m2 = lv_label_create(rect_m2);
    snprintf(int_to_text, 2, "%d", s_1.val % 10);
    lv_label_set_text(lb_m2, int_to_text);
    lv_obj_add_style(lb_m2, &style_title, LV_STATE_DEFAULT);
    lv_obj_align(lb_m2, LV_ALIGN_CENTER, 0, 0);


    //button down
    lv_obj_t *btn_down_m2 = lv_button_create(screen);
    set_button_style(btn_down_m2);
    lv_obj_align(btn_down_m2, LV_ALIGN_TOP_LEFT, 229, 135);

    lv_obj_t *img_arrow_down_m2 = lv_img_create(btn_down_m2);
    lv_img_set_src(img_arrow_down_m2, &arrow_down);
    lv_obj_center(img_arrow_down_m2);

    lv_obj_add_event_cb(btn_down_m2, ui_event_btn_down, LV_EVENT_CLICKED, &s_1);

    //button up
    lv_obj_t *btn_up_m2 = lv_button_create(screen);
    set_button_style(btn_up_m2);
    lv_obj_align(btn_up_m2, LV_ALIGN_TOP_LEFT, 229, MARGIN_Y-24);

    lv_obj_t *img_arrow_up_m2 = lv_img_create(btn_up_m2);
    lv_img_set_src(img_arrow_up_m2, &arrow_up);
    lv_obj_center(img_arrow_up_m2);
    lv_obj_add_event_cb(btn_up_m2, ui_event_btn_up, LV_EVENT_CLICKED, &s_1);



    /* button cancel  */
    lv_obj_t *btn_cancel = lv_button_create(screen);
    set_ext_button_style(btn_cancel);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_MID, - 48, - 48);
    
    lv_obj_t *label_cnc = lv_label_create(btn_cancel);
    lv_label_set_text(label_cnc, "Cancel");
    //lv_obj_center(label);
    lv_obj_add_style(label_cnc, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_align(label_cnc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_cancel, ui_event_cancel, LV_EVENT_CLICKED, NULL);



    /* button select */
    lv_obj_t *btn_select = lv_button_create(screen);
    set_ext_button_style(btn_select);
    lv_obj_align(btn_select, LV_ALIGN_BOTTOM_MID, 48, - 48);
    
    lv_obj_t *label_slc = lv_label_create(btn_select);
    lv_label_set_text(label_slc, "Set");
    //lv_obj_center(label);
    lv_obj_add_style(label_slc, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_align(label_slc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_select, ui_event_select, LV_EVENT_CLICKED, NULL);
}
