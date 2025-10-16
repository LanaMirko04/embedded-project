/*!
 * \file            config.h
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           The configuration module
 */

#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_SSID_LEN 0x1F /*!< SSID buffer size */
#define CONFIG_PSWD_LEN 0xF  /*!< Password buffer size */

/*! Enable/Disable Modules */
/*! */

/*!
 * \brief           The CYD configuration.
 */
struct Config {
    char ssid[CONFIG_SSID_LEN]; /*!< Wi-Fi SSID */
    char pswd[CONFIG_PSWD_LEN]; /*!< Wi-Fi Password */
    /*! TODO:  add other voices */
};


#endif /*! CONFIG_H */
