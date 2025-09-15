#include "RungeKutta.hpp"

void RungeKutta::set_up(System *system, double time_step, double integration_step)
{
    int dof = system->get_degrees_of_freedom();
    k1.resize(dof);
    k2.resize(dof);
    k3.resize(dof);
    k4.resize(dof);
    aux.resize(dof);

    this->time_step = time_step;
    this->integration_step = integration_step;
    this->current_system = system;
}

void RungeKutta::solve(double time_max, std::string output_folder)
{
    int steps_count = std::round((time_max - this->current_system->get_time())/this->time_step);
    this->current_system->record_state();
    auto clock_computation_start = std::chrono::high_resolution_clock::now();
    this->current_system->write_state_to_file(this->current_system->get_time(), this->time_step, output_folder);

    for(int k = 1; k <= steps_count; k++){
        auto clock_step_start = std::chrono::high_resolution_clock::now();
        this->integrate_step(time_max);
        this->current_system->copy_data_from_device_to_host();
        this->current_system->record_state();
        this->current_system->write_state_to_file(this->current_system->get_time(), time_step, output_folder);
        auto clock_step_end = std::chrono::high_resolution_clock::now();

        double time_per_step = (std::chrono::duration<double>(clock_step_end - clock_step_start).count());
        double remaining_time = time_per_step * (steps_count - k);
        int hours_remaining = static_cast<int>(remaining_time) / 3600;
        int minutes_remaining = (static_cast<int>(remaining_time) % 3600) / 60;
        int seconds_remaining = remaining_time - (hours_remaining * 3600 + minutes_remaining * 60);

        double elapsed_time = (std::chrono::duration<double>(clock_step_end - clock_computation_start).count());
        int hours_elapsed = static_cast<int>(elapsed_time) / 3600;
        int minutes_elapsed = (static_cast<int>(elapsed_time) % 3600) / 60;
        int seconds_elapsed = elapsed_time - (hours_elapsed * 3600 + minutes_elapsed * 60);

        std::cout << "Steps completed: " << k << " / " << steps_count << " => " << std::fixed
                  << std::setprecision(2) << (double) k / (double) steps_count * 100.0 << "% ";
        std::cout << "     Time elapsed: " << hours_elapsed << "h " << minutes_elapsed << "m "
                  << seconds_elapsed << "s";
        std::cout << "     Time remaining: " << hours_remaining << "h " << minutes_remaining << "m "
                  << seconds_remaining << "s" << std::endl;
    }
}

void RungeKutta::integrate_step(double time_max)
{
    double start_time = current_system->get_time();
    int cuda_block_size = 256;
    int cuda_grid_size = 88;
    while (this->integration_step <= start_time + this->time_step - current_system->get_time()) {
        double tau = std::min(this->integration_step, start_time + this->time_step - current_system->get_time());

        // Computing k1
        current_system->get_right_hand_side(current_system->get_time(),
                                            current_system->get_cuda_state(),
                                            k1);
        
        // Computing k2
        cuda_aux_computation<<<cuda_grid_size, cuda_block_size>>>(current_system->get_cuda_state().data(),
                                                                  k1.data(),
                                                                  aux.data(),
                                                                  tau,
                                                                  current_system->get_degrees_of_freedom(),
                                                                  0.5);
        cudaDeviceSynchronize();
        current_system->get_right_hand_side(current_system->get_time() + 1.0/2*tau,
                                            aux,
                                            k2);
        
        // Computing k3
        cuda_aux_computation<<<cuda_grid_size, cuda_block_size>>>(current_system->get_cuda_state().data(),
                                                                  k2.data(),
                                                                  aux.data(),
                                                                  tau,
                                                                  current_system->get_degrees_of_freedom(),
                                                                  0.5);
        cudaDeviceSynchronize();
        current_system->get_right_hand_side(current_system->get_time() + 1.0/2*tau,
                                            aux,
                                            k3);

        // Computing k4
        cuda_aux_computation<<<cuda_grid_size, cuda_block_size>>>(current_system->get_cuda_state().data(),
                                                                  k3.data(),
                                                                  aux.data(),
                                                                  tau,
                                                                  current_system->get_degrees_of_freedom(),
                                                                  1.0);
        cudaDeviceSynchronize();
        current_system->get_right_hand_side(current_system->get_time() + tau,
                                            aux,
                                            k4);

        // Final update
        cuda_final_update<<<cuda_grid_size, cuda_block_size>>>(current_system->get_cuda_state().data(),
                                                                k1.data(),
                                                                k2.data(),
                                                                k3.data(),
                                                                k4.data(),
                                                                tau,
                                                                current_system->get_degrees_of_freedom());
        cudaDeviceSynchronize();
        current_system->increase_time(tau);
    }
}

__global__ void cuda_aux_computation(const double* state, const double* k, double* aux, double tau, int dof, double factor) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for(int i = idx; i < dof; i += stride) {
        if (i < dof) {
            aux[i] = state[i] + factor * tau * k[i];
        }
    }
}

__global__ void cuda_final_update(double* state, const double* k1, const double* k2, const double* k3, const double* k4, double tau, int dof) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    for(int i = idx; i < dof; i += stride) {
        if (i < dof) {
            state[i] += 1.0/6 * tau * (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]);
        }
    }
}