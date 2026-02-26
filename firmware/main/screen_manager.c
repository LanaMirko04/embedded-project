#include "screen_manager.h"



static const char *TAG = "screen_manager";



static void screen_timer_cb(lv_timer_t * t)
{
    lv_obj_t *screen = (lv_obj_t *)lv_timer_get_user_data(t);
    //screen_type = (screen_type + 1) % 3;
    screen_type = SCREEN_CLOCK;
    ESP_LOGI(TAG, "screen number: %d", screen_type);
    ui_load_screen(screen);
}


void screen_manager_init(lv_obj_t *screen){

    // draw first screen
    ui_load_screen(screen);

    // create auto-rotation timer
    lv_timer_create(screen_timer_cb, 5000, screen);
}


void ui_load_screen(lv_obj_t *screen){

    screen_clock_destroy();

    lv_obj_clean(screen);

    switch (screen_type){
        case SCREEN_BOOT: 
            ui_load_screen_boot(screen);
            break;
        case SCREEN_WIFI:
            ui_load_screen_wifi(screen);
            break;
        case SCREEN_CLOCK: 
            ui_load_screen_clock(screen);
            break;
        case SCREEN_BUS:
        case SCREEN_WEATHER: 
            ui_load_screen_wifi(screen);
    }

}

