/*!
 * \file            rc.c
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Return codes and error handling.
 */

#include "rc.h"

#include <string.h>

static char rc_err_msg[RC_ERR_MSG_LEN] = { 0 };

int rc_set_err_msg(const char msg[RC_ERR_MSG_LEN]) {
    if (msg == NULL)
        return RC_ERR_NULL_PTR;

    bzero(rc_err_msg, RC_ERR_MSG_LEN);
    strncpy(rc_err_msg, msg, RC_ERR_MSG_LEN);

    return RC_OK;
}

const char *rc_get_err_msg(void) {
    return &rc_err_msg;
}
