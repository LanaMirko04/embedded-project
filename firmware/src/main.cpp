/*!
 * \file            main.cpp
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           CYD main program.
 */

#include "ui/screen/splash-screen.h"
#include "lvgl.h"

extern "C" void app_main(void) {
    // Initialize LVGL (porco dio non possiamo usare smartdisplay dio assassino)

    SplashScreen splash;
    if (splash.create() != Result::SUCCESS) {
        return -1;
    }

    // Show it
    splash.on_enter();
    lv_tick_task();

    return 0;
}
