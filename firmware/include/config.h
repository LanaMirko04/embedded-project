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

#include "nvs.h"
#include "result.h"

class Config {
  public:
    Config(const Config &obj) = delete;
    void operator=(const Config &) = delete;

    static Config &get_instance(void);

    const char *get_ssid(void);
    const char *get_password(void);
    uint32_t get_version(void);
    Result set_ssid(const char *ssid, std::size_t len);
    Result set_password(const char *password, std::size_t len);
    void set_version(std::uint32_t version);

    Result load(void);
    Result store(void);
    Result fetch(void);
    bool is_up_to_date(void);

  private:
    static constexpr std::size_t SSID_SIZE = 33U;
    static constexpr std::size_t PASSWORD_SIZE = 65U;
    static constexpr char NVS_NAMESPACE[] = "config";
    static constexpr char NVS_VERSION_KEY[] = "version";
    static constexpr char NVS_SSID_KEY[] = "ssid";
    static constexpr char NVS_PASSWORD_KEY[] = "password";

    std::uint32_t version;
    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];

    Config(void);
    ~Config(void);
};

#endif /*! CONFIG_H */
