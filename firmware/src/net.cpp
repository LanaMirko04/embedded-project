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
#include "config.h"
#include "freertos/projdefs.h"
#include "result.h"
#include <time.h>
/*! ESP-IDF Components */
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_err.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
/*! Standard Library */
#include <cassert>
#include <cstring>
#include <string_view>

constexpr std::size_t NET_SSID_SIZE = 33U;
constexpr std::size_t NET_PASSWORD_SIZE = 65U;
constexpr std::size_t NET_SNTP_WAIT_MS = 10000U;

static const char *NET_TAG = "NET";

void NetHandler::start_smartconfig(void) {
    ESP_LOGD(NET_TAG, "Now executing %s", __func__);
    if (this->smartconfig_running) {
        return;
    }

    this->smartconfig_running = true;
    this->came_from_smartconfig = true;

    if (xTaskCreate(NetHandler::smartconfig_task, "smartconfig", 4096, nullptr, 3, &this->sc_task_handle) != pdPASS) {
        ESP_LOGE(NET_TAG, "Failed to create SmartConfig task");
        this->smartconfig_running = false;
        this->came_from_smartconfig = false;
        this->sc_task_handle = nullptr;
    }
}

void NetHandler::event_handler(void *arg, esp_event_base_t event_base, std::int32_t event_id, void *event_data) {
    ESP_LOGD(NET_TAG, "Now executing %s", __func__);

    static NetHandler &net = NetHandler::get_instance();
    static Config &config = Config::get_instance();

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(NET_TAG, "WiFi STA started");

        if (net.smartconfig_running) {
            return;
        }

        if (config.get_ssid()[0U] != '\0' && config.get_password()[0U] != '\0') {
            esp_wifi_connect();
        } else {
            ESP_LOGW(NET_TAG, "No credentials, starting SmartConfig immediately");
            net.start_smartconfig();
        }

        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(net.event_group, NET_CONNECTED_BIT);

        if (net.retry_count < NET_MAX_RETRY) {
            ++net.retry_count;
            ESP_LOGW(NET_TAG, "WiFi disconnected, retry %zu/%zu", net.retry_count.load(), NET_MAX_RETRY);

            esp_wifi_connect();
        } else {
            ESP_LOGE(NET_TAG, "Max retries reached, starting SmartConfig");
            net.start_smartconfig();
        }
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(NET_TAG, "Connected to AP \"%s\"", config.get_ssid());

        net.connected = true;
        net.retry_count = 0;
        xEventGroupSetBits(net.event_group, NET_CONNECTED_BIT);

        /*! Save credentials ONLY if they came from SmartConfig */
        if (net.came_from_smartconfig) {
            ESP_LOGI(NET_TAG, "Storing WiFi credentials");
            net.came_from_smartconfig = false;
        }
        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(NET_TAG, "SmartConfig scan done");
        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(NET_TAG, "SmartConfig channel found");
        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(NET_TAG, "SmartConfig received credentials");

        auto *evt =
            static_cast<smartconfig_event_got_ssid_pswd_t *>(event_data);

        char ssid[33U] = { 0 };
        char password[65U] = { 0 };
        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));

        ESP_LOGD(NET_TAG, "SmartConfig SSID: \"%s\"", ssid);
        ESP_LOGD(NET_TAG, "SmartConfig password: \"%s\"", password);

        std::size_t ssid_len = strnlen(ssid, sizeof(ssid) - 1);
        std::size_t password_len = strnlen(password, sizeof(password) - 1);
        Result cr = config.set_credentials(std::string_view(ssid, ssid_len),
                                            std::string_view(password, password_len));
        if (cr != Result::SUCCESS) {
            ESP_LOGE(NET_TAG, "set_credentials failed");
        } else if (config.store() != Result::SUCCESS) {
            ESP_LOGE(NET_TAG, "Config store failed");
        }

        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            uint8_t rvd[RVD_DATA_SIZE] = {};
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd, sizeof(rvd)));
            ESP_LOGI(NET_TAG, "RVD data received");
        }

        wifi_config_t cfg = {};
        memcpy(cfg.sta.ssid, evt->ssid, sizeof(cfg.sta.ssid));
        memcpy(cfg.sta.password, evt->password, sizeof(cfg.sta.password));

        net.retry_count = 0;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
        ESP_ERROR_CHECK(esp_wifi_connect());

        return;
    }

    if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        ESP_LOGI(NET_TAG, "SmartConfig completed");

        xEventGroupSetBits(net.event_group, NET_ESPTOUCH_DONE_BIT);
        return;
    }
}

void NetHandler::smartconfig_task(void *args) {
    ESP_LOGD(NET_TAG, "Now executing %s", __func__);

    EventBits_t bits;
    NetHandler &net_handler = NetHandler::get_instance();

    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_V2));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (true) {
        bits = xEventGroupWaitBits(net_handler.event_group, NET_CONNECTED_BIT | NET_ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (bits & NET_ESPTOUCH_DONE_BIT) {
            ESP_LOGI(NET_TAG, "Smartconfig over");
            esp_smartconfig_stop();
            net_handler.smartconfig_running = false;
            net_handler.sc_task_handle = nullptr;
            vTaskDelete(nullptr);
        }
    }
}

NetHandler &NetHandler::get_instance(void) {
    ESP_LOGD(NET_TAG, "Now executing %s", __func__);

    static NetHandler instance;
    return instance;
}

void NetHandler::init_connection(void) {
    esp_log_level_set(NET_TAG, ESP_LOG_DEBUG);

    static bool netif_initialized = false;
    Config &config = Config::get_instance();

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

    if (config.get_ssid()[0U] && config.get_password()[0U]) {
        wifi_config_t cfg = {};
        memcpy(cfg.sta.ssid, config.get_ssid(), sizeof(cfg.sta.ssid) - 1);
        cfg.sta.ssid[sizeof(cfg.sta.ssid) - 1] = '\0';
        memcpy(cfg.sta.password, config.get_password(), sizeof(cfg.sta.password) - 1);
        cfg.sta.password[sizeof(cfg.sta.password) - 1] = '\0';
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    } else {
        ESP_LOGE(NET_TAG, "Failed to load Wi-Fi credentials: %s", result_get_err_msg());
    }

    ESP_ERROR_CHECK(esp_wifi_start());
}

Result NetHandler::sync_time(void) {
    ESP_LOGD(NET_TAG, "Now executing %s", __func__);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("it.pool.ntp.org");
    esp_netif_sntp_init(&config);

    esp_err_t res = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(NET_SNTP_WAIT_MS));
    if ( res != ESP_OK) {
        result_set_err_msg("SNTP synchronization failed [%s]", esp_err_to_name(res));
        return Result::SYNC_FAILED;
    }

    return Result::SUCCESS;
}

void NetHandler::deinit_connection(void) {
    ESP_LOGD(NET_TAG, "Now executing %s", __func__);

    if (this->smartconfig_running) {
        esp_smartconfig_stop();
        this->smartconfig_running = false;
    }

    if (this->sc_task_handle != nullptr) {
        vTaskDelete(this->sc_task_handle);
        this->sc_task_handle = nullptr;
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
    this->connected = false;
    this->sc_task_handle = nullptr;

    ESP_LOGI(NET_TAG, "Network deinitialized successfully");
}
