#include "screen_bus.h"
#include "api_task.h"
#include "styles.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ROW_WIDTH      270
#define ROW_HEIGHT     32
#define PADDING        2
#define CIRCLE_RADIUS  8
#define BUS_NAME_WIDTH 130

static uint16_t      last_selected_bus = 0;
static DeviceStopList s_stops_cache    = {};
static char           s_dropdown_buf[DEVICE_STOPS_MAX * 65];
static char           selected_text[64];
static lv_timer_t    *server_request_time = NULL;


static const char *bus_destination(const char *name, char *buf, size_t bufsz) {
    const char *p = strstr(name, " - ");
    if (p) {
        const char *last = p;
        const char *next = p;
        while ((next = strstr(next + 3, " - ")) != NULL) last = next;
        strncpy(buf, last + 3, bufsz - 1);
        buf[bufsz - 1] = '\0';
        return buf;
    }
    p = strrchr(name, '/');
    if (p) { strncpy(buf, p + 1, bufsz - 1); buf[bufsz - 1] = '\0'; return buf; }
    p = strrchr(name, '-');
    if (p) { strncpy(buf, p + 1, bufsz - 1); buf[bufsz - 1] = '\0'; return buf; }
    /* space-only names: take last word; if ≤2 chars (line variant), take last two */
    p = strrchr(name, ' ');
    if (p) {
        const char *start = p + 1;
        if (strlen(start) <= 2) {
            const char *q = p - 1;
            while (q > name && *q != ' ') q--;
            start = (*q == ' ') ? q + 1 : name;
        }
        strncpy(buf, start, bufsz - 1);
        buf[bufsz - 1] = '\0';
        return buf;
    }
    return name;
}

static int minutes_remaining(const char *eta) {
    int h = 0, m = 0;
    if (sscanf(eta, "%d:%d", &h, &m) != 2) return 9999;
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    int diff = (h * 60 + m) - (t.tm_hour * 60 + t.tm_min);
    if (diff < -720) diff += 1440;
    return diff;
}

static void bus_dropdown_event_cb(lv_event_t *e) {
    lv_obj_t *dd = lv_event_get_target(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        uint16_t sel = lv_dropdown_get_selected(dd);
        last_selected_bus = sel;
        lv_dropdown_get_selected_str(dd, selected_text, sizeof(selected_text));
        if (sel < (uint16_t)s_stops_cache.count)
            api_task_notify_fetch_bus_stop(s_stops_cache.stops[sel].stop_id);
    }
}


