#pragma once
#include <vector>
#include <map>
#include <cmath>

class System
{
    protected:
        int degrees_of_freedom;
        double time;
        std::vector<double> state;
        std::map<double, std::vector<double>> state_history;

        const std::vector<double>& get_state_history(double time);

    public:
        double get_degrees_of_freedom(){
            return degrees_of_freedom;
        };
        double get_time(){
            return time;
        };
        void increase_time(double time_increase){
            time += time_increase;
        }
        std::vector<double>& get_state(){
            return state;
        }

        virtual void get_right_hand_side(const double time, const std::vector<double>& state, std::vector<double>& right_hand_side) = 0;
        virtual void set_initial_conditions(const double time) = 0;
        virtual void write_state_to_file(int number) = 0;
        virtual void record_state() = 0;
};