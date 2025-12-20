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
#include <string>

/*!
 * \brief           Error message buffer length.
 */
constexpr std::size_t RESULT_MESSAGE_SIZE = 128U;

/*!
 * \brief           An enum class representing all possible return values.
 */
enum class Result {
    /*!
     * \defgroup      CommonResultCodes Common Result Codes
     * @{
     */

    SUCCESS,                 /*!< Everything's OK */
    UNKNOWN_ERROR,           /*!< Unknown error */
    INVALID_ARGUMENT,        /*!< Invalid argument */
    WRONG_SIZE,              /*!< Wrong size */
    UNEXPECTED_NULL_POINTER, /*!< Unexpected NULL pointer */

    /*!
     * @}
     */

    /*!
      * \defgroup      FSMResultCodes FSM Result Codes
      * @{
      */

    INVALID_NEXT_STATE, /*!< Invalid FSM next state */

    /*!
     * @}
     */
};

/*!
 * \brief           Set the error message.
 *
 * \param[in]       fmt Format string.
 * \param[in]       ... variable arguments.
 * \return          Result SUCCESS on success, an error code otherwise.
 *                  - UNKNOWN_ERROR if an error occurs while.
 */
Result result_set_err_msg(const char *fmt, ...);

/*!
 * \brief           Get the error message.
 *
 * \return          char* The pointer to the error message.
 */
const char *result_get_err_msg(void);

/*!
 * \brief           Convert a Result enum value to a string.
 *
 * \param[in]       result The Result enum value.
 * \return          std::string The string representation of the Result enum value.
 */
std::string result_to_str(Result result);

#endif /*! RESULT_H */
