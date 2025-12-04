#ifndef NET_H
#define NET_H

/*! ESP-IDF Libraies */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
/*! Standard Library */
#include <cstddef>
#include <cstdint>
#include <array>
#include <string_view>

constexpr std::string_view NET_TAG = "SDRUMO_NET"; /*!< Network module logging tag. */
constexpr std::size_t NET_SSID_SIZE = 33U;         /*!< SSID max lenght. */
constexpr std::size_t NET_PASSWORD_SIZE = 65U;     /*!< Password max lenght. */
constexpr std::size_t NET_RVD_DATA_SIZE = 33U;     /*!< Received-Value-Data size. */
constexpr int NET_CONNECTED_BIT = BIT0;
constexpr int NET_ESPTOUCH_DONE_BIT = BIT1;

constexpr std::string_view NET_NVS_NAMESPACE = "storage";     /*!< Non-Volatile Storage partition namespace. */
constexpr std::string_view NET_NVS_SSID_KEY = "SSID";         /*!< Stored SSID key. */
constexpr std::string_view NET_NVS_PASSWORD_KEY = "PASSWORD"; /*!< Stored password key. */

class NetHandler {
  public:
    NetHandler(const NetHandler &obj) = delete;
    void operator=(const NetHandler &) = delete;

    static NetHandler &get_instance(void);

    void init_connection(void);
    void deinit_connection(void);

  private:
    EventGroupHandle_t event_group;
    std::array<char, NET_SSID_SIZE> ssid;
    std::array<char, NET_PASSWORD_SIZE> pass;

    static void event_handler(void *arg, esp_event_base_t event_base, std::int32_t event_id, void *event_data);
    static void smartconfig_task(void *args);

    NetHandler();
    int load_connection_info(void);
    int store_connection_info(void);
};

#endif /*! NET_H */
