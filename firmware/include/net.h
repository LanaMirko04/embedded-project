#ifndef NET_H
#define NET_H

#define NET_SSID_MAX_LEN 32U
#define NET_PASS_MAX_LEN 64U
#define NET_WIFI_CONN_RETRY_NUM 10U
#define NET_WIFI_CONNECTED_BIT BIT0
#define NET_WIFI_FAIL_BIT BIT1
#define NET_TAG "NET"

#ifdef __cplusplus
extern "C" {
#endif /*! __cplusplus */

int net_init(const char *ssid, const char *pass);

// int net_disconnect(void);

#ifdef __cplusplus
}
#endif /*! __cplusplus */

#endif /*! NET_H */
