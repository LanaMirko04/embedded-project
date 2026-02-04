/*!
 * \file            rc.c
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Return codes and error handling.
 */

#include "rc.hh"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char rc_err_msg[RC_ERR_MSG_LEN] = { 0 }; /*!< The error message buffer */

int rc_set_err_msg(const char *fmt, ...) {
    if (!fmt)
        return RC_ERR_NULL_PTR;

    va_list args;
    va_start(args, fmt);

    bzero(rc_err_msg, RC_ERR_MSG_LEN);
    int len = vsnprintf(rc_err_msg, RC_ERR_MSG_LEN, fmt, args);

    va_end(args);

    return len < 0 ? RC_FAIL : len;
}

const char *rc_get_err_msg(void) {
    return rc_err_msg;
}
