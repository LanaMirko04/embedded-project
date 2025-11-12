#ifndef NET_H
#define NET_H

#define NET_SSID_MAX_LEN 32U
#define NET_PASS_MAX_LEN 64U

#ifdef __cplusplus
extern "C" {
#endif /*! __cplusplus */

int net_connect(const char *ssid, const char *pass);

int net_disconnect(void);

#ifdef __cplusplus
}
#endif /*! __cplusplus */

#endif /*! NET_H */
