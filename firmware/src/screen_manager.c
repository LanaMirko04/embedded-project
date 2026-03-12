#include "screen_manager.h"

static const char *TAG = "screen_manager";

/* every 100 ms check if present_screen_type has changed*/
static void screen_timer_cb(lv_timer_t * t){

    lv_obj_t *screen = (lv_obj_t *)lv_timer_get_user_data(t);
    if(present_screen_type != next_screen_type){
        present_screen_type = next_screen_type;
        ui_load_screen(screen);
        ESP_LOGI(TAG, "screen type %d", present_screen_type);
    }
    
}




void screen_manager_init(lv_obj_t *screen) {

    // draw first screen
    ui_load_screen(screen);

    // create auto-rotation timer
    lv_timer_create(screen_timer_cb, 100, screen);
    
}

void ui_load_screen(lv_obj_t *screen) {

    screen_clock_destroy();

    lv_obj_clean(screen);

    switch (present_screen_type) {
        case SCREEN_BOOT:
            ui_load_screen_bus(screen);
            //ui_load_screen_boot(screen);
            ui_load_arrows_btn(screen);
            break;
        case SCREEN_WIFI:
            ui_load_screen_wifi(screen);
            ui_load_arrows_btn(screen);

            break;
        case SCREEN_CLOCK:
            ui_load_screen_clock(screen);
            ui_load_arrows_btn(screen);
            break;
        case SCREEN_BUS:
            ui_load_screen_bus(screen);
            ui_load_arrows_btn(screen);
            break;
        case SCREEN_WEATHER:
        case SCREEN_NUMBER:
            ui_load_screen_wifi(screen);
            ui_load_arrows_btn(screen);
    }
}
