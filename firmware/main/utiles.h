#pragma once

#include <stdio.h>

typedef enum {
    SCREEN_BOOT,
    SCREEN_WIFI, 
    SCREEN_CLOCK,
    SCREEN_BUS,
    SCREEN_WEATHER
} Screens;

extern Screens screen_type;