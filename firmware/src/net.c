/*! Sdrumo Modules */
#include "net.h"
#include "rc.h"
/*! ESP-IDF Libraries */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "nvs.h"
/*! Standard C Libraries */
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

static const int NET_CONNECTED_BIT = BIT0;
static const int NET_ESPTOUCH_DONE_BIT = BIT1;
const char *NET_TAG = "SDRUMO_NET";

static EventGroupHandle_t net_event_group;

static int prv_net_load_credentials(char *ssid, char *pass) {
    if (!ssid || !pass)
        return false;

    nvs_handle_t nvs_handle;
    int res = nvs_open(NET_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return RC_IO_ERR;
    }

    res = nvs_get_str(nvs_handle, NET_NVS_SSID_KEY, ssid, NULL);
    switch (res) {
        case ESP_OK:
            ESP_LOGI(NET_TAG, "SSID read!");
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(NET_TAG, "The SSID is not initialized yet!");
            break;

        default:
            ESP_LOGE(NET_TAG, "Error (%s) reading!", esp_err_to_name(res));
            abort();
    }

    res = nvs_get_str(nvs_handle, NET_NVS_PASSWORD_KEY, pass, NULL);
    switch (res) {
        case ESP_OK:
            ESP_LOGI(NET_TAG, "Password read!");
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(NET_TAG, "The password is not initialized yet!");
            break;

        default:
            ESP_LOGE(NET_TAG, "Error (%s) reading!", esp_err_to_name(res));
            abort();
    }

    //nvs_close(nvs_handle);

    return true;
}

static int prv_net_store_credentials(const char *ssid, const char *pass) {
    if (!ssid || !pass)
        return RC_ERR_INVALID_ARG;

    nvs_handle_t nvs_handle;
    int res = nvs_open(NET_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return RC_IO_ERR;
    }

    res = nvs_set_str(nvs_handle, NET_NVS_SSID_KEY, ssid);
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) saving SSID!", esp_err_to_name(res));
        return RC_IO_ERR;
    }

    res = nvs_set_str(nvs_handle, NET_NVS_PASSWORD_KEY, pass);
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) saving password!", esp_err_to_name(res));
        return RC_IO_ERR;
    }

    nvs_close(nvs_handle);

    return RC_OK;
}

static void prv_net_smartconfig_task(void *args) {
    EventBits_t bits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (true) {
        bits = xEventGroupWaitBits(net_event_group, NET_CONNECTED_BIT | NET_ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (bits & NET_CONNECTED_BIT) {
            ESP_LOGI(NET_TAG, "WiFi Connected to ap");
        }
        if (bits & NET_ESPTOUCH_DONE_BIT) {
            ESP_LOGI(NET_TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

static void prv_net_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xTaskCreate(prv_net_smartconfig_task, "prv_net_smartconfig_task", 4096, NULL, 3, NULL);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(net_event_group, NET_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(net_event_group, NET_CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(NET_TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(NET_TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(NET_TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_cfg;
        uint8_t ssid[NET_SSID_SIZE] = { 0 };
        uint8_t password[NET_PASSWORD_SIZE] = { 0 };
        uint8_t rvd_data[NET_RVD_DATA_SIZE] = { 0 };

        bzero(&wifi_cfg, sizeof(wifi_config_t));
        memcpy(wifi_cfg.sta.ssid, evt->ssid, sizeof(wifi_cfg.sta.ssid));
        memcpy(wifi_cfg.sta.password, evt->password, sizeof(wifi_cfg.sta.password));

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGD(NET_TAG, "SSID:%s", ssid);
        ESP_LOGD(NET_TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(NET_TAG, "RVD_DATA:");
            for (int i = 0; i < NET_RVD_DATA_SIZE; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(net_event_group, NET_ESPTOUCH_DONE_BIT);
    }
}

void net_init(void) {

    ESP_ERROR_CHECK(esp_netif_init());
    net_event_group = xEventGroupCreate();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &prv_net_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &prv_net_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &prv_net_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void net_deinit(void) {
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    esp_netif_destroy(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
    if (net_event_group) {
        vEventGroupDelete(net_event_group);
        net_event_group = NULL;
    }

    ESP_LOGI(NET_TAG, "Network deinitialized successfully.");
}
