/*!
 * \file            fsm.h
 * \date            2025-10-27
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [mirko.lana@studenti.unitn.it]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           FSM Module.
 */

#ifndef FSM_H
#define FSM_H

#include "result.h"

#include <array>
#include <string_view>

/*!
 * \class           Fsm
 * \brief           Finite State Machine (FSM) class.
 */
class Fsm {
  public:
    /*!
     * \brief          Type alias for a state action.
     */
    using StateAction = Result (*)(void);

    /*!
     * \brief           An enumeration with all the possible FSM states.
     */
    enum class State {
        NO_STATE,        /*!< No state (used as initial state) */
        INIT,            /*!< Board initialization */
        WIFI_CONNECTION, /*!< Connecting to WiFi network */
        FETCH_CONFIG,    /*!< Fetching configuration */
        UPDATE_VIEW,     /*!< Updating the current view and relative model */
        ERROR,           /*!< Error state */
        COUNT            /*!< Number of states (internal use only, specifically for arrays) */
    };

    /*!
     * \brief           Deleted copy constructor.
     */
    Fsm(const Fsm &) = delete;

    /*!
     * \brief           Deleted assignment operator.
     */
    Fsm &operator=(const Fsm &) = delete;

    /*!
     * \brief           Get the instance of the FSM.
     *
     * \retval          Fsm& The instance of the FSM.
     */
    static Fsm &get_instance(void);

    /*!
     * \brief           Convert a state to a string.
     *
     * \param[in]       state The state to convert.
     *
     * \retval          std::string_view The string representation of the state.
     */
    static constexpr std::string_view state_to_str(State state);

    /*!
     * \brief           Register an action for a state.
     *
     * \param[in]       state The state to register the action for.
     * \param[in]       action The action to register.
     *
     * \retval          Result SUCCESS if the action was registered successfully, an error otherwise.
     *                  - INVALID_ARG if the state is either NO_STATE or COUNT (internal use only).
     *                  - ACTION_ALREADY_REGISTERED if the action is already registered for the state.
     */
    Result register_action(State state, StateAction action);

    /*!
     * \brief           Get the current state of the FSM.
     *
     * \retval          State The current state of the FSM.
     */
    State get_curr_state(void);

    /*!
     * \brief           Transition to a new state.
     *
     * \param[in]       state The state to transition to.
     *
     * \retval          Result SUCCESS if the transition was successful, an error otherwise.
     *                  - INVALID_ARG if the state is either NO_STATE or COUNT (internal use only).
     *                  - ACTION_NOT_REGISTERED if the action for the state is not registered.
     */
    Result transition_to(State state);

  private:
    static const bool transition_matrix[static_cast<std::size_t>(State::COUNT)][static_cast<std::size_t>(State::COUNT)];

    std::array<StateAction, static_cast<std::size_t>(State::COUNT)> actions; /*! Array of state actions */
    State curr_state;                                                        /*!< Current state of the FSM */

    /*!
     * \brief           Deleted default constructor.
     */
    Fsm(void);

    bool is_valid_transition(State next_state);
};

#endif /*! FSM_H */
