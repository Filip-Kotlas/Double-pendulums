#include <System.hpp>

const std::vector<double>& System::get_state_history(int number) {
    if (state_history.empty()) {
        throw std::runtime_error("State history is empty");
    }
    if (number >= state_history.size()) {
        std::stringstream message;
        message << "The argument number is larger than the size of state history." << number;
        throw std::invalid_argument(message.str());
    }

    return state_history[number];
}

const std::vector<double>& System::get_state_history(double time) {
    if (time_step == 0) {
        throw std::logic_error("Time step was not set.");
    }
    return get_state_history(std::round(time/time_step));
}
