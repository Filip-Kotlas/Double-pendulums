#pragma once

#include "Pendulum_system.hpp"

#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <chrono>

class RungeKutta
{
    private:
        std::vector<double> k1;
        std::vector<double> k2;
        std::vector<double> k3;
        std::vector<double> k4;
        std::vector<double> aux;

        double time_step;
        double integration_step;
        PendulumSystem *current_system;

    
    public:
        void set_up(PendulumSystem *system, double time_step, double integration_step);
        void solve(double time_max, std::string output_folder);
        void integrate_step(double time_max);
};