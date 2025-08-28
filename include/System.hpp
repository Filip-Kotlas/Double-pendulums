#pragma once
#include <vector>

class System
{
    protected:
        int degrees_of_freedom;
        double time;
        std::vector<double> u;
        std::vector<std::vector<double>> state_history;

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
            return u;
        }

        virtual void get_right_hand_side(const double time, const std::vector<double>& u, std::vector<double>& right_hand_side) = 0;
        virtual void set_initial_conditions(const double time) = 0;
        virtual void write_state_to_file(int number) = 0;
        virtual void record_state() = 0;
};