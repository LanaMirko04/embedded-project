/*!
 * \file            main.h
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           CYD main program.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

// Your display driver header
#include "display.h"

// Generated UI header (from XML)
#include "ui_1.h"

static void lv_tick_task(void *arg)
{
    (void)arg;
    while (1) {
        lv_tick_inc(1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static void lvgl_task(void *arg)
{
    (void)arg;
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

extern "C" void app_main(void)
{
    // 1️⃣ Init LVGL core
    lv_init();

    // 2️⃣ Init display + register driver
    display_init();   // YOU must provide this

    // 3️⃣ Start LVGL tick (1 ms)
    xTaskCreate(
        lv_tick_task,
        "lv_tick",
        2048,
        NULL,
        configMAX_PRIORITIES - 1,
        NULL
    );

    // 4️⃣ Start LVGL handler task
    xTaskCreate(
        lvgl_task,
        "lvgl",
        4096,
        NULL,
        5,
        NULL
    );

    // 5️⃣ Init UI generated from XML
    ui_init();

    // 6️⃣ Load first screen
    lv_scr_load(sdrumo_opening_screen);   // name depends on your generator
}
