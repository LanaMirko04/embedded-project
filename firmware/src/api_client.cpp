/*!
 * \file            api_client.cpp
 * \brief           HTTP client for the Sdrumo backend.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "api_client.h"
#include "config.h"
#include "cJSON.h"
#include "lvgl.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "result.h"

extern "C" {
extern const lv_image_dsc_t sunny;
extern const lv_image_dsc_t partly_cloudy;
extern const lv_image_dsc_t cloud;
extern const lv_image_dsc_t rainy;
extern const lv_image_dsc_t storm;
extern const lv_image_dsc_t snow;
extern const lv_image_dsc_t fog;
}

static const lv_image_dsc_t *icon_from_int(int n) {
    switch (n) {
        case 1:  return &sunny;
        case 2:  return &partly_cloudy;
        case 3:  return &cloud;
        case 4:  return &rainy;
        case 5:  return &storm;
        case 6:  return &snow;
        case 7:  return &fog;
        default: return &partly_cloudy;
    }
}

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
    char url[256];
    snprintf(url, sizeof(url), "%s%s", BASE_URL, path);

    esp_http_client_config_t cfg = {
        .url = url,
        .timeout_ms = TIMEOUT_MS,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        return Result::IO_ERROR;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GET %s: open failed: %s", path, esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return Result::IO_ERROR;
    }

    if (esp_http_client_fetch_headers(client) < 0) {
        ESP_LOGE(TAG, "GET %s: fetch headers failed", path);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return Result::IO_ERROR;
    }

    int read = esp_http_client_read_response(client, resp, static_cast<int>(resp_cap) - 1);
    if (read < 0) {
        ESP_LOGE(TAG, "GET %s: read failed", path);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return Result::IO_ERROR;
    }
    resp[read] = '\0';

    if (status_out) {
        *status_out = esp_http_client_get_status_code(client);
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return Result::SUCCESS;
}

Result ApiClient::do_post(const char *path, const char *body, char *resp, std::size_t resp_cap, int *status_out) {
    char url[256];
    snprintf(url, sizeof(url), "%s%s", BASE_URL, path);

    int body_len = body ? static_cast<int>(strlen(body)) : 0;

    esp_http_client_config_t cfg = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = TIMEOUT_MS,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        return Result::IO_ERROR;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_open(client, body_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "POST %s: open failed: %s", path, esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return Result::IO_ERROR;
    }

    if (body_len > 0) {
        if (esp_http_client_write(client, body, body_len) < 0) {
            ESP_LOGE(TAG, "POST %s: write failed", path);
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return Result::IO_ERROR;
        }
    }

    if (esp_http_client_fetch_headers(client) < 0) {
        ESP_LOGE(TAG, "POST %s: fetch headers failed", path);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return Result::IO_ERROR;
    }

    int read = esp_http_client_read_response(client, resp, static_cast<int>(resp_cap) - 1);
    if (read < 0) {
        ESP_LOGE(TAG, "POST %s: read failed", path);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return Result::IO_ERROR;
    }
    resp[read] = '\0';

    if (status_out) {
        *status_out = esp_http_client_get_status_code(client);
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return Result::SUCCESS;
}

Result ApiClient::status(void) {
    MutexLock lk(this->mutex);

    char resp[64];
    int http_status = 0;
    Result r = do_get("/api/status/", resp, sizeof(resp), &http_status);
    if (r != Result::SUCCESS) {
        return Result::API_STATUS_DOWN;
    }
    if (http_status != 200) {
        return Result::API_STATUS_DOWN;
    }
    cJSON *root = cJSON_Parse(resp);
    if (!root) {
        return Result::API_STATUS_DOWN;
    }
    cJSON *s = cJSON_GetObjectItemCaseSensitive(root, "status");
    bool ok = cJSON_IsString(s) && strcmp(s->valuestring, "ok") == 0;
    cJSON_Delete(root);
    ESP_LOGI(TAG, "status: HTTP %d → %s", http_status, ok ? "OK" : "DOWN");
    return ok ? Result::API_STATUS_OK : Result::API_STATUS_DOWN;
}

Result ApiClient::register_device(void) {
    MutexLock lk(this->mutex);

    char *resp = static_cast<char *>(malloc(RESP_BUF_REGISTER));
    if (!resp) return Result::IO_ERROR;

    int http_status = 0;
    Result r = do_post("/api/sdrumo/auth/register", "{}", resp, RESP_BUF_REGISTER, &http_status);
    if (r != Result::SUCCESS) {
        free(resp);
        return r;
    }
    if (http_status != 200 && http_status != 201) {
        result_set_err_msg("register_device: HTTP %d", http_status);
        free(resp);
        return Result::IO_ERROR;
    }

    cJSON *root = cJSON_Parse(resp);
    free(resp);
    if (!root) {
        result_set_err_msg("register_device: JSON parse failed");
        return Result::IO_ERROR;
    }

    cJSON *token = cJSON_GetObjectItemCaseSensitive(root, "token");
    if (!cJSON_IsString(token) || !token->valuestring) {
        result_set_err_msg("register_device: missing 'token' field");
        cJSON_Delete(root);
        return Result::IO_ERROR;
    }

    ESP_LOGI(TAG, "register_device: HTTP %d token=%s", http_status, token->valuestring);
    Config &cfg = Config::get_instance();
    r = cfg.set_device_token(std::string_view(token->valuestring));
    cJSON_Delete(root);
    return r;
    /* token intentionally NOT persisted here — caller saves after pairing confirmed */
}

