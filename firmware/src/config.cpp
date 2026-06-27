/*!
 * \file            config.cpp
 * \date            2025-11-12
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Configuration handling module.
 */

#include <cerrno>
#include <cstdint>
#include <cstring>

#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "result.h"

namespace {
struct NvsHandle {
    nvs_handle_t h{};
    bool open{false};
    ~NvsHandle() {
        if (open) nvs_close(h);
    }
};
} // namespace

#define NVS_TRY(call, msg)                                                      \
    do {                                                                        \
        esp_err_t _r = (call);                                                  \
        if (_r != ESP_OK) {                                                     \
            result_set_err_msg(msg " (%s)", esp_err_to_name(_r));               \
            return Result::IO_ERROR;                                            \
        }                                                                       \
    } while (0)

Config &Config::get_instance(void) {
    static Config instance;
    return instance;
}

const char *Config::get_ssid(void) {
    MutexLock lk(this->mutex);
    return this->ssid;
}

const char *Config::get_password(void) {
    MutexLock lk(this->mutex);
    return this->password;
}

std::uint32_t Config::get_schema_ver(void) {
    MutexLock lk(this->mutex);
    return this->schema_ver;
}

std::uint32_t Config::get_cfg_rev(void) {
    MutexLock lk(this->mutex);
    return this->cfg_rev;
}

const char *Config::get_device_token(void) {
    MutexLock lk(this->mutex);
    return this->device_token;
}

Result Config::set_device_token(std::string_view token) {
    if (token.size() >= Config::DEVICE_TOKEN_SIZE) {
        result_set_err_msg("device token too long (%zu >= %zu)", token.size(), Config::DEVICE_TOKEN_SIZE);
        return Result::INVALID_ARGUMENT;
    }
    MutexLock lk(this->mutex);
    memset(this->device_token, 0, Config::DEVICE_TOKEN_SIZE);
    memcpy(this->device_token, token.data(), token.size());
    this->dirty = true;
    return Result::SUCCESS;
}

bool Config::is_valid_ssid(const char *s, std::size_t len) {
    if (len == 0U || len > 32U) return false;
    for (std::size_t i = 0; i < len; ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x20U || c == 0x7FU) return false;
    }
    return true;
}

Result Config::set_ssid(const char *new_ssid, std::size_t len) {
    if (!new_ssid) {
        result_set_err_msg("SSID is NULL");
        return Result::INVALID_ARGUMENT;
    }
    if (len >= Config::SSID_SIZE) {
        result_set_err_msg("SSID is too long (%zu >= %zu)", len, Config::SSID_SIZE);
        return Result::INVALID_ARGUMENT;
    }
    if (!is_valid_ssid(new_ssid, len)) {
        result_set_err_msg("SSID contains invalid characters");
        return Result::INVALID_ARGUMENT;
    }

    MutexLock lk(this->mutex);
    memset(this->ssid, 0, Config::SSID_SIZE);
    memcpy(this->ssid, new_ssid, len);
    this->dirty = true;
    return Result::SUCCESS;
}

Result Config::set_ssid(std::string_view new_ssid) {
    return set_ssid(new_ssid.data(), new_ssid.size());
}

Result Config::set_password(const char *new_password, std::size_t len) {
    if (!new_password) {
        result_set_err_msg("PASSWORD is NULL");
        return Result::INVALID_ARGUMENT;
    }
    if (len >= Config::PASSWORD_SIZE) {
        result_set_err_msg("PASSWORD is too long (%zu >= %zu)", len, Config::PASSWORD_SIZE);
        return Result::INVALID_ARGUMENT;
    }

    MutexLock lk(this->mutex);
    memset(this->password, 0, Config::PASSWORD_SIZE);
    memcpy(this->password, new_password, len);
    this->dirty = true;
    return Result::SUCCESS;
}

Result Config::set_password(std::string_view new_password) {
    return set_password(new_password.data(), new_password.size());
}

Result Config::set_credentials(std::string_view new_ssid, std::string_view new_password) {
    if (new_ssid.size() >= Config::SSID_SIZE) {
        result_set_err_msg("SSID too long (%zu >= %zu)", new_ssid.size(), Config::SSID_SIZE);
        return Result::INVALID_ARGUMENT;
    }
    if (new_password.size() >= Config::PASSWORD_SIZE) {
        result_set_err_msg("PASSWORD too long (%zu >= %zu)", new_password.size(), Config::PASSWORD_SIZE);
        return Result::INVALID_ARGUMENT;
    }
    if (!is_valid_ssid(new_ssid.data(), new_ssid.size())) {
        result_set_err_msg("SSID contains invalid characters");
        return Result::INVALID_ARGUMENT;
    }

    MutexLock lk(this->mutex);
    memset(this->ssid, 0, Config::SSID_SIZE);
    memcpy(this->ssid, new_ssid.data(), new_ssid.size());
    memset(this->password, 0, Config::PASSWORD_SIZE);
    memcpy(this->password, new_password.data(), new_password.size());
    this->dirty = true;
    return Result::SUCCESS;
}

