/*!
 * \file            result.h
 * \date            2025-12-19
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Return codes and error handling.
 */

#ifndef RESULT_H
#define RESULT_H

#include <cstddef>
#include <string_view>

/*!
 * \brief           Error message buffer length.
 */
constexpr std::size_t RESULT_MESSAGE_SIZE = 128U;

/*!
 * \brief           An enum class representing all possible retval values.
 */
enum class Result {
    /*!
     * \defgroup      CommonResultCodes Common Result Codes
     * \{
     */
    SUCCESS,                 /*!< Everything's OK */
    UNKNOWN_ERROR,           /*!< Unknown error */
    INVALID_ARGUMENT,        /*!< Invalid argument passed to function */
    WRONG_SIZE,              /*!< Wrong size */
    UNEXPECTED_NULL_POINTER, /*!< Unexpected NULL pointer */
    IO_ERROR,                /*!< I/O error */
    NOT_FOUND,               /*!< When a resource is not found */

    /*!
     * \}
     */

    /*!
     * \defgroup      FSMResultCodes FSM Result Codes
     * \{
     */

    ACTION_ALREADY_REGISTERED, /*!< Action already registered */
    ACTION_NOT_REGISTERED,     /*!< Action not registered */
    INVALID_NEXT_STATE,        /*!< Invalid FSM next state */

    /*!
     * \}
     */

    /*!
     * \defgroup      NetworkRelatedResultCodes Network related Result Codes
     * \{
     */

    SYNC_FAILED,              /*!< Synchronization failed for some reason */

    /*!
     * \}
     */

    /*!
     * \defgroup      ApiRelatedReturnCodes API related Return Codes
     * \{
     */

    API_STATUS_OK,  /*!< API status OK */
    API_STATUS_DOWN /*!< API status down */

    /*!
     * \}
     */
};

/*!
 * \brief           Set the error message.
 *
 * \param[in]       fmt Format string.
 * \param[in]       ... variable arguments.
 * \retval          Result SUCCESS on success, an error code otherwise.
 *                  - UNKNOWN_ERROR if an error occurs while.
 */
Result result_set_err_msg(const char *fmt, ...);

/*!
 * \brief           Get the error message.
 *
 * \retval          char* The pointer to the error message.
 */
const char *result_get_err_msg(void);

/*!
 * \brief           Convert a Result enum value to a string.
 *
 * \param[in]       result The Result enum value.
 * \retval          char* The pointer to the Result enum string.
 */
const char *result_to_str(Result result);

#endif /*! RESULT_H */
