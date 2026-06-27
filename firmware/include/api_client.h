/*!
 * \file            api_client.h
 * \brief           HTTP client for the Sdrumo backend.
 */

#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mutex_lock.h"

#include "bus_model.h"
#include "result.h"
#include "screen_weather.h"

class ApiClient {
  public:
    ApiClient(const ApiClient &) = delete;
    void operator=(const ApiClient &) = delete;

    static ApiClient &get_instance(void);

    Result status(void);
    Result register_device(void);
    Result fetch_trips(BusModel &out, int stop_id = 0);
    Result fetch_weather(Weather &out);
    Result fetch_device_config(uint32_t &cfg_rev_out, int &stop_id_out);
    Result fetch_device_stops(DeviceStopList &out);

  private:
    static constexpr const char *TAG = "API";
    static constexpr const char *BASE_URL = "http://172.20.10.2:5000";
    static constexpr int TIMEOUT_MS = 5000;
    static constexpr int RESP_BUF_REGISTER = 128;
    static constexpr int RESP_BUF_TRIPS    = 2048;
    static constexpr int RESP_BUF_WEATHER  = 1024;
    static constexpr int RESP_BUF_CONFIG   = 128;
    static constexpr int RESP_BUF_STOPS    = 1024;

    SemaphoreHandle_t mutex;

    ApiClient(void);
    ~ApiClient(void);

    Result do_get(const char *path, char *resp, std::size_t resp_cap, int *status_out);
    Result do_post(const char *path, const char *body, char *resp, std::size_t resp_cap, int *status_out);
};

#endif /*! API_CLIENT_H */
