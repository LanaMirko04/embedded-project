#include "screen_weather.h"
#include ""

static const char *TAG = "screen_weather";


typedef struct {
    char city[32];
    char date[32];

    int today_temp;
    char today_weath[32];

    int today_hummidity;
    int today_rain;
    int today_wind;

    char day1[4];
    char weather_1[32];
    int temp_1;

    char day2[4];
    char weather_2[32];
    int temp_2;

    char day3[4];
    char weather_3[32];
    int temp_3;

} Weather;

void ui_load_screen_weather(lv_obj_t *screen) {
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "WEATHERRRR");
    lv_obj_add_style(label, &style_subtitle, LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
