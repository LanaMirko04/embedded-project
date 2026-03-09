#include "fsm.h"
#include "result.h"

#include <cstddef>
#include <string_view>

const bool Fsm::transition_matrix[static_cast<std::size_t>(State::COUNT)][static_cast<std::size_t>(State::COUNT)] = {
    { false, true, false, false, false, false },
    { false, false, true, false, false, true },
    { false, false, false, true, true, true },
    { false, false, false, false, true, true },
    { false, false, false, true, true, true },
    { false, false, true, true, true, true },
};

Fsm &Fsm::get_instance() {
    static Fsm instance;
    return instance;
}

constexpr std::string_view Fsm::state_to_str(Fsm::State state) {
    switch (state) {
        case Fsm::State::NO_STATE:
            return "NO_STATE";

        case Fsm::State::INIT:
            return "INIT";

        case Fsm::State::WIFI_CONNECTION:
            return "WIFI_CONNECTION";

        case Fsm::State::FETCH_CONFIG:
            return "FETCH_CONFIG";

        case Fsm::State::UPDATE_VIEW:
            return "UPDATE_VIEW";

        case Fsm::State::ERROR:
            return "ERROR";

        case Fsm::State::COUNT: /*! This state does not exist (used for counting) */
        default:                /*! To avoid warnings and undefined behavior (should never happen) */
            return "UNKNOWN";
    }
}

Result Fsm::register_action(Fsm::State state, Fsm::StateAction action) {
    if (state == Fsm::State::NO_STATE || state == Fsm::State::COUNT) {
        result_set_err_msg("You cannot register an action for state %s", Fsm::state_to_str(state).data());
        return Result::INVALID_ARGUMENT;
    }

    if (this->actions[static_cast<std::size_t>(state)] != nullptr) {
        result_set_err_msg("Action for state %s already registered", Fsm::state_to_str(state).data());
        return Result::ACTION_ALREADY_REGISTERED;
    }

    this->actions[static_cast<std::size_t>(state)] = action;
    return Result::SUCCESS;
}

Fsm::State Fsm::get_curr_state(void) {
    return this->curr_state;
}

Result Fsm::transition_to(Fsm::State next_state) {
    if (next_state == Fsm::State::NO_STATE || next_state == Fsm::State::COUNT) {
        result_set_err_msg("Invalid state %s", Fsm::state_to_str(next_state).data());
        return Result::INVALID_ARGUMENT;
    }

    if (this->is_valid_transition(next_state)) {
        result_set_err_msg("Transition from state %s to state %s not allowed", Fsm::state_to_str(this->curr_state).data(), Fsm::state_to_str(next_state).data());
        return Result::INVALID_NEXT_STATE;
    }

    if (this->actions[static_cast<std::size_t>(next_state)] == nullptr) {
        result_set_err_msg("Action for state %s not registered", Fsm::state_to_str(next_state).data());
        return Result::ACTION_NOT_REGISTERED;
    }

    this->curr_state = next_state;
    return this->actions[static_cast<std::size_t>(next_state)]();
}

Fsm::Fsm(void) : curr_state(Fsm::State::NO_STATE) {
}

bool Fsm::is_valid_transition(Fsm::State next_state) {
    return transition_matrix[static_cast<std::size_t>(this->curr_state)][static_cast<std::size_t>(next_state)];
}
