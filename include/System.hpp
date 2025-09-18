#pragma once
#include <vector>
#include <map>
#include <cmath>
#include <string>

template<typename Derived, typename Real = double>
class BaseSystem
{
    protected:
        int degrees_of_freedom;
        Real time;
        std::vector<Real> state;
        std::map<Real, std::vector<Real>> state_history;

        const std::vector<double>& get_state_history(Real time) const {
            if (state_history.empty()) {
                throw std::runtime_error("State history is empty.");
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

    public:
        int get_degrees_of_freedom() const {
            return this->degrees_of_freedom;
        };
        Real get_time() const {
            return this->time;
        };
        void increase_time(Real time_increase){
            this->time += time_increase;
        }
        const std::vector<Real>& get_state() const {
            return this->state;
        }
        std::vector<Real>& get_state() {
            return this->state;
        }


        void get_right_hand_side(const Real time, const std::vector<Real>& state, std::vector<Real>& right_hand_side) {
            static_cast<Derived*>(this)->get_right_hand_side(time, state, right_hand_side);
        }
        void set_initial_conditions(const Real time) {
            static_cast<Derived*>(this)->set_initial_conditions(time);
        }
        void write_state_to_file(double save_time, std::string folder_name) {
            static_cast<Derived*>(this)->write_state_to_file(save_time, folder_name);
        }
        void record_state() {
            static_cast<Derived*>(this)->record_state();
        }
};