static bool parse_bus_trip(cJSON *item, BusTrip &t) {
    cJSON *bus_number = cJSON_GetObjectItemCaseSensitive(item, "busNumber");
    cJSON *bus_name   = cJSON_GetObjectItemCaseSensitive(item, "busName");
    cJSON *bus_color  = cJSON_GetObjectItemCaseSensitive(item, "busColor");
    cJSON *delay      = cJSON_GetObjectItemCaseSensitive(item, "delay");
    cJSON *orig_time  = cJSON_GetObjectItemCaseSensitive(item, "originalArrivalTime");
    cJSON *upd_time   = cJSON_GetObjectItemCaseSensitive(item, "updatedArrivalTime");

    if (!cJSON_IsString(bus_number)) return false;

    snprintf(t.bus_number, sizeof(t.bus_number), "%s", bus_number->valuestring);

    if (cJSON_IsString(bus_name))
        snprintf(t.bus_name, sizeof(t.bus_name), "%s", bus_name->valuestring);

    if (cJSON_IsNumber(bus_color)) {
        double v = bus_color->valuedouble;
        t.bus_color = (v >= 0.0 && v <= (double)0xFFFFFF) ? (uint32_t)v : 0x808080U;
    } else {
        t.bus_color = 0x808080U;
    }

    const char *time_str = nullptr;
    if (cJSON_IsString(upd_time) && upd_time->valuestring[0] != '\0')
        time_str = upd_time->valuestring;
    else if (cJSON_IsString(orig_time))
        time_str = orig_time->valuestring;

    if (time_str) {
        const char *t_ptr = strchr(time_str, 'T');
        const char *time_part = t_ptr ? t_ptr + 1 : time_str;
        snprintf(t.eta, sizeof(t.eta), "%.5s", time_part);
    }

    t.delay = cJSON_IsNumber(delay) ? (float)delay->valuedouble : -1.0f;
    return true;
}

static Result parse_trips_response(char *resp, BusModel &out) {
    cJSON *root = cJSON_Parse(resp);
    if (!root || !cJSON_IsArray(root)) {
        result_set_err_msg("fetch_trips: unexpected response format");
        cJSON_Delete(root);
        return Result::IO_ERROR;
    }

    BusModel model = {};
    int idx = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, root) {
        if (idx >= 10) break;
        if (parse_bus_trip(item, model.trips[idx]))
            idx++;
    }
    model.count = idx;
    cJSON_Delete(root);

    out = model;
    ESP_LOGD("API", "fetch_trips: %d trips", idx);
    return Result::SUCCESS;
}

Result ApiClient::fetch_trips(BusModel &out, int stop_id) {
    MutexLock lk(this->mutex);

    char dev_tok[64] = {};
    {
        Config &cfg = Config::get_instance();
        strncpy(dev_tok, cfg.get_device_token(), sizeof(dev_tok) - 1);
    }

    char path[160];
    if (stop_id > 0)
        snprintf(path, sizeof(path), "/api/sdrumo/bus/getTrips/%s?stop_id=%d", dev_tok, stop_id);
    else
        snprintf(path, sizeof(path), "/api/sdrumo/bus/getTrips/%s", dev_tok);

    char *resp = static_cast<char *>(malloc(RESP_BUF_TRIPS));
    if (!resp) return Result::IO_ERROR;

    int http_status = 0;
    Result r = do_get(path, resp, RESP_BUF_TRIPS, &http_status);
    if (r != Result::SUCCESS) { free(resp); return r; }
    if (http_status != 200) {
        result_set_err_msg("fetch_trips: HTTP %d", http_status);
        free(resp);
        return Result::IO_ERROR;
    }

    ESP_LOGI(TAG, "fetch_trips: HTTP %d token=%s stop_id=%d", http_status, dev_tok, stop_id);
    r = parse_trips_response(resp, out);
    free(resp);
    return r;
}

