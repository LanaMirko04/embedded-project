/*!
 * \file            net.cpp
 * \date            2025-11-12
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Network handling module.
 */

/*! Sdrumo Modules */
#include "net.h"
#include "result.h"
/*! ESP-IDF Components */
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
#include <cassert>
#include <atomic>

NetHandler::NetHandler() : event_group(nullptr), retry_count(0U), smartconfig_running(false), came_from_smartconfig(false) {
    this->event_group = nullptr;
    this->ssid.fill(0U);
    this->password.fill(0U);
}

Result NetHandler::load_credentials(void) {
    if (this->ssid.at(0U) != 0U || this->password.at(0U) != 0U) {
        ESP_LOGW(NET_TAG.data(), "Connection information already loaded!");
        return Result::SUCCESS;
    }

    nvs_handle_t nvs_handle;
    int res = nvs_open(NET_NVS_NAMESPACE.data(), NVS_READONLY, &nvs_handle);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    std::size_t ssid_len = NetHandler::SSID_SIZE - 1;
    res = nvs_get_str(nvs_handle, NET_NVS_SSID_KEY.data(), this->ssid.data(), &ssid_len);
    switch (res) {
        case ESP_OK:
            ESP_LOGI(NET_TAG.data(), "SSID read!");
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(NET_TAG.data(), "The SSID is not initialized yet!");
            return Result::NOT_FOUND;

        default:
            result_set_err_msg("Error (%s) reading!", esp_err_to_name(res));
            nvs_close(nvs_handle);
            return Result::UNKNOWN_ERROR;
    }

    std::size_t password_len = NetHandler::PASSWORD_SIZE - 1;
    res = nvs_get_str(nvs_handle, NET_NVS_PASSWORD_KEY.data(), this->password.data(), &password_len);
    switch (res) {
        case ESP_OK:
            ESP_LOGI(NET_TAG.data(), "Password read!");
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(NET_TAG.data(), "The password is not initialized yet!");
            return Result::NOT_FOUND;

        default:
            result_set_err_msg("Error (%s) reading!", esp_err_to_name(res));
            nvs_close(nvs_handle);
            return Result::UNKNOWN_ERROR;
    }

    nvs_close(nvs_handle);

    ESP_LOGI(NET_TAG.data(), "Loaded credentials { SSID=%s,\tPassword=%s }", this->ssid.data(), this->password.data());

    return Result::SUCCESS;
}

Result NetHandler::store_credentials(void) {
    nvs_handle_t nvs_handle;
    int res = nvs_open(NET_NVS_NAMESPACE.data(), NVS_READWRITE, &nvs_handle);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    res = nvs_set_str(nvs_handle, NET_NVS_SSID_KEY.data(), this->ssid.data());
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) saving SSID!", esp_err_to_name(res));
        nvs_close(nvs_handle);
        return Result::IO_ERROR;
    }

    res = nvs_set_str(nvs_handle, NET_NVS_PASSWORD_KEY.data(), this->password.data());
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) saving password!", esp_err_to_name(res));
        nvs_close(nvs_handle);
        return Result::IO_ERROR;
    }

    nvs_close(nvs_handle);
    return Result::SUCCESS;
}

