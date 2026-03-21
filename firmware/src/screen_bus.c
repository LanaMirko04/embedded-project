#include "screen_bus.h"

#define ROW_WIDTH 270
#define ROW_HEIGHT 32
#define PADDING 2
#define CIRCLE_RADIUS 8

typedef struct {
    char arrival_time[32];
    uint8_t bus_color[3];
    int bus_id;
    char bus_name[64];
    char bus_number[8];
    float delay;
    char eta[5]; //estimated time of arrival
} Bus;

static uint16_t last_selected_bus = 0;

static const char *bus_options =
    "S. Francesco Porta Nuova\n"
    "Povo Valoni\n"
    "Piazza Fiera";

static char selected_text[64];

static lv_timer_t *server_request_time = NULL;








/* dropdown menu event */
static void bus_dropdown_event_cb(lv_event_t *e) {

    lv_obj_t *dd = lv_event_get_target(e);
    lv_obj_t *drop_down_menu = lv_event_get_user_data(e);

    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        last_selected_bus = lv_dropdown_get_selected(dd);
    }

    lv_dropdown_get_selected_str(drop_down_menu,
                                 selected_text,
                                 sizeof(selected_text));
}



/* bus row */
void create_bus_row(Bus bus, lv_obj_t *screen, int index) {

    /* row box */
    lv_obj_t *row = lv_obj_create(screen);
    lv_obj_set_width(row, ROW_WIDTH);
    lv_obj_set_height(row, ROW_HEIGHT);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 50 + index * ROW_HEIGHT + 5);
    //lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    //lv_obj_set_flex_align(row,
    //                      LV_FLEX_ALIGN_START,
    //                      LV_FLEX_ALIGN_CENTER,
    //                      LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_all(row, PADDING, 0);
    lv_obj_set_style_radius(row, 6, 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0xf5f5f5), 0);

    /* bus number + square */
    lv_obj_t *square = lv_obj_create(row);
    lv_obj_set_size(square, ROW_HEIGHT - PADDING * 4, ROW_HEIGHT - PADDING * 4);
    lv_color_t color = lv_color_make(
        bus.bus_color[0],
        bus.bus_color[1],
        bus.bus_color[2]);

    lv_obj_set_style_bg_color(square, color, 0);
    lv_obj_set_style_radius(square, 6, 0);
    lv_obj_set_style_pad_all(square, 0, 0);
    lv_obj_set_style_border_width(square, 0, 0);

    //lv_obj_set_flex_flow(square, LV_FLEX_FLOW_COLUMN);
    //lv_obj_set_flex_align(square,
    //                      LV_FLEX_ALIGN_CENTER,
    //                      LV_FLEX_ALIGN_CENTER,
    //                      LV_FLEX_ALIGN_CENTER);

    lv_obj_t *bus_num = lv_label_create(square);
    lv_label_set_text(bus_num, bus.bus_number);
    lv_obj_set_style_text_color(bus_num, lv_color_white(), 0);
    lv_obj_align(bus_num, LV_ALIGN_CENTER, 0, 0);

    /* label bus name*/
    lv_obj_t *bus_name = lv_label_create(row);
    lv_label_set_text(bus_name, bus.bus_name);
    lv_obj_add_style(bus_name, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_align(bus_name, LV_ALIGN_LEFT_MID, ROW_HEIGHT, 0);

    /* label minutes from arrival*/
    lv_obj_t *arrival_time = lv_label_create(row);
    lv_label_set_text(arrival_time, bus.eta);
    lv_obj_add_style(arrival_time, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_align(arrival_time, LV_ALIGN_RIGHT_MID, -PADDING - 15, 0);

    /* tracked - not tracked bus */
    if(bus.delay != -1){    // da capire che valore dare a delay se il bus non è tracked
        lv_obj_t *tracked_bus = lv_obj_create(row);
        lv_obj_set_size(tracked_bus, CIRCLE_RADIUS, CIRCLE_RADIUS);
        lv_obj_set_style_radius(tracked_bus, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(tracked_bus, lv_color_make(0x00, 0x80, 0x00), 0);
        lv_obj_set_style_border_width(tracked_bus, 0, 0);
        lv_obj_align(tracked_bus, LV_ALIGN_RIGHT_MID, -PADDING -3, 0);
    }
    
}

/* server request for bus list */
static void server_timer_cb (lv_timer_t *timer){
    // poi ci sarà parte di richiesta a server per i bus

    //intanto creo i bus a mano
    lv_obj_t *screen = (lv_obj_t *) lv_timer_get_user_data(timer);    
    if(screen == NULL || !lv_obj_is_valid(screen)) return;

    Bus bus_list[5] = {
        {
            .arrival_time = "20:30",
            .bus_color = { 0xff, 0x00, 0x00 },   // red
            .bus_id = 55,
            .bus_name = "Villazzano 3",
            .bus_number = "3",
            .delay = 0,
            .eta = "6'"
        },
        {
            .arrival_time = "20:35",
            .bus_color = { 0x00, 0x80, 0x00 },   // green
            .bus_id = 12,
            .bus_name = "Trento Stazione",
            .bus_number = "5",
            .delay = 2,
            .eta = "11'"
        },
        {
            .arrival_time = "20:40",
            .bus_color = { 0x00, 0x00, 0xff },   // blue
            .bus_id = 78,
            .bus_name = "Povo Università",
            .bus_number = "13",
            .delay = 0,
            .eta = "16'"
        },
        {
            .arrival_time = "20:45",
            .bus_color = { 0xff, 0xa5, 0x00 },   // orange
            .bus_id = 34,
            .bus_name = "Gardolo",
            .bus_number = "7",
            .delay = 5,
            .eta = "21'"
        },
        {
            .arrival_time = "20:50",
            .bus_color = { 0x80, 0x00, 0x80 },   // purple
            .bus_id = 90,
            .bus_name = "Mattarello",
            .bus_number = "A",
            .delay = 0,
            .eta = "26'"
        }
    };

    for(int i = 0; i < 5; i++){
        create_bus_row(bus_list[i], screen, i);
    }
    
    
}


/* create timer for server request */
void create_bus_list(lv_obj_t *scr){
    server_request_time = lv_timer_create(server_timer_cb, 60000, scr);
    server_timer_cb(server_request_time);  
}

/* destroy timer */
void screen_bus_destroy_timer(void) {
    if (server_request_time) {
        lv_timer_del(server_request_time);
        server_request_time = NULL;
    }
}



void ui_load_screen_bus(lv_obj_t *screen) {

    lv_obj_clean(screen);

    /* create dropdown menu */
    lv_obj_t *drop_down_menu = lv_dropdown_create(screen);

    lv_dropdown_set_options(drop_down_menu, bus_options);

    lv_obj_set_width(drop_down_menu, ROW_WIDTH);
    lv_obj_set_height(drop_down_menu, ROW_HEIGHT);
    lv_obj_set_style_radius(drop_down_menu, 14, 0);
    lv_obj_align(drop_down_menu, LV_ALIGN_TOP_MID, 0, 20);
    //lv_obj_set_style_bg_color(drop_down_menu, lv_color_make(0xE0, 0xD4, 0xC6), LV_STATE_DEFAULT);

    // Restore last selection
    lv_dropdown_set_selected(drop_down_menu, last_selected_bus);

    lv_obj_add_event_cb(drop_down_menu,
                        bus_dropdown_event_cb,
                        LV_EVENT_VALUE_CHANGED,
                        drop_down_menu);

    lv_dropdown_get_selected_str(drop_down_menu,
                                 selected_text,
                                 sizeof(selected_text));

    /* create list of busses */
    create_bus_list(screen);

    
}