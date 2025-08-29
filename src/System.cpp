#include "System.hpp"
#include <stdexcept>
#include <cmath>
#include <map>

const std::vector<double>& System::get_state_history(double time) {
    if (state_history.empty()) {
        throw std::runtime_error("State history is empty");
    }

    auto lower_bound = this->state_history.lower_bound(time);

    if (lower_bound == state_history.begin())
        return lower_bound->second;
    else if (lower_bound == state_history.end())
        return std::prev(lower_bound)->second;
    else {
        auto previous = std::prev(lower_bound);
        if (std::abs(previous->first - time) < std::abs(lower_bound->first - time))
            return previous->second;
        else
            return lower_bound->second;
    }
}