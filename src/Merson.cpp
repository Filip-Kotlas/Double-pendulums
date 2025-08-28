#include "Merson.hpp"

void Merson::set_up(System *system, double time_step, double integration_step)
{
    int dof = system->get_degrees_of_freedom();
    std::vector<double> k1(dof);
    std::vector<double> k2(dof);
    std::vector<double> k3(dof);
    std::vector<double> k4(dof);
    std::vector<double> aux(dof);

    this->time_step = time_step;
    this->integration_step = integration_step;
    this->current_system = system;
}

void Merson::solve(double time_max)
{
    int steps_count = std::ceil((time_max - this->current_system->get_time())/time_step);
    this->current_system->record_state();

    for(int k = 0; k <= steps_count; k++){
        this->integrate_step(time_max);
        this->current_system->record_state();
        std::cout << "Steps completed: " << k << " / " << steps_count << " => " << std::fixed
                  << std::setprecision(2) << (double) k / (double) steps_count * 100.0 << "% ";
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
