#pragma once

#include <lvgl.h>
#include "styles.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_load_screen_pairing(lv_obj_t *screen);
void screen_pairing_destroy_timer(void);

#ifdef __cplusplus
}
#endif
