#include "net.h"
#include "rc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <string.h>

int net_connect(const char *ssid, const char *pass) {
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = { 0 },
            .password = { 0 },
            .threshold.authmode = WIFI_AUTH_OPEN,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = "",
        },
    };

    if (strncpy((char *)wifi_cfg.sta.ssid, ssid, NET_SSID_MAX_LEN)) {
        rc_set_err_msg("An error occurred while copying Wi-Fi's SSID");
        return RC_FAIL;
    }

    if (strncpy((char *)wifi_cfg.sta.password, pass, NET_PASS_MAX_LEN)) {
        rc_set_err_msg("An error occurred while copying Wi-Fi's password");
        return RC_FAIL; /*! TODO: define a return code */
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    return RC_OK;
}

int net_disconnect(void) {
    return RC_OK;
}
