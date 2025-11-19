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

static EventGroupHandle_t net_wifi_event_group;

static void net_wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    static int retry_num = 0;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_num < NET_WIFI_CONN_RETRY_NUM) {
            esp_wifi_connect();
            retry_num++;
            ESP_LOGI(NET_TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(net_wifi_event_group, NET_WIFI_FAIL_BIT);
        }
        ESP_LOGI(NET_TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(NET_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_num = 0;
        xEventGroupSetBits(net_wifi_event_group, NET_WIFI_CONNECTED_BIT);
    }
}

int net_init(const char *ssid, const char *pass) {
    net_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &net_wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &net_wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = { 0 },
            .password = { 0 },
            .threshold.authmode = WIFI_AUTH_OPEN,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = "",
        },
    };

    if (!strncpy((char *)wifi_cfg.sta.ssid, ssid, NET_SSID_MAX_LEN)) {
        rc_set_err_msg("An error occurred while copying Wi-Fi's SSID");
        return RC_FAIL;
    }

    if (!strncpy((char *)wifi_cfg.sta.password, pass, NET_PASS_MAX_LEN)) {
        rc_set_err_msg("An error occurred while copying Wi-Fi's password");
        return RC_FAIL; /*! TODO: define a return code */
    }

    ESP_LOGI(NET_TAG, "SSID=%s\tPASS=%s", wifi_cfg.sta.ssid, wifi_cfg.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(net_wifi_event_group,
                                           NET_WIFI_CONNECTED_BIT | NET_WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & NET_WIFI_CONNECTED_BIT) {
        ESP_LOGI(NET_TAG, "connected to ap SSID:%s password:%s", ssid, pass);
        return RC_OK;
    } else if (bits & NET_WIFI_FAIL_BIT) {
        ESP_LOGI(NET_TAG, "Failed to connect to SSID:%s, password:%s", ssid, pass);
        return RC_FAIL;
    } else {
        ESP_LOGE(NET_TAG, "UNEXPECTED EVENT");
        return RC_FAIL;
    }
}

// int net_disconnect(void) {
//     return RC_OK;
// }