Result ApiClient::fetch_device_config(uint32_t &cfg_rev_out, int &stop_id_out) {
    MutexLock lk(this->mutex);

    char dev_tok[64] = {};
    {
        Config &cfg = Config::get_instance();
        strncpy(dev_tok, cfg.get_device_token(), sizeof(dev_tok) - 1);
    }

    char path[128];
    snprintf(path, sizeof(path), "/api/sdrumo/config/getConfig/%s", dev_tok);

    char resp[RESP_BUF_CONFIG];
    int http_status = 0;
    Result r = do_get(path, resp, sizeof(resp), &http_status);
    if (r != Result::SUCCESS) return r;
    if (http_status != 200) {
        result_set_err_msg("fetch_device_config: HTTP %d", http_status);
        return Result::IO_ERROR;
    }

    cJSON *root = cJSON_Parse(resp);
    if (!root) {
        result_set_err_msg("fetch_device_config: JSON parse failed");
        return Result::IO_ERROR;
    }

    cJSON *cfg_rev = cJSON_GetObjectItemCaseSensitive(root, "cfg_rev");
    cJSON *stop_id = cJSON_GetObjectItemCaseSensitive(root, "stop_id");

    if (!cJSON_IsNumber(cfg_rev)) {
        result_set_err_msg("fetch_device_config: missing cfg_rev");
        cJSON_Delete(root);
        return Result::IO_ERROR;
    }

    cfg_rev_out  = (uint32_t)cfg_rev->valuedouble;
    stop_id_out  = cJSON_IsNumber(stop_id) ? stop_id->valueint : 0;
    ESP_LOGI(TAG, "fetch_device_config: cfg_rev=%lu stop_id=%d",
             (unsigned long)cfg_rev_out, stop_id_out);

    cJSON_Delete(root);
    return Result::SUCCESS;
}

Result ApiClient::fetch_device_stops(DeviceStopList &out) {
    MutexLock lk(this->mutex);

    char dev_tok[64] = {};
    {
        Config &cfg = Config::get_instance();
        strncpy(dev_tok, cfg.get_device_token(), sizeof(dev_tok) - 1);
    }

    char path[128];
    snprintf(path, sizeof(path), "/api/sdrumo/config/getStops/%s", dev_tok);

    char *resp = static_cast<char *>(malloc(RESP_BUF_STOPS));
    if (!resp) return Result::IO_ERROR;

    int http_status = 0;
    Result r = do_get(path, resp, RESP_BUF_STOPS, &http_status);
    if (r != Result::SUCCESS) { free(resp); return r; }
    if (http_status != 200) {
        result_set_err_msg("fetch_device_stops: HTTP %d", http_status);
        free(resp);
        return Result::IO_ERROR;
    }

    cJSON *root = cJSON_Parse(resp);
    free(resp);
    if (!root || !cJSON_IsArray(root)) {
        result_set_err_msg("fetch_device_stops: JSON parse failed");
        cJSON_Delete(root);
        return Result::IO_ERROR;
    }

    DeviceStopList list = {};
    cJSON *item;
    cJSON_ArrayForEach(item, root) {
        if (list.count >= DEVICE_STOPS_MAX) break;
        cJSON *sid  = cJSON_GetObjectItemCaseSensitive(item, "stop_id");
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "stop_name");
        if (!cJSON_IsNumber(sid)) continue;
        list.stops[list.count].stop_id = sid->valueint;
        if (cJSON_IsString(name))
            snprintf(list.stops[list.count].name, sizeof(list.stops[0].name), "%s", name->valuestring);
        list.count++;
    }
    cJSON_Delete(root);

    out = list;
    ESP_LOGI(TAG, "fetch_device_stops: %d stops", list.count);
    return Result::SUCCESS;
}

