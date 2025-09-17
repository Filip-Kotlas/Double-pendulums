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

        double get_phi_1(int i, int j) const {
            return state[(j*size_x + i)*4];
        };
        double get_phi_2(int i, int j) const {
            return state[(j*size_x + i)*4 + 1];
        };
        double get_der_phi_1(int i, int j) const {
            return state[(j*size_x + i)*4 + 2];
        }
        double get_der_phi_2(int i, int j) const {
            return state[(j*size_x + i)*4 + 3];
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
            time = 0;
            state.resize(this->degrees_of_freedom, 0);
            this->set_initial_conditions(time);
        }

        // Konstruktor pro subgrid (kopie části původního systému)
        PendulumSystem(const PendulumSystem& original, int start_y, int end_y);

        // construction from data in txt file
        PendulumSystem(std::string folder_name);
        void add_state_to_history_from_file(std::string file_name);

        std::array<int, 2> get_size() const {
            return {this->size_x, this->size_y};
        }

        void get_right_hand_side(const double time, const std::vector<double>& state, std::vector<double>& right_hand_side);
        void set_initial_conditions(const double time);
        void write_state_to_file(double save_time, std::string folder_name);
        void record_state();
        // Sloučení výsledků z podmřížky zpět do hlavního systému
        void merge(const PendulumSystem& part, int start_y);


        double get_phi_1(int i, int j, double time) const {
            return get_state_history(time)[(j*size_x + i)*4];
        }
        double get_phi_2(int i, int j, double time) const {
            return get_state_history(time)[(j*size_x + i)*4 + 1];
        }
        double get_der_phi_1(int i, int j, double time) const {
            return get_state_history(time)[(j*size_x + i)*4 + 2];
        }
        double get_der_phi_2(int i, int j, double time) const {
            return get_state_history(time)[(j*size_x + i)*4 + 3];
        }
        
};