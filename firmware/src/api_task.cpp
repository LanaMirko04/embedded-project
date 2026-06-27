/*!
 * \file            api_task.cpp
 * \brief           Background FreeRTOS task for polling the Sdrumo API.
 */

#include <cstring>

#include "api_task.h"
#include "api_client.h"
#include "config.h"
#include "result.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static constexpr char TAG[] = "api_task";

static constexpr TickType_t INTERVAL_STATUS  = pdMS_TO_TICKS(30UL * 1000UL);
static constexpr TickType_t INTERVAL_BUS     = pdMS_TO_TICKS(60UL * 1000UL);
static constexpr TickType_t INTERVAL_WEATHER = pdMS_TO_TICKS(30UL * 60UL * 1000UL);
static constexpr TickType_t INTERVAL_CONFIG  = pdMS_TO_TICKS(5UL * 60UL * 1000UL);

static constexpr uint32_t NOTIFY_BUS_FETCH        = (1U << 0U);
static constexpr uint32_t NOTIFY_BUS_STOP_CHANGED = (1U << 1U);

static TaskHandle_t      s_handle     = nullptr;
static SemaphoreHandle_t s_data_mutex = nullptr;

static BusModel      s_bus     = {};
static Weather       s_weather = {};
static DeviceStopList s_stops  = {};
static bool s_bus_fresh     = false;
static bool s_weather_fresh = false;
static bool s_stops_fresh   = false;
static bool s_server_down   = false;
static bool s_fetch_stops   = true;
static int  s_stop_id       = 0;

static void api_poll_task(void *) {
    ApiClient &api = ApiClient::get_instance();

    TickType_t last_status  = xTaskGetTickCount() - INTERVAL_STATUS;
    TickType_t last_bus     = xTaskGetTickCount() - INTERVAL_BUS;
    TickType_t last_weather = xTaskGetTickCount() - INTERVAL_WEATHER;
    TickType_t last_config  = xTaskGetTickCount() - INTERVAL_CONFIG;

    for (;;) {
        TickType_t now = xTaskGetTickCount();

        TickType_t since_s = now - last_status;
        TickType_t since_b = now - last_bus;
        TickType_t since_w = now - last_weather;
        TickType_t since_c = now - last_config;

        TickType_t wait_s = (since_s >= INTERVAL_STATUS)  ? 0 : INTERVAL_STATUS  - since_s;
        TickType_t wait_b = (since_b >= INTERVAL_BUS)     ? 0 : INTERVAL_BUS     - since_b;
        TickType_t wait_w = (since_w >= INTERVAL_WEATHER) ? 0 : INTERVAL_WEATHER - since_w;
        TickType_t wait_c = (since_c >= INTERVAL_CONFIG)  ? 0 : INTERVAL_CONFIG  - since_c;

        TickType_t wait = wait_s;
        if (wait_b < wait) wait = wait_b;
        if (wait_w < wait) wait = wait_w;
        if (wait_c < wait) wait = wait_c;
        if (wait == 0) wait = 1;

        uint32_t notif = 0;
        xTaskNotifyWait(0, UINT32_MAX, &notif, wait);

        now = xTaskGetTickCount();

        xSemaphoreTake(s_data_mutex, portMAX_DELAY);
        bool server_down = s_server_down;
        xSemaphoreGive(s_data_mutex);

        if ((TickType_t)(now - last_status) >= INTERVAL_STATUS) {
            Result r = api.status();
            bool down = (r != Result::API_STATUS_OK);
            xSemaphoreTake(s_data_mutex, portMAX_DELAY);
            s_server_down = down;
            xSemaphoreGive(s_data_mutex);
            last_status = now;
            ESP_LOGI(TAG, "status: %s", down ? "DOWN" : "OK");
        }

        if (s_fetch_stops && !server_down) {
            DeviceStopList tmp = {};
            Result r = api.fetch_device_stops(tmp);
            if (r == Result::SUCCESS) {
                xSemaphoreTake(s_data_mutex, portMAX_DELAY);
                s_stops       = tmp;
                s_stops_fresh = true;
                xSemaphoreGive(s_data_mutex);
                s_fetch_stops = false;
                if (s_stop_id == 0 && tmp.count > 0) {
                    s_stop_id = tmp.stops[0].stop_id;
                    last_bus  = xTaskGetTickCount() - INTERVAL_BUS;
                }
                Config &cfg = Config::get_instance();
                cfg.set_stops(tmp);
                cfg.store();
                ESP_LOGI(TAG, "stops updated: %d stops", tmp.count);
            } else {
                ESP_LOGW(TAG, "fetch_device_stops failed: %s", result_to_str(r));
            }
        }

        if ((notif & (NOTIFY_BUS_FETCH | NOTIFY_BUS_STOP_CHANGED)) ||
            (TickType_t)(now - last_bus) >= INTERVAL_BUS) {

            if (notif & NOTIFY_BUS_STOP_CHANGED)
                last_bus = now - INTERVAL_BUS;

            if (!server_down) {
                BusModel tmp = {};
                xSemaphoreTake(s_data_mutex, portMAX_DELAY);
                int stop_id = s_stop_id;
                xSemaphoreGive(s_data_mutex);
                Result r = api.fetch_trips(tmp, stop_id);
                if (r == Result::SUCCESS) {
                    xSemaphoreTake(s_data_mutex, portMAX_DELAY);
                    s_bus       = tmp;
                    s_bus_fresh = true;
                    xSemaphoreGive(s_data_mutex);
                    ESP_LOGI(TAG, "bus updated: %d trips (stop_id=%d)", tmp.count, stop_id);
                } else {
                    ESP_LOGW(TAG, "fetch_trips failed: %s", result_to_str(r));
                }
            }
            last_bus = now;
        }

        if ((TickType_t)(now - last_weather) >= INTERVAL_WEATHER) {
            if (!server_down) {
                Weather tmp = {};
                Result r = api.fetch_weather(tmp);
                if (r == Result::SUCCESS) {
                    xSemaphoreTake(s_data_mutex, portMAX_DELAY);
                    s_weather       = tmp;
                    s_weather_fresh = true;
                    xSemaphoreGive(s_data_mutex);
                    ESP_LOGI(TAG, "weather updated: %s", tmp.city);
                } else {
                    ESP_LOGW(TAG, "fetch_weather failed: %s", result_to_str(r));
                }
            }
            last_weather = now;
        }

        if ((TickType_t)(now - last_config) >= INTERVAL_CONFIG) {
            if (!server_down) {
                uint32_t srv_cfg_rev = 0;
                int srv_stop_id = 0;
                Result r = api.fetch_device_config(srv_cfg_rev, srv_stop_id);
                if (r == Result::SUCCESS) {
                    Config &cfg = Config::get_instance();
                    uint32_t local_rev = cfg.get_cfg_rev();
                    if (srv_cfg_rev != local_rev) {
                        ESP_LOGI(TAG, "config changed: cfg_rev %lu→%lu stop_id=%d",
                                 (unsigned long)local_rev, (unsigned long)srv_cfg_rev, srv_stop_id);
                        cfg.set_cfg_rev(srv_cfg_rev);
                        cfg.store();
                        if (srv_stop_id > 0 && srv_stop_id != s_stop_id) {
                            s_stop_id = srv_stop_id;
                            last_bus  = now - INTERVAL_BUS;
                        }
                        s_fetch_stops = true;
                    } else {
                        ESP_LOGD(TAG, "config up to date (cfg_rev=%lu)", (unsigned long)srv_cfg_rev);
                    }
                } else {
                    ESP_LOGW(TAG, "fetch_device_config failed: %s", result_to_str(r));
                }
            }
            last_config = now;
        }
    }
}