static void create_bus_row(const BusTrip *trip, lv_obj_t *parent, int index) {
    int remain   = minutes_remaining(trip->eta);
    int traced   = (trip->delay >= 0.0f);
    int departed = traced || (remain >= 0 && remain < 60);

    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_width(row, ROW_WIDTH);
    lv_obj_set_height(row, ROW_HEIGHT);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, index * (ROW_HEIGHT + PADDING));
    lv_obj_set_style_pad_all(row, PADDING, 0);
    lv_obj_set_style_radius(row, 6, 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0xf5f5f5), 0);

    lv_obj_t *square = lv_obj_create(row);
    lv_obj_set_size(square, ROW_HEIGHT - PADDING * 4, ROW_HEIGHT - PADDING * 4);
    lv_color_t color = lv_color_hex(trip->bus_color);
    lv_obj_set_style_bg_color(square, color, 0);
    lv_obj_set_style_bg_opa(square, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(square, 6, 0);
    lv_obj_set_style_pad_all(square, 0, 0);
    lv_obj_set_style_border_width(square, 0, 0);

    lv_obj_t *bus_num = lv_label_create(square);
    lv_label_set_text(bus_num, trip->bus_number);
    lv_obj_set_style_text_color(bus_num, lv_color_white(), 0);
    lv_obj_align(bus_num, LV_ALIGN_CENTER, 0, 0);

    char dest_buf[64];
    const char *dest = bus_destination(trip->bus_name, dest_buf, sizeof(dest_buf));

    lv_obj_t *bus_name = lv_label_create(row);
    lv_obj_set_width(bus_name, BUS_NAME_WIDTH);
    lv_label_set_long_mode(bus_name, LV_LABEL_LONG_MODE_DOTS);
    lv_label_set_text(bus_name, dest);
    lv_obj_add_style(bus_name, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_align(bus_name, LV_ALIGN_LEFT_MID, ROW_HEIGHT, 0);

    char remain_buf[12];
    if (remain < 0)
        snprintf(remain_buf, sizeof(remain_buf), "Now");
    else if (remain >= 60)
        snprintf(remain_buf, sizeof(remain_buf), "%s", trip->eta);
    else
        snprintf(remain_buf, sizeof(remain_buf), "%d'", remain);

    lv_obj_t *remain_lbl = lv_label_create(row);
    lv_label_set_text(remain_lbl, remain_buf);
    lv_obj_add_style(remain_lbl, &style_label_16, LV_STATE_DEFAULT);
    lv_obj_align(remain_lbl, LV_ALIGN_RIGHT_MID, -(PADDING + CIRCLE_RADIUS + 4), 0);

    if (departed) {
        lv_obj_t *dot = lv_obj_create(row);
        lv_obj_set_size(dot, CIRCLE_RADIUS, CIRCLE_RADIUS);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_color_t dot_color;
        if (traced)
            dot_color = lv_color_make(0x00, 0xC0, 0x00);
        else
            dot_color = lv_color_make(0xFF, 0x80, 0x00);
        lv_obj_set_style_bg_color(dot, dot_color, 0);
        lv_obj_align(dot, LV_ALIGN_RIGHT_MID, -PADDING, 0);
    }
}


static void server_timer_cb(lv_timer_t *timer) {
    lv_obj_t *container = (lv_obj_t *)lv_timer_get_user_data(timer);
    if (container == NULL || !lv_obj_is_valid(container)) return;

    lv_obj_clean(container);

    if (api_task_is_server_down()) {
        lv_obj_t *lbl = lv_label_create(container);
        lv_label_set_text(lbl, "Server unavailable");
        lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 40);
        lv_obj_add_style(lbl, &style_subtitle, LV_STATE_DEFAULT);
        return;
    }

    BusModel model;
    if (!api_task_copy_bus_model(&model)) {
        lv_obj_t *lbl = lv_label_create(container);
        lv_label_set_text(lbl, "Loading...");
        lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 40);
        lv_obj_add_style(lbl, &style_subtitle, LV_STATE_DEFAULT);
        return;
    }

    if (model.count == 0) {
        lv_obj_t *lbl1 = lv_label_create(container);
        lv_label_set_text(lbl1, "No buses scheduled");
        lv_obj_align(lbl1, LV_ALIGN_TOP_MID, 0, 40);
        lv_obj_add_style(lbl1, &style_label_16, LV_STATE_DEFAULT);
        lv_obj_t *lbl2 = lv_label_create(container);
        lv_label_set_text(lbl2, "at this stop");
        lv_obj_align_to(lbl2, lbl1, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
        lv_obj_add_style(lbl2, &style_label_16, LV_STATE_DEFAULT);
        return;
    }

    int limit = model.count < 5 ? model.count : 5;
    for (int i = 0; i < limit; i++)
        create_bus_row(&model.trips[i], container, i);
}


static void create_bus_list(lv_obj_t *scr) {
    lv_obj_t *container = lv_obj_create(scr);
    lv_obj_set_size(container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(container, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_set_style_bg_opa(container, 0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(container, 0, 0);

    server_request_time = lv_timer_create(server_timer_cb, 60000, container);
    server_timer_cb(server_request_time);
}


void screen_bus_destroy_timer(void) {
    if (server_request_time) {
        lv_timer_del(server_request_time);
        server_request_time = NULL;
    }
}


void ui_load_screen_bus(lv_obj_t *screen) {
    lv_obj_clean(screen);

    api_task_copy_stops(&s_stops_cache);

    s_dropdown_buf[0] = '\0';
    if (s_stops_cache.count == 0) {
        strncpy(s_dropdown_buf, "No stops configured", sizeof(s_dropdown_buf) - 1);
    } else {
        for (int i = 0; i < s_stops_cache.count; i++) {
            if (i > 0)
                strncat(s_dropdown_buf, "\n",
                        sizeof(s_dropdown_buf) - strlen(s_dropdown_buf) - 1);
            strncat(s_dropdown_buf, s_stops_cache.stops[i].name,
                    sizeof(s_dropdown_buf) - strlen(s_dropdown_buf) - 1);
        }
    }

    if (last_selected_bus >= (uint16_t)s_stops_cache.count)
        last_selected_bus = 0;

    lv_obj_t *drop_down_menu = lv_dropdown_create(screen);
    lv_dropdown_set_options(drop_down_menu, s_dropdown_buf);
    lv_obj_set_width(drop_down_menu, ROW_WIDTH);
    lv_obj_set_height(drop_down_menu, ROW_HEIGHT);
    lv_obj_set_style_radius(drop_down_menu, 14, 0);
    lv_obj_align(drop_down_menu, LV_ALIGN_TOP_MID, 0, 20);

    lv_dropdown_set_selected(drop_down_menu, last_selected_bus);
    lv_obj_add_event_cb(drop_down_menu, bus_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_dropdown_get_selected_str(drop_down_menu, selected_text, sizeof(selected_text));

    create_bus_list(screen);
    api_task_notify_fetch_bus();
}