Result ApiClient::fetch_weather(Weather &out) {
    MutexLock lk(this->mutex);

    char dev_tok[64] = {};
    {
        Config &cfg = Config::get_instance();
        strncpy(dev_tok, cfg.get_device_token(), sizeof(dev_tok) - 1);
    }

    char path[128];
    snprintf(path, sizeof(path), "/api/sdrumo/weather/getWeather/%s", dev_tok);

    char *resp = static_cast<char *>(malloc(RESP_BUF_WEATHER));
    if (!resp) return Result::IO_ERROR;

    int http_status = 0;
    Result r = do_get(path, resp, RESP_BUF_WEATHER, &http_status);
    if (r != Result::SUCCESS) {
        free(resp);
        return r;
    }
    if (http_status != 200) {
        result_set_err_msg("fetch_weather: HTTP %d", http_status);
        free(resp);
        return Result::IO_ERROR;
    }

    cJSON *root = cJSON_Parse(resp);
    free(resp);
    if (!root) {
        result_set_err_msg("fetch_weather: JSON parse failed");
        return Result::IO_ERROR;
    }

    Weather w = {};

    cJSON *city = cJSON_GetObjectItemCaseSensitive(root, "city");
    if (cJSON_IsString(city))
        snprintf(w.city, sizeof(w.city), "%s", city->valuestring);

    cJSON *cur = cJSON_GetObjectItemCaseSensitive(root, "current");
    if (cur) {
        cJSON *date  = cJSON_GetObjectItemCaseSensitive(cur, "date");
        cJSON *weath = cJSON_GetObjectItemCaseSensitive(cur, "weather");
        cJSON *icon  = cJSON_GetObjectItemCaseSensitive(cur, "weather_icon");
        cJSON *temp  = cJSON_GetObjectItemCaseSensitive(cur, "temperature");
        cJSON *rain  = cJSON_GetObjectItemCaseSensitive(cur, "rain_probability");
        cJSON *hum   = cJSON_GetObjectItemCaseSensitive(cur, "humidity");
        cJSON *wind  = cJSON_GetObjectItemCaseSensitive(cur, "wind_speed");

        if (cJSON_IsString(date))
            snprintf(w.date, sizeof(w.date), "%s", date->valuestring);
        if (cJSON_IsString(weath))
            snprintf(w.today_weath, sizeof(w.today_weath), "%s", weath->valuestring);
        /* Numeric fields are always present (safe_hourly defaults to 0, weather_icon to 1) */
        if (temp) snprintf(w.today_temp,      sizeof(w.today_temp),      "%dC",    temp->valueint);
        if (rain) snprintf(w.today_rain,      sizeof(w.today_rain),      "%d%%",   rain->valueint);
        if (hum)  snprintf(w.today_hummidity, sizeof(w.today_hummidity), "%d%%",   hum->valueint);
        if (wind) snprintf(w.today_wind,      sizeof(w.today_wind),      "%dkm/h", wind->valueint);
        if (icon) w.today_weath_icon = *icon_from_int(icon->valueint);
    }

    cJSON *forecast = cJSON_GetObjectItemCaseSensitive(root, "forecast");
    if (cJSON_IsArray(forecast)) {
        struct { char *day; size_t dsz; char *weath; size_t wsz; lv_image_dsc_t *icon; char *temp; size_t tsz; } slots[3] = {
            { w.day1, sizeof(w.day1), w.weather_1, sizeof(w.weather_1), &w.weather_1_icon, w.temp_1, sizeof(w.temp_1) },
            { w.day2, sizeof(w.day2), w.weather_2, sizeof(w.weather_2), &w.weather_2_icon, w.temp_2, sizeof(w.temp_2) },
            { w.day3, sizeof(w.day3), w.weather_3, sizeof(w.weather_3), &w.weather_3_icon, w.temp_3, sizeof(w.temp_3) },
        };
        for (int i = 0; i < 3; i++) {
            cJSON *d = cJSON_GetArrayItem(forecast, i);
            if (!d) break;
            cJSON *day   = cJSON_GetObjectItemCaseSensitive(d, "day");
            cJSON *weath = cJSON_GetObjectItemCaseSensitive(d, "weather");
            cJSON *icon  = cJSON_GetObjectItemCaseSensitive(d, "weather_icon");
            cJSON *temp  = cJSON_GetObjectItemCaseSensitive(d, "temperature");
            if (cJSON_IsString(day))   snprintf(slots[i].day,   slots[i].dsz, "%s", day->valuestring);
            if (cJSON_IsString(weath)) snprintf(slots[i].weath, slots[i].wsz, "%s", weath->valuestring);
            if (temp) snprintf(slots[i].temp, slots[i].tsz, "%dC", temp->valueint);
            if (icon) *slots[i].icon = *icon_from_int(icon->valueint);
        }
    }

    cJSON_Delete(root);
    out = w;
    ESP_LOGI(TAG, "fetch_weather: HTTP %d city=%s", http_status, w.city);
    return Result::SUCCESS;
}
