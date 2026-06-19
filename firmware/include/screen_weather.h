#pragma once

#include <esp_err.h>
#include <esp_lvgl_port.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_system.h>

#include "styles.h"
#include "utiles.h"


/* Dall'API bisogna con uno switch case trasformare le variabili "*_icon" 
 * che saranno in numero con il corrispondente file. Il numero associato 
 * ad ogni file è:
 * sunny --> 1
 * partly_cloudy --> 2
 * cloudy --> 3
 * rainy --> 4
 * storm --> 5
 * snow --> 6
 * fog --> 7
 */


typedef struct {
    char city[32];
    char date[32];

    char today_temp[6];
    char today_weath[32];
    lv_image_dsc_t today_weath_icon;

    char today_hummidity[5];
    char today_rain[5];
    char today_wind[12];

    char day1[4];
    char weather_1[32];
    lv_image_dsc_t weather_1_icon;
    char temp_1 [6];

    char day2[4];
    char weather_2[32];
    lv_image_dsc_t weather_2_icon;
    char temp_2[6];

    char day3[4];
    char weather_3[32];
    lv_image_dsc_t weather_3_icon;
    char temp_3[6];

} Weather;



void ui_load_screen_weather(lv_obj_t *screen);
