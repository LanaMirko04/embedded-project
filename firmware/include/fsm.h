/*!
 * \file            fsm.h
 * \date            2025-10-27
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           FSM Module.
 */

#ifndef FSM_H
#define FSM_H

/*!
 * \brief           An enumeration with all the possible FSM states.
 */
enum FsmState {
    FSM_STATE_INIT,            /*!< Initialization state */
    FSM_STATE_WIFI_CONNECTION, /*!< Connecting to WiFi network */
    FSM_STATE_CHECK_CONFIG,    /*!< Checking configuration presence or "version" */
    FSM_STATE_LOAD_CONFIG,     /*!< Loading/fetching configuration */
    FSM_STATE_DISPLAY_UPDATE,  /*!< Updating the display and relative models */
    FSM_STATE_ERROR,           /*!< Error state */
    FSM_STATE_LEN              /*!< Length of the enum, not a real state */
};

/*!
 * \brief           Initialize the FSM.
 */
void fsm_init(void);

/*!
 * \brief           Get the current state of the FSM.
 *
 * \return          The state of the FSM.
 */
inline enum FsmState fsm_get_state(void);

/*!
 * \brief           Get the given state name.
 *
 * \param[in]       state: The state to get the name for.
 * \return          The name of the given state.
 */
inline const char *fsm_get_state_name(enum FsmState state);

/*!
 * \brief           Update the current state of the FSM.
 *
 * \param[out]      next_state: The next FSM state.
 * \return          RC_OK on success, an error code otherwise:
 *                   - RC_FSM_ERR_INVALID_NEXT_STATE  if the given state is not
 *                     relat  to the current one.
 */
int fsm_update(enum FsmState next_state);

#endif /*! FSM_H */
