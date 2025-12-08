/*!
 * \file            net.c
 * \date            2025-11-12
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Network handling module.
 */

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
/*! Standard Library */
#include <cstdint>
#include <iterator>

NetHandler::NetHandler() {
}

int NetHandler::load_connection_info(void) {
    nvs_handle_t nvs_handle;
    int res = nvs_open(NET_NVS_NAMESPACE.data(), NVS_READONLY, &nvs_handle);
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return RC_ERR_IO_OPERATION;
    }

    std::size_t ssid_len = NetHandler::SSID_SIZE - 1;
    res = nvs_get_str(nvs_handle, NET_NVS_SSID_KEY.data(), this->ssid.data(), &ssid_len);
    switch (res) {
        case ESP_OK:
            ESP_LOGI(NET_TAG.data(), "SSID read!");
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(NET_TAG.data(), "The SSID is not initialized yet!");
            return RC_NET_NO_STORED_CONN;

        default:
            rc_set_err_msg("Error (%s) reading!", esp_err_to_name(res));
            nvs_close(nvs_handle);
            return RC_FAIL;
    }

    std::size_t password_len = NetHandler::PASSWORD_SIZE - 1;
    res = nvs_get_str(nvs_handle, NET_NVS_PASSWORD_KEY.data(), this->password.data(), &password_len);
    switch (res) {
        case ESP_OK:
            ESP_LOGI(NET_TAG.data(), "Password read!");
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(NET_TAG.data(), "The password is not initialized yet!");
            return RC_NET_NO_STORED_CONN;

        default:
            rc_set_err_msg("Error (%s) reading!", esp_err_to_name(res));
            nvs_close(nvs_handle);
            return RC_FAIL;
    }

    nvs_close(nvs_handle);
    return RC_OK;
}

int NetHandler::store_connection_info(void) {
    nvs_handle_t nvs_handle;
    int res = nvs_open(NET_NVS_NAMESPACE.data(), NVS_READWRITE, &nvs_handle);
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return RC_ERR_IO_OPERATION;
    }

    res = nvs_set_str(nvs_handle, NET_NVS_SSID_KEY.data(), this->ssid.data());
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) saving SSID!", esp_err_to_name(res));
        nvs_close(nvs_handle);
        return RC_ERR_IO_OPERATION;
    }

    res = nvs_set_str(nvs_handle, NET_NVS_PASSWORD_KEY.data(), this->password.data());
    if (res != ESP_OK) {
        rc_set_err_msg("Error (%s) saving password!", esp_err_to_name(res));
        nvs_close(nvs_handle);
        return RC_ERR_IO_OPERATION;
    }

    nvs_close(nvs_handle);
    return RC_OK;
}

void NetHandler::event_handler(void *arg, esp_event_base_t event_base, std::int32_t event_id, void *event_data) {
    static std::size_t retry_count = 0U;
    NetHandler &net_handler = NetHandler::get_instance();

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        retry_count = 0U;
        int res = net_handler.load_connection_info();
        if (res == RC_NET_NO_STORED_CONN) {
            xTaskCreate(NetHandler::smartconfig_task, "NetHandler::smartconfig_task", 4096, NULL, 3, NULL);
        } else if (res == RC_OK) {
            ESP_LOGI(NET_TAG.data(), "Using stored WiFi credentials");

            wifi_config_t wifi_cfg = {};
            memcpy(wifi_cfg.sta.ssid, net_handler.ssid.data(), net_handler.ssid.size());
            memcpy(wifi_cfg.sta.password, net_handler.password.data(), net_handler.password.size());

            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
            ESP_ERROR_CHECK(esp_wifi_connect());
        } else {
            ESP_LOGE(NET_TAG.data(), "An unexpected error occurred! - %s", rc_get_err_msg());
            abort();
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_count < NET_MAX_RETRY) {
            ESP_LOGW(NET_TAG.data(), "Connection failed, retry %zu/%zu", ++retry_count, NET_MAX_RETRY);
            esp_wifi_connect();
        } else {
            ESP_LOGI(NET_TAG.data(), "Connection failed! Starting smartconfig...");
            xTaskCreate(NetHandler::smartconfig_task, "NetHandler::smartconfig_task", 4096, NULL, 3, NULL);
        }

        xEventGroupClearBits(net_handler.event_group, NET_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(NET_TAG.data(), "Connected to %d!", net_handler.ssid.data());
        retry_count = 0;
        xEventGroupSetBits(net_handler.event_group, NET_CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(NET_TAG.data(), "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(NET_TAG.data(), "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(NET_TAG.data(), "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        std::copy(evt->ssid, evt->ssid + NetHandler::SSID_SIZE - 1, net_handler.ssid.begin());
        std::copy(evt->password, evt->password + NetHandler::PASSWORD_SIZE - 1, net_handler.password.begin());

        // ESP_LOGD(NET_TAG.data(), "SSID:%s", ssid);
        // ESP_LOGD(NET_TAG.data(), "PASSWORD:%s", password);
        uint8_t rvd_data[NetHandler::RVD_DATA_SIZE] = { 0 };
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(NET_TAG.data(), "RVD_DATA:");
            for (int i = 0; i < NetHandler::RVD_DATA_SIZE; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        wifi_config_t wifi_cfg = {};
        memcpy(wifi_cfg.sta.ssid, evt->ssid, sizeof(wifi_cfg.sta.ssid));
        memcpy(wifi_cfg.sta.password, evt->password, sizeof(wifi_cfg.sta.password));

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
        ESP_ERROR_CHECK(esp_wifi_connect());
        net_handler.store_connection_info(); /*! I should check it, but trust me: it works, DON'T TOUCH IT ~Mirko */
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(net_handler.event_group, NET_ESPTOUCH_DONE_BIT);
    }
}

void NetHandler::smartconfig_task(void *args) {
    EventBits_t bits;
    NetHandler &net_handler = NetHandler::get_instance();

    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (true) {
        bits = xEventGroupWaitBits(net_handler.event_group, NET_CONNECTED_BIT | NET_ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (bits & NET_CONNECTED_BIT) {
            ESP_LOGI(NET_TAG.data(), "WiFi Connected to ap");
        }
        if (bits & NET_ESPTOUCH_DONE_BIT) {
            ESP_LOGI(NET_TAG.data(), "Smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

NetHandler &NetHandler::get_instance(void) {
    static NetHandler instance;
    return instance;
}

void NetHandler::init_connection(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    this->event_group = xEventGroupCreate();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void NetHandler::deinit_connection(void) {
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    esp_netif_destroy(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
    if (this->event_group) {
        vEventGroupDelete(this->event_group);
        this->event_group = NULL;
    }

    ESP_LOGI(NET_TAG.data(), "Network deinitialized successfully.");
}
