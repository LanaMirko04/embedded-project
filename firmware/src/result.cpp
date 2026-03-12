/*!
 * \file            result.c
 * \date            2025-12-19
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Return codes and error handling.
 */

#include "result.h"

#include <cstdio>
#include <cstdarg>
#include <array>
#include <string_view>

/*!
 * \brief           The error message buffer.
 */
static std::array<char, RESULT_MESSAGE_SIZE> return_err_msg;

Result result_set_err_msg(const char *fmt, ...) {
    if (!fmt)
        return Result::INVALID_ARGUMENT;

    std::va_list args;
    va_start(args, fmt);

    return_err_msg.fill('\0');
    int len = vsnprintf(return_err_msg.data(), return_err_msg.size(), fmt, args);

    va_end(args);

    return len < 0 ? Result::UNKNOWN_ERROR : Result::SUCCESS;
}

const char *result_get_err_msg(void) {
    return return_err_msg.data();
}

const char *result_to_str(Result result) {
    switch (result) {
        case Result::SUCCESS:
            return "SUCCESS";

        case Result::INVALID_ARGUMENT:
            return "INVALID_ARGUMENT";

        case Result::WRONG_SIZE:
            return "WRONG_SIZE";

        case Result::UNEXPECTED_NULL_POINTER:
            return "UNEXPECTED_NULL_POINTER";

        case Result::ACTION_ALREADY_REGISTERED:
            return "ACTION_ALREADY_REGISTERED";

        case Result::ACTION_NOT_REGISTERED:
            return "ACTION_NOT_REGISTERED";

        case Result::INVALID_NEXT_STATE:
            return "INVALID_NEXT_STATE";

        case Result::UNKNOWN_ERROR:
        default:
            return "UNKNOWN_ERROR";
    }
}
