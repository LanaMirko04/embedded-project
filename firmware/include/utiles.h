#pragma once

#include <stdio.h>

typedef enum {
    SCREEN_BOOT,
    SCREEN_WIFI, 
    SCREEN_CLOCK,
    SCREEN_BUS,
    SCREEN_WEATHER,
    SCREEN_NUMBER
} Screens;

extern Screens present_screen_type;
extern Screens next_screen_type;