void Config::set_schema_ver(std::uint32_t v) {
    MutexLock lk(this->mutex);
    if (this->schema_ver != v) {
        this->schema_ver = v;
        this->dirty = true;
    }
}

void Config::set_cfg_rev(std::uint32_t v) {
    MutexLock lk(this->mutex);
    if (this->cfg_rev != v) {
        this->cfg_rev = v;
        this->dirty = true;
    }
}

int Config::get_stop_id(void) {
    MutexLock lk(this->mutex);
    return this->stop_id;
}

void Config::set_stop_id(int v) {
    MutexLock lk(this->mutex);
    if (this->stop_id != v) {
        this->stop_id = v;
        this->dirty = true;
    }
}

DeviceStopList Config::get_stops(void) {
    MutexLock lk(this->mutex);
    return this->stops;
}

void Config::set_stops(const DeviceStopList &s) {
    MutexLock lk(this->mutex);
    if (memcmp(&this->stops, &s, sizeof(DeviceStopList)) != 0) {
        this->stops = s;
        this->dirty = true;
    }
}

static Result read_str_field(nvs_handle_t h, const char *key, char *dst, std::size_t dst_size) {
    std::size_t required = 0U;
    esp_err_t res = nvs_get_str(h, key, nullptr, &required);
    if (res == ESP_ERR_NVS_NOT_FOUND) {
        return Result::NOT_FOUND;
    }
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) sizing %s", esp_err_to_name(res), key);
        return Result::IO_ERROR;
    }
    if (required > dst_size) {
        result_set_err_msg("Stored %s too large (%zu > %zu)", key, required, dst_size);
        return Result::IO_ERROR;
    }
    res = nvs_get_str(h, key, dst, &required);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) reading %s", esp_err_to_name(res), key);
        return Result::IO_ERROR;
    }
    return Result::SUCCESS;
}

