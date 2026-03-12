#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "screen_clock.h"
#include "images/clock_screen.c"
#include "images/timer.c"
#include "images/alarm.c"

const char *TAG = "screen_clock";

static lv_obj_t *label_timer;
static lv_obj_t *label_alarm;

/* clock hands */
static lv_obj_t *min_hand = NULL;
static lv_obj_t *hour_hand = NULL;

static lv_point_t min_points[2];
static lv_point_t hour_points[2];

static lv_timer_t *clock_timer = NULL;

static int clock_cx;
static int clock_cy;

static void clock_update(uint8_t h, uint8_t m);

void ui_event_clock(lv_event_t *e) {
    static uint8_t counter = 0;
    //lv_event_code_t event_code = lv_event_get_code(e);
    //lv_obj_t *btn = (lv_obj_t *)lv_event_get_user_data(e);
    //if (event_code == LV_EVENT_CLICKED) {
    counter++;
    //}
}

/* destroy timer */
void screen_clock_destroy(void) {
    if (clock_timer) {
        lv_timer_del(clock_timer);
        clock_timer = NULL;
    }
}

/* update position of clock hands */
static void clock_update(uint8_t h, uint8_t m) {

    float min_angle = ((m * 6.0f) - 90.0f) * M_PI / 180.0f;
    float hour_angle = (((h % 12) * 30.0f + m * 0.5f) - 90.0f) * M_PI / 180.0f;

    // Start point (center)
    hour_points[0].x = clock_cx;
    hour_points[0].y = clock_cy;

    min_points[0].x = clock_cx;
    min_points[0].y = clock_cy;

    // End point calculations
    hour_points[1].x = clock_cx + 50 * cosf(hour_angle);
    hour_points[1].y = clock_cy + 50 * sinf(hour_angle);

    min_points[1].x = clock_cx + 80 * cosf(min_angle);
    min_points[1].y = clock_cy + 80 * sinf(min_angle);

    lv_line_set_points(hour_hand, hour_points, 2);
    lv_line_set_points(min_hand, min_points, 2);
}

/* create right clock */
static void clock_timer_cb(lv_timer_t *t) {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    static int hour = -1;
    static int minute = -1;

    if (hour != timeinfo.tm_hour || minute != timeinfo.tm_min) {
        clock_update(timeinfo.tm_hour, timeinfo.tm_min);
    }

    ESP_LOGI(TAG, "%02d:%02d", hour, minute);

    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
}

/* create clock hands */
void clock_create(lv_obj_t *parent) {

    clock_cx = lv_obj_get_width(parent) / 2;
    clock_cy = lv_obj_get_height(parent) / 2 - 15;

    hour_hand = lv_line_create(parent);
    lv_line_set_points(hour_hand, hour_points, 2);
    lv_obj_set_style_line_width(hour_hand, 8, 0);
    lv_obj_set_style_line_color(hour_hand, lv_color_hex(0x610B0B), 0);

    min_hand = lv_line_create(parent);
    lv_line_set_points(min_hand, min_points, 2);
    lv_obj_set_style_line_width(min_hand, 5, 0);
    lv_obj_set_style_line_color(min_hand, lv_color_hex(0x610B0B), 0);

    // Initial position
    //clock_update(12, 0);
    clock_timer_cb(NULL);

    // Create timer
    clock_timer = lv_timer_create(clock_timer_cb, 1000, NULL);
}

/* draw clock screen */
void ui_load_screen_clock(lv_obj_t *screen) {

    /* background */
    lv_obj_t *bg = lv_img_create(screen);
    lv_img_set_src(bg, &clock_screen);

    lv_obj_set_size(bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(bg, 0, 0);

    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);

    /* timer button */
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

    /* timer label */
    label_timer = lv_label_create(screen);
    lv_label_set_text(label_timer, "00:00"); // initial value
    lv_obj_align_to(label_timer, btn_timer, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_style(label_timer, &style_par, 0);

    /* alarm button */
    lv_obj_t *btn_alarm = lv_button_create(screen);
    lv_obj_add_style(btn_alarm, &style_btn, 0);
    lv_obj_add_style(btn_alarm, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn_alarm, &style_btn, LV_STATE_FOCUSED);
    lv_obj_add_style(btn_alarm, &style_btn, LV_STATE_DISABLED);

    lv_obj_align(btn_alarm, LV_ALIGN_BOTTOM_RIGHT, -110, -10);
    lv_obj_set_size(btn_alarm, 28, 28);

    lv_obj_t *img_alarm = lv_img_create(btn_alarm);
    lv_img_set_src(img_alarm, &alarm1);
    lv_obj_center(img_alarm);

    lv_obj_add_event_cb(btn_alarm, ui_event_clock, LV_EVENT_CLICKED, NULL);

    /* alarm label */
    label_alarm = lv_label_create(screen);
    lv_label_set_text(label_alarm, "00:00"); // initial value
    lv_obj_align_to(label_alarm, btn_alarm, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_style(label_alarm, &style_par, 0);

    /* to manage clock hands */
    clock_create(screen);
}
