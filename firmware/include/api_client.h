/*!
 * \file            api_client.h
 * \brief           HTTP(S) client for the Sdrumo backend.
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

    Result ping(void);
    Result register_device(void);
    Result fetch_trips(BusModel &out);
    Result fetch_weather(Weather &out);

  private:
    static constexpr const char *TAG = "API";

    SemaphoreHandle_t mutex;

    ApiClient(void);
    ~ApiClient(void);

    Result do_get(const char *path, char *resp, std::size_t resp_cap, int *status_out);
    Result do_post(const char *path, const char *body, char *resp, std::size_t resp_cap, int *status_out);
};

#endif /*! API_CLIENT_H */
