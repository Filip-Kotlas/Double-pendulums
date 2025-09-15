#pragma once
#include <vector>
#include <map>
#include <cmath>
#include <string>

#include "CudaArray.hpp"

class System
{
    protected:
        int degrees_of_freedom;
        double time;
        CudaArray cuda_state;
        std::vector<double> host_state;
        std::map<double, std::vector<double>> state_history;

        const std::vector<double>& get_state_history(double time);

    public:
        System() = default;

        double get_degrees_of_freedom(){
            return degrees_of_freedom;
        };
        double get_time(){
            return time;
        };
        void increase_time(double time_increase){
            time += time_increase;
        }
        std::vector<double>& get_host_state(){
            return host_state;
        }
        CudaArray& get_cuda_state(){
            return cuda_state;
        };

        virtual void get_right_hand_side(const double time, const CudaArray &state, CudaArray &right_hand_side) = 0;
        virtual void set_initial_conditions(const double time) = 0;
        virtual void write_state_to_file(double save_time, std::string folder_name) = 0;
        virtual void record_state() = 0;
        virtual void copy_data_from_device_to_host() = 0;
};