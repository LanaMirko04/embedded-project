#include "rc.h"
#include "fsm.h"

/*!
 * \brief           Type definition for FSM transition functions.
 *
 * \param[in]       next_state: The next FSM state.
 * \return          RC_OK on success, an error code otherwise:
 *                   - RC_FSM_ERR_INVALID_NEXT_STATE if the given state is not
 *                     related to the current one.
 */
typedef int (*fsm_trans_fun)(enum FsmState next_state);

static const char *FSM_STATE_NAMES[FSM_STATE_LEN] = {
    "FSM_STATE_INIT",
    "FSM_STATE_WIFI_CONNECTION",
    "FSM_STATE_CHECK_CONFIG",
    "FSM_STATE_LOAD_CONFIG",
    "FSM_STATE_DISPLAY_UPDATE",
    "FSM_STATE_ERROR"
}; /*!< Array of state names for debugging purposes */

static enum FsmState fsm_current_state;              /*!< Current FSM state */
static fsm_trans_fun fsm_trans_array[FSM_STATE_LEN]; /*!< Array of transition functions */

/*!
 * \brief           Transition function for the INIT state.
 */
static int prv_fsm_init_trans_fun(enum FsmState next_state) {
    switch (next_state) {
        case FSM_STATE_INIT:
        case FSM_STATE_ERROR:
        case FSM_STATE_WIFI_CONNECTION:
        case FSM_STATE_CHECK_CONFIG:
            fsm_current_state = next_state;
            break;
        default:

            rc_set_err_msg("The state %s is not reachable from %s state.",
                           fsm_get_state_name(next_state),
                           fsm_get_state_name(fsm_current_state));
            return RC_FSM_ERR_INVALID_NEXT_STATE;
    }
    return RC_OK;
}

/*!
 * \brief           Transition function for the WIFI_CONNECTION state.
 */
static int prv_fsm_wifi_connection_trans_fun(enum FsmState next_state) {
    switch (next_state) {
        case FSM_STATE_CHECK_CONFIG:
        case FSM_STATE_WIFI_CONNECTION:
        case FSM_STATE_ERROR:
            fsm_current_state = next_state;
            break;
        default:

            rc_set_err_msg("The state %s is not reachable from %s state.",
                           fsm_get_state_name(next_state),
                           fsm_get_state_name(fsm_current_state));
            return RC_FSM_ERR_INVALID_NEXT_STATE;
    }
    return RC_OK;
}

/*!
 * \brief           Transition function for the CHECK_CONFIG state.
 */
static int prv_fsm_check_config_trans_fun(enum FsmState next_state) {
    switch (next_state) {
        case FSM_STATE_ERROR:
        case FSM_STATE_LOAD_CONFIG:
            fsm_current_state = next_state;
            break;
        default:

            rc_set_err_msg("The state %s is not reachable from %s state.",
                           fsm_get_state_name(next_state),
                           fsm_get_state_name(fsm_current_state));
            return RC_FSM_ERR_INVALID_NEXT_STATE;
    }
    return RC_OK;
}

/*!
 * \brief           Transition function for the LOAD_CONFIG state.
 */
static int prv_fsm_load_config_trans_fun(enum FsmState next_state) {
    switch (next_state) {
        case FSM_STATE_ERROR:
        case FSM_STATE_DISPLAY_UPDATE:
            fsm_current_state = next_state;
            break;

        default:
            rc_set_err_msg("The state %s is not reachable from %s state.",
                           fsm_get_state_name(next_state),
                           fsm_get_state_name(fsm_current_state));
            return RC_FSM_ERR_INVALID_NEXT_STATE;
    }
    return RC_OK;
}

/*!
 * \brief           Transition function for the DISPLAY_UPDATE state.
 */
static int prv_fsm_display_update_trans_fun(enum FsmState next_state) {
    switch (next_state) {
        case FSM_STATE_ERROR:
        case FSM_STATE_DISPLAY_UPDATE:
        case FSM_STATE_INIT:
            fsm_current_state = next_state;
            break;

        default:
            rc_set_err_msg("The state %s is not reachable from %s state.",
                           fsm_get_state_name(next_state),
                           fsm_get_state_name(fsm_current_state));
            return RC_FSM_ERR_INVALID_NEXT_STATE;
    }
    return RC_OK;
}

/*!
 * \brief           Transition function for the ERROR state.
 */
static int prv_fsm_error_trans_fun(enum FsmState next_state) {
    switch (next_state) {
        case FSM_STATE_ERROR:
        case FSM_STATE_INIT:
            fsm_current_state = next_state;
            break;

        default:
            rc_set_err_msg("The state %s is not reachable from %s state.",
                           fsm_get_state_name(next_state),
                           fsm_get_state_name(fsm_current_state));
            return RC_FSM_ERR_INVALID_NEXT_STATE;
    }
    return RC_OK;
}

void fsm_init(void) {
    fsm_current_state = FSM_STATE_INIT;

    fsm_trans_array[FSM_STATE_INIT] = prv_fsm_init_trans_fun;
    fsm_trans_array[FSM_STATE_WIFI_CONNECTION] = prv_fsm_wifi_connection_trans_fun;
    fsm_trans_array[FSM_STATE_CHECK_CONFIG] = prv_fsm_check_config_trans_fun;
    fsm_trans_array[FSM_STATE_LOAD_CONFIG] = prv_fsm_load_config_trans_fun;
    fsm_trans_array[FSM_STATE_DISPLAY_UPDATE] = prv_fsm_display_update_trans_fun;
    fsm_trans_array[FSM_STATE_ERROR] = prv_fsm_error_trans_fun;
}

enum FsmState fsm_get_state(void) {
    return fsm_current_state;
}

const char *fsm_get_state_name(enum FsmState state) {
    return FSM_STATE_NAMES[state];
}

int fsm_update(enum FsmState next_state) {
    return fsm_trans_array[fsm_current_state](next_state);
}
