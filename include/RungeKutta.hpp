#pragma once

#include "Pendulum_system.hpp"
#include "System.hpp"

#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <chrono>
#include <atomic>

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
        System *current_system;
        float* progress_ptr = nullptr;
        std::atomic<bool>* cancel_ptr = nullptr;

    
    public:
        void set_up(System *system, double time_step, double integration_step, float* progress, std::atomic<bool>* cancel);
        void solve(double time_max);
        void integrate_step(double time_max);
        void print_progress(int step, int steps_count,
                    std::chrono::high_resolution_clock::time_point start,
                    std::chrono::high_resolution_clock::time_point step_start,
                    std::chrono::high_resolution_clock::time_point step_end);

};