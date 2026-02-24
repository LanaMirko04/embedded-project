#include "screen_manager.h"


static uint8_t screen_number = 0;
static const char *TAG = "screen_manager";


static void screen_timer_cb(lv_timer_t * t)
{
    lv_obj_t *screen = (lv_obj_t *)lv_timer_get_user_data(t);
    screen_number = (screen_number + 1) % 3;
    ESP_LOGI(TAG, "screen number: %d", screen_number);
    ui_load_screen(screen, screen_number);
}


void screen_manager_init(lv_obj_t *screen){

    // draw first screen
    ui_load_screen(screen, screen_number);

    // create auto-rotation timer
    lv_timer_create(screen_timer_cb, 5000, screen);
}


void ui_load_screen(lv_obj_t *screen, uint8_t num){

    screen_clock_destroy();

    lv_obj_clean(screen);

    switch (num){
        case 0: 
            ui_load_screen_boot(screen);
            break;
        case 1:
            ui_load_screen_wifi(screen);
            break;
        case 2: 
            ui_load_screen_clock(screen);
            break;
    }

}

