#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    SCREEN_BOOT,
    SCREEN_WIFI, 
    SCREEN_CLOCK,
    SCREEN_BUS,
    SCREEN_ALARM,
    SCREEN_WEATHER,
    SCREEN_NUMBER
} Screens;

extern bool change_status;

extern Screens present_screen_type;
extern Screens next_screen_type;

