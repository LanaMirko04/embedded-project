#include "screen_weather.h"
#include "core/lv_obj.h"
#include "core/lv_obj_pos.h"
#include "core/lv_obj_style.h"
#include "core/lv_obj_style_gen.h"
#include "images/partly_cloudy.c"
#include "images/rainy.c"
#include "images/snow.c"
#include "images/storm.c"
#include "images/sunny.c"
#include "images/wind.c"
#include "images/fog.c"
#include "images/cloud.c"
#include "images/humidity.c"
#include "layouts/flex/lv_flex.h"
#include "misc/lv_area.h"
#include "misc/lv_types.h"
#include "styles.h"
#include "widgets/image/lv_image.h"
#include "widgets/label/lv_label.h"

#define WIDTH 50
#define HEIGHT 78
#define MARGIN_X 56
#define MARGIN_Y 22
#define SPACE_BETW_RECT 27


static const char *TAG = "screen_weather";

//Example of how to fill weather_data
Weather weather_data = {.city = "Trento", 
                        .date = "Thu 19 Mar", 
                        .today_temp = "13°C", 
                        .today_weath = "Cloudy", 
                        .today_rain = "100%", 
                        .today_hummidity = "100%", 
                        .today_wind = "100 km/h",
                        .today_weath_icon = partly_cloudy, 
                        .day1 = "Fri", .weather_1 = "Cloudy", .weather_1_icon = cloud, .temp_1 = "11°C",
                        .day2 = "Sat", .weather_2 = "Sunny", .weather_2_icon = sunny, .temp_2 = "15°C",
                        .day3 = "Sun", .weather_3 = "Snowy", .weather_3_icon = snow, .temp_3 = "0°C"};

static void create_rect_bg(lv_obj_t *rect) {
    lv_obj_set_size(rect, WIDTH, HEIGHT);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0xe6dcd2), 0);
    lv_obj_set_style_radius(rect, 6, 0);
    lv_obj_set_style_pad_all(rect, 4, 0);
    lv_obj_set_style_border_width(rect, 0, 0);
}

static void create_rect_info_bg(lv_obj_t *rect){
    lv_obj_set_size(rect, 208, 27);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0xe6dcd2), 0);
    lv_obj_set_style_radius(rect, 6, 0);
    lv_obj_set_style_pad_all(rect, 3, 0);
    lv_obj_set_style_border_width(rect, 0, 0);
}

