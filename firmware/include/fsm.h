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

enum FsmState {FSM_STATE_INIT, FSM_STATE_WIFI_CONNECTION, FSM_STATE_INIT_CONFIG, FSM_STATE_FETCH, FSM_STATE_DISPLAY_UPDATE, FSM_STATE_ERROR, FSM_STATE_LEN};

void fsm_init(void);

/*!
 * \brief           get the current state of the FSM.
 *
 * \return          state of the FSM
 */
enum FsmState fsm_get_state(void);

const char* fsm_get_state_name(void);

/*!
 * \brief           update the current state of the FSM.
 *
 * \param[out]      data: pointer to state data.           
 * \return          RC_OK: state of the FSM
 */
int fsm_update(void * data);


#endif /*! FSM_H */