void NetHandler::event_handler(void *arg, esp_event_base_t event_base, std::int32_t event_id, void *event_data) {
    NetHandler &net = NetHandler::get_instance();

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(NET_TAG.data(), "WiFi STA started");

        if (!net.smartconfig_running && net.ssid.at(0U) != 0U && net.password.at(0U) != 0U) {
            esp_wifi_connect();
        } else {
            ESP_LOGI(NET_TAG.data(), "SmartConfig running, skip auto-connect");
        }

        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(net.event_group, NET_CONNECTED_BIT);

        if (net.retry_count < NET_MAX_RETRY) {
            ++net.retry_count;
            ESP_LOGW(NET_TAG.data(), "WiFi disconnected, retry %zu/%zu", net.retry_count.load(), NET_MAX_RETRY);

            esp_wifi_connect();
        } else {
            ESP_LOGE(NET_TAG.data(), "Max retries reached, starting SmartConfig");

            if (!net.smartconfig_running) {
                net.smartconfig_running = true;
                net.came_from_smartconfig = true;

                xTaskCreate(NetHandler::smartconfig_task, "NetHandler::smartconfig_task", 4096, nullptr, 3, nullptr);
            }
        }
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(NET_TAG.data(), "Connected to AP \"%s\"", net.ssid.data());

        net.retry_count = 0;
        xEventGroupSetBits(net.event_group, NET_CONNECTED_BIT);

        /*! Save credentials ONLY if they came from SmartConfig */
        if (net.came_from_smartconfig) {
            ESP_LOGI(NET_TAG.data(), "Storing WiFi credentials");
            net.store_credentials();
            net.came_from_smartconfig = false;
        }
        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(NET_TAG.data(), "SmartConfig scan done");
        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(NET_TAG.data(), "SmartConfig channel found");
        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(NET_TAG.data(), "SmartConfig received credentials");

        auto *evt =
            static_cast<smartconfig_event_got_ssid_pswd_t *>(event_data);

        std::copy(evt->ssid, evt->ssid + SSID_SIZE - 1, net.ssid.begin());
        std::copy(evt->password, evt->password + PASSWORD_SIZE - 1, net.password.begin());

        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            uint8_t rvd[RVD_DATA_SIZE] = {};
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd, sizeof(rvd)));
            ESP_LOGI(NET_TAG.data(), "RVD data received");
        }

        wifi_config_t cfg = {};
        memcpy(cfg.sta.ssid, evt->ssid, sizeof(cfg.sta.ssid));
        memcpy(cfg.sta.password, evt->password, sizeof(cfg.sta.password));

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
        ESP_ERROR_CHECK(esp_wifi_connect());

        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        ESP_LOGI(NET_TAG.data(), "SmartConfig completed");

        esp_smartconfig_stop();
        net.smartconfig_running = false;

        xEventGroupSetBits(net.event_group, NET_ESPTOUCH_DONE_BIT);
        return;
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
            vTaskDelete(nullptr);
        }
    }
}

NetHandler &NetHandler::get_instance(void) {
    static NetHandler instance;
    return instance;
}

void NetHandler::init_connection(void) {
    static bool netif_initialized = false;

    if (!netif_initialized) {
        ESP_ERROR_CHECK(esp_netif_init());
        netif_initialized = true;
    }

    this->event_group = xEventGroupCreate();
    assert(this->event_group);

    this->sta_netif = esp_netif_create_default_wifi_sta();
    assert(this->sta_netif);

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &NetHandler::event_handler, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &NetHandler::event_handler, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &NetHandler::event_handler, nullptr));

    this->retry_count = 0;
    this->smartconfig_running = false;
    this->came_from_smartconfig = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    Result res = this->load_credentials();
    if (res == Result::SUCCESS) {
        wifi_config_t cfg = {};
        memcpy(cfg.sta.ssid, this->ssid.data(), SSID_SIZE - 1);
        memcpy(cfg.sta.password, this->password.data(), PASSWORD_SIZE - 1);
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    } else if (res != Result::NOT_FOUND) {
        ESP_LOGE(NET_TAG.data(), "Failed to load Wi-Fi credentials: %s", result_get_err_msg());
        abort();
    }

    ESP_ERROR_CHECK(esp_wifi_start());
}

void NetHandler::deinit_connection(void) {
    if (this->smartconfig_running) {
        esp_smartconfig_stop();
        this->smartconfig_running = false;
    }

    esp_wifi_disconnect();
    ESP_ERROR_CHECK(esp_wifi_stop());

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &NetHandler::event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &NetHandler::event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &NetHandler::event_handler));

    if (this->sta_netif) {
        esp_netif_destroy(this->sta_netif);
        this->sta_netif = nullptr;
    }

    if (this->event_group) {
        vEventGroupDelete(this->event_group);
        this->event_group = nullptr;
    }

    this->retry_count = 0;
    this->came_from_smartconfig = false;

    ESP_LOGI(NET_TAG.data(), "Network deinitialized successfully");
}
