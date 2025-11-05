/*!
 * \file            rc.h
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Return codes and error handling.
 */

#ifndef RC_H
#define RC_H

#define RC_ERR_MSG_LEN 128U /*!< Error message buffer length */

#define RC_FAIL -1 /*!< Undefined error */
#define RC_OK 0    /*!< Everything's OK */

#define RC_ERR_INVALID_ARG 0x101  /*!< Invalid argument */
#define RC_ERR_INVALID_SIZE 0x102 /*!< Invalid argument */
#define RC_ERR_NULL_PTR 0x103     /*!< Unexpected NULL pointer */

#define RC_FSM_ERR_INVALID_NEXT_STATE 0x201 /*!< Invalid FSM next state */

/*!
 * \brief           Set the error message.
 *
 * \param[in]       fmt: Format string.
 * \param[in]       ...: variable arguments.
 * \return          The length of the error message on success, RC_FAIL otherwise.
 */
int rc_set_err_msg(const char *fmt, ...);

/*!
 * \brief           Get the error message.
 *
 * \return          The pointer to the error message.
 */
const char *rc_get_err_msg(void);

#endif /*! ERR_H */
