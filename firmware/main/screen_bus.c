#include "screen_bus.h"

static uint16_t last_selected_bus = 0;

static const char *bus_options =
    "S. Francesco Porta Nuova\n"
    "Povo Valoni\n"
    "Piazza Fiera";

static char selected_text[64];

static void bus_dropdown_event_cb(lv_event_t *e)
{
    lv_obj_t *dd = lv_event_get_target(e);
    lv_obj_t *drop_down_menu = lv_event_get_user_data(e);
    
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        last_selected_bus = lv_dropdown_get_selected(dd);
    }

    lv_dropdown_get_selected_str(drop_down_menu,
                             selected_text,
                             sizeof(selected_text));
}

void ui_load_screen_bus(lv_obj_t *screen)
{
    lv_obj_t *drop_down_menu = lv_dropdown_create(screen);

    lv_dropdown_set_options(drop_down_menu, bus_options);

    // Restore last selection
    lv_dropdown_set_selected(drop_down_menu, last_selected_bus);

    lv_obj_set_width(drop_down_menu, 220);
    lv_obj_align(drop_down_menu, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_add_event_cb(drop_down_menu,
                        bus_dropdown_event_cb,
                        LV_EVENT_VALUE_CHANGED,
                        drop_down_menu);

    lv_dropdown_get_selected_str(drop_down_menu,
                             selected_text,
                             sizeof(selected_text));
}