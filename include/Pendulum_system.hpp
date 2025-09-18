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

        // Constructor for subsystem of PenduluSystem with y coordinates starting at start_y and ending at end_y
        PendulumSystem(const PendulumSystem& original, int start_y, int end_y);

        // Constructor from data file.
        PendulumSystem(std::string folder_name);
        void add_state_to_history_from_file(std::string file_name);

        enum class Component {
        phi_1 = 0,
        phi_2 = 1,
        der_phi_1 = 2,
        der_phi_2 = 3
        };

        std::array<int, 2> get_size() const {
            return {this->size_x, this->size_y};
        }

        void get_right_hand_side(const double time, const std::vector<double>& state, std::vector<double>& right_hand_side);
        void set_initial_conditions(const double time);
        void write_state_to_file(double save_time, std::string folder_name);
        void record_state();
        // Sloučení výsledků z podmřížky zpět do hlavního systému
        void merge(const PendulumSystem& part, int start_y);

        // Getter for state in history
        template<Component C>
        double get(int i, int j, double time) const {
            constexpr int offset = static_cast<int>(C);
            return get_state_history(time)[(j*size_x + i)*4 + offset];
        }
        
        
    private:
        int size_x;
        int size_y;
        double mass_1;
        double mass_2;
        double length_1;
        double length_2;
        std::array<double, 4> bounds;
    
    
        // Template setter and getter for phi 1, phi 2 and their derivations
        template<Component C>
        double get(int i, int j) const {
            constexpr int offset = static_cast<int>(C);
            return state[(j * size_x + i) * 4 + offset];
        }
        template<Component C>
        void set(int i, int j, double value) {
            constexpr int offset = static_cast<int>(C);
            state[(j * size_x + i) * 4 + offset] = value;
        }


};    





























