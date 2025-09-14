#pragma once

#include "System.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>

class PendulumSystem : public System
{
    private:
        int size_x;
        int size_y;
        double mass_1;
        double mass_2;
        double length_1;
        double length_2;
        std::array<double, 4> bounds;

        double get_phi_1(int i, int j){
            return state[(j*size_x + i)*4];
        };
        double get_phi_2(int i, int j){
            return state[(j*size_x + i)*4 + 1];
        };
        double get_der_phi_1(int i, int j){
            return state[(j*size_x + i)*4 + 2];
        }
        double get_der_phi_2(int i, int j){
            return state[(j*size_x + i)*4 + 3];
        }

        double get_phi_1(int i, int j, int number){
            std::cout << "phi_1" << std::endl;
            return get_state_history(number)[(j*size_x + i)*4];
        };
        double get_phi_2(int i, int j, int number){
            std::cout << "phi_2" << std::endl;
            return get_state_history(number)[(j*size_x + i)*4 + 1];
        };
        double get_der_phi_1(int i, int j, int number){
            return get_state_history(number)[(j*size_x + i)*4 + 2];
        }
        double get_der_phi_2(int i, int j, int number){
            return get_state_history(number)[(j*size_x + i)*4 + 3];
        }

        void set_phi_1(int i, int j, double value){
            state[(j*size_x + i)*4] = value;
        }
        void set_phi_2(int i, int j, double value){
            state[(j*size_x + i)*4 + 1] = value;
        }
        void set_der_phi_1(int i, int j, double value){
            state[(j*size_x + i)*4 + 2] = value;
        }
        void set_der_phi_2(int i, int j, double value){
            state[(j*size_x + i)*4 + 3] = value;
        }

    public:
        // constructor
        PendulumSystem(int size_x,
                    int size_y,
                    std::array<double, 4>& bounds,
                    double mass_1,
                    double mass_2,
                    double length_1,
                    double length_2)
        : size_x(size_x),
        size_y(size_y),
        bounds(bounds),
        mass_1(mass_1),
        mass_2(mass_2),
        length_1(length_1),
        length_2(length_2)
        {
            this->degrees_of_freedom = size_x * size_y * 4;
            this->time = 0;
            state.resize(this->degrees_of_freedom, 0);
            this->set_initial_conditions(time);
        }

        std::array<int, 2> get_size(){
            return {this->size_x, this->size_y};
        }

        void set_time_step(double time_step) {
            this->time_step = time_step;
        }

        void get_right_hand_side(const double time, const std::vector<double>& state, std::vector<double>& right_hand_side);
        void set_initial_conditions(const double time);
        void write_state_to_file(int number, std::string folder_name);
        void record_state();
        void save_history_to_folder(std::string folder_name);

        double get_phi_1(int i, int j, double time){
            return get_state_history(time)[(j*size_x + i)*4];
        };
        double get_phi_2(int i, int j, double time){
            return get_state_history(time)[(j*size_x + i)*4 + 1];
        };
        double get_der_phi_1(int i, int j, double time){
            return get_state_history(time)[(j*size_x + i)*4 + 2];
        }
        double get_der_phi_2(int i, int j, double time){
            return get_state_history(time)[(j*size_x + i)*4 + 3];
        }

};