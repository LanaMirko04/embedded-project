/*!
 * \file            config.h
 * \date            2025-11-12
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Configuration handling module.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bus_model.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "result.h"
#include "mutex_lock.h"

class Config {
  public:
    static constexpr std::uint32_t CURRENT_SCHEMA_VER = 2U;

    Config(const Config &obj) = delete;
    void operator=(const Config &) = delete;

    static Config &get_instance(void);

    const char *get_ssid(void);
    const char *get_password(void);
    const char *get_device_token(void);
    std::uint32_t get_schema_ver(void);
    std::uint32_t get_cfg_rev(void);
    int           get_stop_id(void);
    DeviceStopList get_stops(void);

    Result set_ssid(const char *ssid, std::size_t len);
    Result set_ssid(std::string_view ssid);
    Result set_password(const char *password, std::size_t len);
    Result set_password(std::string_view password);
    Result set_credentials(std::string_view ssid, std::string_view password);
    Result set_device_token(std::string_view token);
    void set_schema_ver(std::uint32_t v);
    void set_cfg_rev(std::uint32_t v);
    void set_stop_id(int v);
    void set_stops(const DeviceStopList &stops);

    Result load(void);
    Result store(void);
    Result fetch(void);
    Result factory_reset(void);
    bool is_up_to_date(void);
    bool is_dirty(void) const { return dirty; }

  private:
    static constexpr std::size_t SSID_SIZE = 33U;
    static constexpr std::size_t PASSWORD_SIZE = 65U;
    static constexpr std::size_t DEVICE_TOKEN_SIZE = 64U;
    static constexpr char NVS_NAMESPACE[] = "config";
    static constexpr char NVS_SCHEMA_VER_KEY[] = "schema_ver";
    static constexpr char NVS_CFG_REV_KEY[] = "cfg_rev";
    static constexpr char NVS_STOP_ID_KEY[]  = "stop_id";
    static constexpr char NVS_STOPS_KEY[]    = "stops";
    static constexpr char NVS_SSID_KEY[] = "ssid";
    static constexpr char NVS_PASSWORD_KEY[] = "password";
    static constexpr char NVS_DEVICE_TOKEN_KEY[] = "dev_token";
    static constexpr const char *TAG = "CONFIG";

    static_assert(SSID_SIZE <= 33U, "SSID_SIZE must fit WiFi struct (32 + NUL)");
    static_assert(PASSWORD_SIZE <= 65U, "PASSWORD_SIZE must fit WiFi struct (64 + NUL)");

    std::uint32_t schema_ver;
    std::uint32_t cfg_rev;
    int           stop_id;
    DeviceStopList stops;
    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];
    char device_token[DEVICE_TOKEN_SIZE];
    bool dirty;
    SemaphoreHandle_t mutex;

    Config(void);
    ~Config(void);

    static bool is_valid_ssid(const char *s, std::size_t len);
};

#endif /*! CONFIG_H */
