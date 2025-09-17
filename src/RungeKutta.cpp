#include "RungeKutta.hpp"

void RungeKutta::set_up(System *system, double time_step, double integration_step, float* progress, std::atomic<bool>* cancel)
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
    this->progress_ptr = progress;
    this->cancel_ptr = cancel;
}

void RungeKutta::solve(double time_max)
{
    int steps_count = std::ceil((time_max - this->current_system->get_time())/time_step);
    this->current_system->record_state();
    
    for (int k = 0; k <= steps_count; k++) {
        if (cancel_ptr && cancel_ptr->load())
            break;

        this->integrate_step(time_max);
        this->current_system->record_state();

        if (progress_ptr) {
            *progress_ptr = float(k) / float(steps_count);
        }
    }
}

void RungeKutta::integrate_step(double time_max)
{
    double start_time = current_system->get_time();
    while (this->integration_step <= start_time + this->time_step - current_system->get_time()) {
        double tau = std::min(this->integration_step, start_time + this->time_step - current_system->get_time());

        // Computing k1
        current_system->get_right_hand_side(current_system->get_time(),
                                            current_system->get_state(),
                                            k1);
        
        // Computing k2
        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            aux[i] = current_system->get_state()[i] + 1.0/2 * tau * k1[i];
        }
        current_system->get_right_hand_side(current_system->get_time() + 1.0/2*tau,
                                            aux,
                                            k2);
        
        // Computing k3
        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            aux[i] = current_system->get_state()[i] + 1.0/2 * tau * k2[i];
        }
        current_system->get_right_hand_side(current_system->get_time() + 1.0/2*tau,
                                            aux,
                                            k3);

        // Computing k4
        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            aux[i] = current_system->get_state()[i] + tau * k3[i];
        }
        current_system->get_right_hand_side(current_system->get_time() + tau,
                                            aux,
                                            k4);

        for(int i = 0; i < current_system->get_degrees_of_freedom(); i++){
            current_system->get_state()[i] += 1.0/6 * tau * (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]);
        }
        current_system->increase_time(tau);
    }
}