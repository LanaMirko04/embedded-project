/*!
 * \file            api_client.cpp
 * \brief           HTTP(S) client for the Sdrumo backend.
 */

#include <cstring>

#include "api_client.h"
#include "config.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "result.h"
#include "sdkconfig.h"

#ifndef CONFIG_SDRUMO_API_BASE_URL
#define CONFIG_SDRUMO_API_BASE_URL "https://api.sdrumo.example"
#endif
#ifndef CONFIG_SDRUMO_API_TIMEOUT_MS
#define CONFIG_SDRUMO_API_TIMEOUT_MS 5000
#endif
#ifndef CONFIG_SDRUMO_API_RESP_BUF_SIZE
#define CONFIG_SDRUMO_API_RESP_BUF_SIZE 8192
#endif

ApiClient &ApiClient::get_instance(void) {
    static ApiClient instance;
    return instance;
}

ApiClient::ApiClient(void) : mutex(xSemaphoreCreateMutex()) {
    if (!mutex) {
        ESP_LOGE(TAG, "Mutex creation failed");
    }
}

ApiClient::~ApiClient(void) {
    if (mutex) {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
    }
}

Result ApiClient::do_get(const char *path, char *resp, std::size_t resp_cap, int *status_out) {
    (void)path;
    (void)resp;
    (void)resp_cap;
    if (status_out) *status_out = 0;
    ESP_LOGW(TAG, "do_get not implemented yet");
    return Result::SUCCESS;
}

Result ApiClient::do_post(const char *path, const char *body, char *resp, std::size_t resp_cap, int *status_out) {
    (void)path;
    (void)body;
    (void)resp;
    (void)resp_cap;
    if (status_out) *status_out = 0;
    ESP_LOGW(TAG, "do_post not implemented yet");
    return Result::SUCCESS;
}

Result ApiClient::ping(void) {
    MutexLock lk(this->mutex);
    ESP_LOGW(TAG, "ping not implemented yet");
    return Result::SUCCESS;
}

Result ApiClient::register_device(void) {
    MutexLock lk(this->mutex);
    ESP_LOGW(TAG, "register_device not implemented yet");
    return Result::SUCCESS;
}

Result ApiClient::fetch_trips(BusModel &out) {
    MutexLock lk(this->mutex);
    (void)out;
    ESP_LOGW(TAG, "fetch_trips not implemented yet");
    return Result::SUCCESS;
}

Result ApiClient::fetch_weather(Weather &out) {
    MutexLock lk(this->mutex);
    (void)out;
    ESP_LOGW(TAG, "fetch_weather not implemented yet");
    return Result::SUCCESS;
}
