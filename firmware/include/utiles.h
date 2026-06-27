#pragma once

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SCREEN_CLOCK,
    SCREEN_BUS,
    SCREEN_WEATHER,
    SCREEN_NUMBER,
    SCREEN_BOOT,
    SCREEN_WIFI,
    SCREEN_ALARM,
    SCREEN_TIMER,
    SCREEN_PAIRING,
} Screens;

extern bool change_status;

extern Screens present_screen_type;
extern Screens next_screen_type;

#ifdef __cplusplus
}
#endif

