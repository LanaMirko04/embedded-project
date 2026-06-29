/*!
 * \file            bus_model.h
 * \brief           Bus trips data model returned by the Sdrumo API.
 */

#ifndef BUS_MODEL_H
#define BUS_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char     bus_number[8];
    char     bus_name[64];
    uint32_t bus_color;   /* 0xRRGGBB; 0x808080 if null */
    char     eta[12];     /* arrival time string e.g. "10:30" */
    float    delay;       /* minutes; -1.0 if not real-time tracked */
} BusTrip;

typedef struct {
    BusTrip trips[10];
    int     count;
} BusModel;

#define DEVICE_STOPS_MAX 10

typedef struct {
    int  stop_id;
    char name[64];
} DeviceStop;

typedef struct {
    DeviceStop stops[DEVICE_STOPS_MAX];
    int        count;
} DeviceStopList;

#ifdef __cplusplus
}
#endif

#endif /*! BUS_MODEL_H */
