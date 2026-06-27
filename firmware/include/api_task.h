/*!
 * \file            api_task.h
 * \brief           Background API polling task for Sdrumo.
 *
 * C-compatible header — included by C screen files (screen_bus.c, screen_weather.c).
 */

#ifndef API_TASK_H
#define API_TASK_H

#include <stdbool.h>
#include "bus_model.h"
#include "screen_weather.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Create and start the API polling FreeRTOS task.
 *        Must be called after WiFi is connected and device_token is present in Config.
 *        Idempotent: calling twice is a no-op.
 */
void api_task_start(void);

/*!
 * \brief Wake the API task for an immediate bus trip fetch using the device token.
 *        Safe to call from any task context (not from ISR).
 */
void api_task_notify_fetch_bus(void);

/*!
 * \brief Switch to fetching trips for a specific stop and trigger an immediate fetch.
 *        Pass stop_id=0 to revert to the device-token endpoint.
 */
void api_task_notify_fetch_bus_stop(int stop_id);

/*!
 * \brief Copy the latest BusModel into *out under the data mutex.
 * \return true if data has been fetched at least once, false if not yet available.
 */
bool api_task_copy_bus_model(BusModel *out);

/*!
 * \brief Copy the latest Weather into *out under the data mutex.
 * \return true if data has been fetched at least once.
 */
bool api_task_copy_weather(Weather *out);

/*!
 * \brief Returns true if the last status check indicated the server is down.
 */
bool api_task_is_server_down(void);

/*!
 * \brief Copy the latest DeviceStopList into *out under the data mutex.
 * \return true if stops have been fetched at least once, false if not yet available.
 */
bool api_task_copy_stops(DeviceStopList *out);

#ifdef __cplusplus
}
#endif

#endif /*! API_TASK_H */
