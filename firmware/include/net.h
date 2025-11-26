#ifndef NET_H
#define NET_H

#define NET_SSID_SIZE 33U     /*!<  */
#define NET_PASSWORD_SIZE 65U /*!<  */
#define NET_RVD_DATA_SIZE 33U /*!<  */

#define NET_NVS_NAMESPACE "storage"
#define NET_NVS_SSID_KEY "SSID"         /*!<*/
#define NET_NVS_PASSWORD_KEY "PASSWORD" /*!<*/

#ifdef __cplusplus
extern "C" {
#endif /*! __cplusplus */

/*!
 * \brief
 */
void net_init(void);

/*!
 */
void net_deinit(void);

#ifdef __cplusplus
}
#endif /*! __cplusplus */

#endif /*! NET_H */
