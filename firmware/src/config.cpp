/*!
 * \file            config.cpp
 * \date            2025-11-12
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Configuration handling module.
 */

// #include <cstddef>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <cerrno>

#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "result.h"

static const char *CONFIG_TAG = "CONFIG";

Config &Config::get_instance(void) {
    static Config instance;
    return instance;
}

const char *Config::get_ssid(void) {
    return this->ssid;
}

const char *Config::get_password(void) {
    return this->password;
}

std::uint32_t Config::get_version(void) {
    return this->version;
}

Result Config::set_ssid(const char *ssid, std::size_t len) {
    if (!ssid) {
        result_set_err_msg("SSID is NULL");
        return Result::INVALID_ARGUMENT;
    }

    if (len >= Config::SSID_SIZE) {
        result_set_err_msg("SSID is too long (%lu >= %lu)", len, Config::SSID_SIZE);
        return Result::INVALID_ARGUMENT;
    }

    memset(this->ssid, 0, Config::SSID_SIZE);
    if (!strncpy(this->ssid, ssid, len)) {
        result_set_err_msg("An error occurred during strncpy(): %s", strerror(errno));
        return Result::UNEXPECTED_NULL_POINTER;
    }

    return Result::SUCCESS;
}

Result Config::set_password(const char *password, std::size_t len) {
    if (!password) {
        result_set_err_msg("PASSWORD is NULL");
        return Result::INVALID_ARGUMENT;
    }

    if (len >= Config::PASSWORD_SIZE) {
        result_set_err_msg("PASSWORD is too long (%lu >= %lu)", len, Config::PASSWORD_SIZE);
        return Result::INVALID_ARGUMENT;
    }

    memset(this->password, 0, Config::PASSWORD_SIZE);
    if (!strncpy(this->password, password, len)) {
        result_set_err_msg("An error occurred during strncpy(): %s", strerror(errno));
        return Result::UNEXPECTED_NULL_POINTER;
    }

    return Result::SUCCESS;
}

void Config::set_version(std::uint32_t version) {
    this->version = version;
}

Result Config::load(void) {
    nvs_handle_t nvs;

    int res = nvs_open(Config::NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    std::size_t len;
    memset(this->ssid, 0U, Config::SSID_SIZE);
    res = nvs_get_str(nvs, Config::NVS_SSID_KEY, this->ssid, &len);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) reading SSID", esp_err_to_name(res));
        nvs_close(nvs);
        return Result::IO_ERROR;
    }

    memset(this->password, 0, Config::PASSWORD_SIZE);
    res = nvs_get_str(nvs, Config::NVS_PASSWORD_KEY, this->password, &len);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) reading password", esp_err_to_name(res));
        nvs_close(nvs);
        return Result::IO_ERROR;
    }

    res = nvs_get_u32(nvs, Config::NVS_VERSION_KEY, &this->version);
    switch (res) {
        case ESP_OK:
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(CONFIG_TAG, "No stored configuration");
            nvs_close(nvs);
            return Result::NOT_FOUND; // CONFIG_NOT_FOUND could be a better name, but I'm too lazy to a specific return code only for that

        default:
            result_set_err_msg("Unknown error (%s) reading config version", esp_err_to_name(res));
            nvs_close(nvs);
            return Result::UNKNOWN_ERROR;
    }

    nvs_close(nvs);
    return Result::SUCCESS;
}

Result Config::store(void) {
    nvs_handle_t nvs;

    esp_err_t res = nvs_open(Config::NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) opening the storage", esp_err_to_name(res));
        return Result::IO_ERROR;
    }

    res = nvs_set_str(nvs, Config::NVS_SSID_KEY, this->ssid);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) saving SSID!", esp_err_to_name(res));
        nvs_close(nvs);
        return Result::IO_ERROR;
    }

    res = nvs_set_str(nvs, Config::NVS_PASSWORD_KEY, this->password);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) saving password!", esp_err_to_name(res));
        nvs_close(nvs);
        return Result::IO_ERROR;
    }

    if (this->version == 0U) {
        ESP_LOGW(CONFIG_TAG, "No loaded configuration");
        nvs_close(nvs);
        return Result::SUCCESS;
    }

    res = nvs_set_u32(nvs, Config::NVS_VERSION_KEY, this->version);
    if (res != ESP_OK) {
        result_set_err_msg("Error (%s) saving version!", esp_err_to_name(res));
        nvs_close(nvs);
        return Result::IO_ERROR;
    }

    /*! TODO: give me config parameters porcaccia la madonna plz */
    ESP_LOGW(CONFIG_TAG, "%s not implemented yet... ( T^T)", __FUNCTION__);

    nvs_close(nvs);
    return Result::SUCCESS;
}

Result Config::fetch(void) {
    /*! TODO: API request needed (no server porcaccio dio) */
    ESP_LOGW(CONFIG_TAG, "%s not implemented yet... ( T^T)", __FUNCTION__);
    return Result::SUCCESS;
}

bool Config::is_up_to_date(void) {
    /*! TODO: API request needed (no server porcaccio dio) */
    return true;
}

Config::Config(void) {
    memset(this->ssid, 0, Config::SSID_SIZE);
    memset(this->password, 0, Config::PASSWORD_SIZE);
    this->version = 0;

}

Config::~Config(void) {
    /*! TODO: store configuration into nvs. */
}