Result Config::load(void) {
    MutexLock lk(this->mutex);

    NvsHandle nvs;
    NVS_TRY(nvs_open(Config::NVS_NAMESPACE, NVS_READONLY, &nvs.h), "Error opening the storage");
    nvs.open = true;

    char tmp_ssid[Config::SSID_SIZE] = {};
    char tmp_password[Config::PASSWORD_SIZE] = {};
    char tmp_device_token[Config::DEVICE_TOKEN_SIZE] = {};
    std::uint32_t tmp_schema_ver = 0U;
    std::uint32_t tmp_cfg_rev = 0U;

    Result r = read_str_field(nvs.h, Config::NVS_SSID_KEY, tmp_ssid, Config::SSID_SIZE);
    if (r == Result::NOT_FOUND) {
        ESP_LOGW(TAG, "No stored configuration");
        return Result::NOT_FOUND;
    }
    if (r != Result::SUCCESS) return r;

    r = read_str_field(nvs.h, Config::NVS_PASSWORD_KEY, tmp_password, Config::PASSWORD_SIZE);
    if (r != Result::SUCCESS) return r;

    r = read_str_field(nvs.h, Config::NVS_DEVICE_TOKEN_KEY, tmp_device_token, Config::DEVICE_TOKEN_SIZE);
    if (r != Result::SUCCESS && r != Result::NOT_FOUND) return r;

    esp_err_t res = nvs_get_u32(nvs.h, Config::NVS_SCHEMA_VER_KEY, &tmp_schema_ver);
    if (res != ESP_OK && res != ESP_ERR_NVS_NOT_FOUND) {
        result_set_err_msg("Error (%s) reading schema_ver", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    res = nvs_get_u32(nvs.h, Config::NVS_CFG_REV_KEY, &tmp_cfg_rev);
    if (res != ESP_OK && res != ESP_ERR_NVS_NOT_FOUND) {
        result_set_err_msg("Error (%s) reading cfg_rev", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    DeviceStopList tmp_stops = {};
    std::size_t stops_sz = sizeof(DeviceStopList);
    res = nvs_get_blob(nvs.h, Config::NVS_STOPS_KEY, &tmp_stops, &stops_sz);
    if (res != ESP_OK && res != ESP_ERR_NVS_NOT_FOUND) {
        result_set_err_msg("Error (%s) reading stops", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    int32_t tmp_stop_id = 0;
    res = nvs_get_i32(nvs.h, Config::NVS_STOP_ID_KEY, &tmp_stop_id);
    if (res != ESP_OK && res != ESP_ERR_NVS_NOT_FOUND) {
        result_set_err_msg("Error (%s) reading stop_id", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    memcpy(this->ssid, tmp_ssid, Config::SSID_SIZE);
    memcpy(this->password, tmp_password, Config::PASSWORD_SIZE);
    memcpy(this->device_token, tmp_device_token, Config::DEVICE_TOKEN_SIZE);
    this->schema_ver = tmp_schema_ver;
    this->cfg_rev    = tmp_cfg_rev;
    this->stops      = tmp_stops;
    this->stop_id    = static_cast<int>(tmp_stop_id);
    this->dirty = false;

    ESP_LOGI(TAG, "Config loaded: ssid=%s token=%s schema_ver=%lu",
             this->ssid,
             this->device_token[0] ? this->device_token : "(none)",
             static_cast<unsigned long>(this->schema_ver));

    // cppcheck-suppress knownConditionTrueFalse ; intentional: branch activates when CURRENT_SCHEMA_VER bumps
    if (tmp_schema_ver != 0U && tmp_schema_ver < CURRENT_SCHEMA_VER) {
        ESP_LOGW(TAG, "Schema upgrade %lu -> %lu",
                 static_cast<unsigned long>(tmp_schema_ver),
                 static_cast<unsigned long>(CURRENT_SCHEMA_VER));
        this->schema_ver = CURRENT_SCHEMA_VER;
        this->dirty = true;
    } else if (tmp_schema_ver > CURRENT_SCHEMA_VER) {
        ESP_LOGE(TAG, "Stored schema (%lu) newer than firmware (%lu)",
                 static_cast<unsigned long>(tmp_schema_ver),
                 static_cast<unsigned long>(CURRENT_SCHEMA_VER));
        return Result::INVALID_ARGUMENT;
    }

    return Result::SUCCESS;
}

Result Config::store(void) {
    MutexLock lk(this->mutex);

    if (!this->dirty) {
        ESP_LOGD(TAG, "Skip store: clean");
        return Result::SUCCESS;
    }

    NvsHandle nvs;
    NVS_TRY(nvs_open(Config::NVS_NAMESPACE, NVS_READWRITE, &nvs.h), "Error opening the storage");
    nvs.open = true;

    NVS_TRY(nvs_set_str(nvs.h, Config::NVS_SSID_KEY, this->ssid), "Error saving SSID");
    NVS_TRY(nvs_set_str(nvs.h, Config::NVS_PASSWORD_KEY, this->password), "Error saving password");
    if (this->device_token[0] != '\0') {
        NVS_TRY(nvs_set_str(nvs.h, Config::NVS_DEVICE_TOKEN_KEY, this->device_token), "Error saving device token");
    }

    std::uint32_t sv = (this->schema_ver == 0U) ? CURRENT_SCHEMA_VER : this->schema_ver;
    NVS_TRY(nvs_set_u32(nvs.h, Config::NVS_SCHEMA_VER_KEY, sv), "Error saving schema_ver");

    if (this->cfg_rev != 0U) {
        NVS_TRY(nvs_set_u32(nvs.h, Config::NVS_CFG_REV_KEY, this->cfg_rev), "Error saving cfg_rev");
    }
    if (this->stops.count > 0) {
        NVS_TRY(nvs_set_blob(nvs.h, Config::NVS_STOPS_KEY, &this->stops, sizeof(DeviceStopList)), "Error saving stops");
    }
    if (this->stop_id != 0) {
        NVS_TRY(nvs_set_i32(nvs.h, Config::NVS_STOP_ID_KEY, static_cast<int32_t>(this->stop_id)), "Error saving stop_id");
    }

    NVS_TRY(nvs_commit(nvs.h), "Error committing config");
    this->schema_ver = sv;
    this->dirty = false;
    ESP_LOGI(TAG, "Config stored: ssid=%s token=%s", this->ssid, this->device_token[0] ? this->device_token : "(none)");

    return Result::SUCCESS;
}

Result Config::fetch(void) {
    /*! TODO: API request needed (no server porcaccio dio) */
    ESP_LOGW(TAG, "%s not implemented yet... ( T^T)", __FUNCTION__);
    return Result::SUCCESS;
}

Result Config::factory_reset(void) {
    MutexLock lk(this->mutex);

    NvsHandle nvs;
    NVS_TRY(nvs_open(Config::NVS_NAMESPACE, NVS_READWRITE, &nvs.h), "Error opening the storage");
    nvs.open = true;

    esp_err_t res = nvs_erase_all(nvs.h);
    if (res != ESP_OK && res != ESP_ERR_NVS_NOT_FOUND) {
        result_set_err_msg("Error (%s) erasing config", esp_err_to_name(res));
        return Result::IO_ERROR;
    }
    NVS_TRY(nvs_commit(nvs.h), "Error committing erase");

    memset(this->ssid, 0, Config::SSID_SIZE);
    memset(this->password, 0, Config::PASSWORD_SIZE);
    memset(this->device_token, 0, Config::DEVICE_TOKEN_SIZE);
    this->schema_ver = 0U;
    this->cfg_rev    = 0U;
    this->stop_id    = 0;
    memset(&this->stops, 0, sizeof(DeviceStopList));
    this->dirty = false;

    ESP_LOGW(TAG, "Factory reset done");
    return Result::SUCCESS;
}

bool Config::is_up_to_date(void) {
    /*! TODO: API request needed (no server porcaccio dio) */
    return true;
}

Config::Config(void)
    : schema_ver(0U), cfg_rev(0U), stop_id(0), stops{}, ssid{}, password{}, device_token{}, dirty(false), mutex(xSemaphoreCreateMutex()) {
    if (!mutex) {
        ESP_LOGE(TAG, "Mutex creation failed");
    }
    load(); // TODO: error handling
}

Config::~Config(void) {
    if (mutex) {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
    }
}
