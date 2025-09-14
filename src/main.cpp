
#include <iostream>
#include <string>

#include "System.hpp"
#include "Pendulum_system.hpp"
#include "RungeKutta.hpp"

constexpr double PI = 3.141592653589793;

int main() {
    
    std::array<double, 4> bounds = {-PI, PI, -PI, PI};
    int size_x = 256;
    int size_y = 256;
    float show_time = 0;
    float max_time = 1.0;
    double integration_step = 0.001;
    PendulumSystem system(size_x, size_y, bounds, 1.0, 1.0, 1.0, 1.0);
    std::string txt_file_name = "";
    RungeKutta solver;
    int step_count = 100;
    double time_step = max_time / step_count;

    solver.set_up(&system, time_step, integration_step);
    solver.solve(max_time);
    system.write_state_to_file(max_time, "results");

    return 0;
}
