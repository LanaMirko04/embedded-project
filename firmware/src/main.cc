/*!
 * \file            main.h
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           CYD main program.
 */

/*! Sdrumo Modules */
#include "net.h"
/*! ESP-IDF Libraries */
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *MAIN_TAG = "SDRUMO_MAIN";

static void prv_main_init(void);

extern "C" void app_main(void) {
    prv_main_init();

    while (1)
        ;
}

static void prv_main_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    net_init();
}
