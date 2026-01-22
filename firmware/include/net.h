/*!
 * \file            net.h
 * \date            2025-11-12
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Network handling module.
 */

#ifndef NET_H
#define NET_H

/*! Sdrumo Modules */
#include "result.h"
/*! ESP-IDF Components */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
/*! Standard Library */
#include <atomic>
#include <cstddef>
#include <cstdint>

constexpr std::size_t NET_MAX_RETRY = 5U;   /*!< Max number of Wi-Fi connection retries. */
constexpr int NET_CONNECTED_BIT = BIT0;     /*!< Bit that indicates that Sdumo got an IP address. */
constexpr int NET_ESPTOUCH_DONE_BIT = BIT1; /*!< Bit that indicates that Sdrumo finished SmartConfig/ESPTouch routine. */

/*!
 * \class           NetHandler
 * \brief           Singleton class responsible for Wi-Fi initialization, connection handling,
 *                  SmartConfig provisioning, and credential storage.
 */
class NetHandler {
  public:
    /*!
     * \brief       Deleted copy constructor.
*/
    NetHandler(const NetHandler &obj) = delete;

    /*!
     * \brief       Deleted copy assignment operator.
     */
    void operator=(const NetHandler &) = delete;

    /*!
     * \brief       Returns the global instance of NetHandler.
     */
    static NetHandler &get_instance(void);

    /*!
     * \brief       Initializes the Wi-Fi subsystem and attempts to connect.
     */
    void init_connection(void);

    /*!
     * \brief       Deinitializes Wi-Fi and frees internal resources.
     */
    void deinit_connection(void);

  private:
    static constexpr std::size_t RVD_DATA_SIZE = 33U; /*!< Received-Value-Data size. */

    EventGroupHandle_t event_group;         /*!< Event group handle for Wi-Fi events. */
    esp_netif_t *sta_netif;                 /*!< Station network interface handle. */
    std::atomic_size_t retry_count;         /*!< Retry count for Wi-Fi connection attempts. */
    std::atomic_bool smartconfig_running;   /*!< Flag indicating if SmartConfig is running. */
    std::atomic_bool came_from_smartconfig; /*!< Flag indicating if connection came from SmartConfig. */

    /*!
     * \brief       Wi-Fi and SmartConfig event handler callback.
     *
     * \param[in]   arg Optional user argument (unused).
     * \param[in]   event_base Event category (WIFI_EVENT, IP_EVENT, SC_EVENT).
     * \param[in]   event_id Specific event code inside the category.
     * \param[in]   event_data Pointer to event-specific data (varies by event type).
     */
    static void event_handler(void *arg, esp_event_base_t event_base, std::int32_t event_id, void *event_data);

    /*!
     * \brief       Task that runs SmartConfig provisioning until completion.
     *
     * \param[in]   args Optional task parameter (unused).
     */
    static void smartconfig_task(void *args);

    /*!
     * \brief       Private constructor for the singleton.
     */
    NetHandler() : event_group(nullptr), retry_count(0U), smartconfig_running(false), came_from_smartconfig(false) {
    }
};

#endif /*! NET_H */