extern "C" {

void api_task_start(void) {
    if (s_handle != nullptr) return;
    s_data_mutex = xSemaphoreCreateMutex();
    configASSERT(s_data_mutex);

    Config &cfg = Config::get_instance();
    DeviceStopList cached = cfg.get_stops();
    if (cached.count > 0) {
        s_stops       = cached;
        s_stops_fresh = true;
        s_fetch_stops = false;
    }
    int saved_stop = cfg.get_stop_id();
    if (saved_stop > 0) {
        s_stop_id = saved_stop;
    } else if (cached.count > 0) {
        s_stop_id = cached.stops[0].stop_id;
    }

    BaseType_t r = xTaskCreate(api_poll_task, "api_task", 6144, nullptr, 2, &s_handle);
    configASSERT(r == pdPASS);
    ESP_LOGI(TAG, "started (cached stops=%d stop_id=%d)", cached.count, s_stop_id);
}

void api_task_notify_fetch_bus(void) {
    if (s_handle) {
        xTaskNotify(s_handle, NOTIFY_BUS_FETCH, eSetBits);
    }
}

void api_task_notify_fetch_bus_stop(int stop_id) {
    xSemaphoreTake(s_data_mutex, portMAX_DELAY);
    s_stop_id = stop_id;
    xSemaphoreGive(s_data_mutex);
    Config &cfg = Config::get_instance();
    cfg.set_stop_id(stop_id);
    cfg.store();
    if (s_handle) {
        xTaskNotify(s_handle, NOTIFY_BUS_STOP_CHANGED, eSetBits);
    }
}

bool api_task_copy_bus_model(BusModel *out) {
    if (!out || !s_data_mutex) return false;
    xSemaphoreTake(s_data_mutex, portMAX_DELAY);
    *out = s_bus;
    bool fresh = s_bus_fresh;
    xSemaphoreGive(s_data_mutex);
    return fresh;
}

bool api_task_copy_weather(Weather *out) {
    if (!out || !s_data_mutex) return false;
    xSemaphoreTake(s_data_mutex, portMAX_DELAY);
    *out = s_weather;
    bool fresh = s_weather_fresh;
    xSemaphoreGive(s_data_mutex);
    return fresh;
}

bool api_task_is_server_down(void) {
    if (!s_data_mutex) return false;
    xSemaphoreTake(s_data_mutex, portMAX_DELAY);
    bool val = s_server_down;
    xSemaphoreGive(s_data_mutex);
    return val;
}

bool api_task_copy_stops(DeviceStopList *out) {
    if (!out || !s_data_mutex) return false;
    xSemaphoreTake(s_data_mutex, portMAX_DELAY);
    *out = s_stops;
    bool fresh = s_stops_fresh;
    xSemaphoreGive(s_data_mutex);
    return fresh;
}

} /* extern "C" */
