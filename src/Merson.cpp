#include "Merson.hpp"

void Merson::set_up(System *system, double time_step, double integration_step)
{
    int dof = system->get_degrees_of_freedom();
    k1.resize(dof, 0);
    k2.resize(dof, 0);
    k3.resize(dof, 0);
    k4.resize(dof, 0);
    aux.resize(dof, 0);

    this->time_step = time_step;
    this->integration_step = integration_step;
    this->current_system = system;
}

void Merson::solve(double time_max)
{
    int steps_count = std::ceil((time_max - this->current_system->get_time())/time_step);
    this->current_system->record_state();
    auto clock_computation_start = std::chrono::high_resolution_clock::now();

    for(int k = 0; k <= steps_count; k++){
        auto clock_step_start = std::chrono::high_resolution_clock::now();
        this->integrate_step(time_max);
        this->current_system->record_state();
        auto clock_step_end = std::chrono::high_resolution_clock::now();

        double time_per_step = (std::chrono::duration<double>(clock_step_end - clock_step_start).count());
        double remaining_time = time_per_step * (steps_count - k);
        int hours_remaining = static_cast<int>(remaining_time) / 3600;
        int minutes_remaining = (static_cast<int>(remaining_time) % 3600) / 60;
        int seconds_remaining = remaining_time - (hours_remaining * 3600 + minutes_remaining * 60);

        double elapsed_time = (std::chrono::duration<double>(clock_step_end - clock_step_start).count());
        int hours_elapsed = static_cast<int>(elapsed_time) / 3600;
        int minutes_elapsed = (static_cast<int>(elapsed_time) % 3600) / 60;
        int seconds_elapsed = elapsed_time - (hours_remaining * 3600 + minutes_remaining * 60);

        std::cout << "Steps completed: " << k << " / " << steps_count << " => " << std::fixed
                  << std::setprecision(2) << (double) k / (double) steps_count * 100.0 << "% ";
        std::cout << "     Time elapsed: " << hours_elapsed << "h " << minutes_elapsed << "m "
                  << seconds_elapsed << "s";
        std::cout << "     Time remaining: " << hours_remaining << "h " << minutes_remaining << "m "
                  << seconds_remaining << "s" << std::endl;
    }
}
void Merson::integrate_step(double time_max)
{
    double start_time = current_system->get_time();
    while(current_system->get_time() <= std::min(time_max, start_time + this->time_step)){

        // Computing k1
        current_system->get_right_hand_side(current_system->get_time(),
                                            current_system->get_state(),
                                            k1);
        
        // Computing k2
        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            aux[i] = current_system->get_state()[i] + 1.0/2 * this->integration_step * k1[i];
        }
        current_system->get_right_hand_side(current_system->get_time() + 1.0/2*integration_step,
                                            aux,
                                            k2);
        
        // Computing k3
        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            aux[i] = current_system->get_state()[i] + 1.0/2 * this->integration_step * k2[i];
        }
        current_system->get_right_hand_side(current_system->get_time() + 1.0/2*integration_step,
                                            aux,
                                            k3);

        // Computing k4
        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            aux[i] = current_system->get_state()[i] + this->integration_step * k3[i];
        }
        current_system->get_right_hand_side(current_system->get_time() + integration_step,
                                            aux,
                                            k4);

        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            current_system->get_state()[i] += 1.0/6 * integration_step * (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]);
        }
        current_system->increase_time(integration_step);

    }
}