void ui_load_screen_weather(lv_obj_t *screen) {

    /* icon weather */
    lv_obj_t *img_weath = lv_image_create(screen);
    lv_image_set_src(img_weath, &weather_data.today_weath_icon);
    lv_image_set_scale(img_weath, 442);
    lv_obj_align(img_weath, LV_ALIGN_TOP_LEFT, 33, 22);


    /* city and date label */
    lv_obj_t *lbl_date = lv_label_create(screen);
    char date_text[66];
    
    snprintf(date_text, sizeof(date_text),
             "%s, %s",
             weather_data.city,
             weather_data.date);
    lv_label_set_text(lbl_date, date_text);
    lv_obj_align(lbl_date, LV_ALIGN_TOP_LEFT, 106, 30);
    lv_obj_add_style(lbl_date, &style_label_16, 0);


    /* weather and temperature label */
    lv_obj_t *lbl_temp = lv_label_create(screen);
    char temp_text[66];
    
    snprintf(temp_text, sizeof(temp_text),
             "%s, %s",
             weather_data.today_weath,
             weather_data.today_temp);
    lv_label_set_text(lbl_temp, temp_text);
    lv_obj_align(lbl_temp, LV_ALIGN_TOP_LEFT, 106, 52); //da modificare le dimensioni
    lv_obj_add_style(lbl_temp, &style_subtitle, 0);

    /* information bar */
    lv_obj_t *rect_info = lv_obj_create(screen);
    create_rect_info_bg(rect_info);
    lv_obj_align(rect_info, LV_ALIGN_BOTTOM_LEFT, MARGIN_X, -(HEIGHT + MARGIN_Y + 19));
    lv_obj_remove_flag(rect_info, LV_OBJ_FLAG_SCROLLABLE);

    // probability of rain
    lv_obj_t *img_prob_rain = lv_image_create(rect_info);
    lv_image_set_src(img_prob_rain, &rainy);
    lv_image_set_scale(img_prob_rain, 128);  
    lv_obj_align(img_prob_rain, LV_ALIGN_LEFT_MID, -9, 0);

    lv_obj_t *lbl_prob_rain = lv_label_create(rect_info);
    lv_label_set_text(lbl_prob_rain, weather_data.today_rain);
    lv_obj_align(lbl_prob_rain, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_add_style(lbl_prob_rain, &style_par, 0);

    //humidity
    lv_obj_t *img_humidity = lv_image_create(rect_info);
    lv_image_set_src(img_humidity, &humidity);
    lv_obj_align(img_humidity, LV_ALIGN_CENTER, -32, 0);

    lv_obj_t *lbl_humidity = lv_label_create(rect_info);
    lv_label_set_text(lbl_humidity, weather_data.today_hummidity);
    lv_obj_align(lbl_humidity, LV_ALIGN_CENTER, -7, 0);
    lv_obj_add_style(lbl_humidity, &style_par, 0);

    //wind
    lv_obj_t *img_wind = lv_image_create(rect_info);
    lv_image_set_src(img_wind, &wind); 
    lv_obj_align(img_wind, LV_ALIGN_RIGHT_MID, -65, 0);

    lv_obj_t *lbl_wind = lv_label_create(rect_info);
    lv_label_set_text(lbl_wind, weather_data.today_wind);
    lv_obj_align(lbl_wind, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_style(lbl_wind, &style_par, 0);

    

  

    /* day 1 */
    lv_obj_t *rect_t1 = lv_obj_create(screen);

    create_rect_bg(rect_t1);
    lv_obj_align(rect_t1, LV_ALIGN_BOTTOM_LEFT, MARGIN_X, - MARGIN_Y);

    lv_obj_set_layout(rect_t1, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(rect_t1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rect_t1,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    
    lv_obj_set_style_pad_row(rect_t1, 0, 0);
    lv_obj_set_style_pad_all(rect_t1, 2, 0);

    lv_obj_t *lbl_day1 = lv_label_create(rect_t1);
    lv_label_set_text(lbl_day1, weather_data.day1);
    lv_obj_add_style(lbl_day1, &style_par, 0);
    
    lv_obj_t *img_weather1 = lv_image_create(rect_t1);
    lv_image_set_src(img_weather1, &weather_data.weather_1_icon);
    
    lv_obj_t *lbl_temp1 = lv_label_create(rect_t1);
    lv_label_set_text(lbl_temp1, weather_data.temp_1);
    lv_obj_add_style(lbl_temp1, &style_par, 0);


    /* day 2 */
    lv_obj_t *rect_t2 = lv_obj_create(screen);

    create_rect_bg(rect_t2);
    lv_obj_align(rect_t2, LV_ALIGN_BOTTOM_LEFT, MARGIN_X + WIDTH + SPACE_BETW_RECT, - MARGIN_Y);

    lv_obj_set_layout(rect_t2, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(rect_t2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rect_t2,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    
    lv_obj_set_style_pad_row(rect_t2, 0, 0);
    lv_obj_set_style_pad_all(rect_t2, 2, 0);

    lv_obj_t *lbl_day2 = lv_label_create(rect_t2);
    lv_label_set_text(lbl_day2, weather_data.day2);
    lv_obj_add_style(lbl_day2, &style_par, 0);
    
    lv_obj_t *img_weather2 = lv_image_create(rect_t2);
    lv_image_set_src(img_weather2, &weather_data.weather_2_icon);
    
    lv_obj_t *lbl_temp2 = lv_label_create(rect_t2);
    lv_label_set_text(lbl_temp2, weather_data.temp_2);
    lv_obj_add_style(lbl_temp2, &style_par, 0);


    /* day 3 */
    lv_obj_t *rect_t3 = lv_obj_create(screen);

    create_rect_bg(rect_t3);
    lv_obj_align(rect_t3, LV_ALIGN_BOTTOM_LEFT, MARGIN_X + 2 * WIDTH + 2 * SPACE_BETW_RECT, - MARGIN_Y);

    lv_obj_set_layout(rect_t3, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(rect_t3, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rect_t3,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    
    lv_obj_set_style_pad_row(rect_t3, 0, 0);
    lv_obj_set_style_pad_all(rect_t3, 2, 0);

    lv_obj_t *lbl_day3 = lv_label_create(rect_t3);
    lv_label_set_text(lbl_day3, weather_data.day3);
    lv_obj_add_style(lbl_day3, &style_par, 0);
    
    lv_obj_t *img_weather3 = lv_image_create(rect_t3);
    lv_image_set_src(img_weather3, &weather_data.weather_3_icon);
    
    lv_obj_t *lbl_temp3 = lv_label_create(rect_t3);
    lv_label_set_text(lbl_temp3, weather_data.temp_3);
    lv_obj_add_style(lbl_temp3, &style_par, 0);
}
