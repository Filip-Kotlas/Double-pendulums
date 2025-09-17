#include "Pendulum_system.hpp"

void PendulumSystem::get_right_hand_side(const double time, const CudaArray &state, CudaArray &right_hand_side)
{
    dim3 grid_size = 256;
    dim3 block_size = 256;
    cuda_right_hand_side<<<block_size, grid_size>>>(state.data(), right_hand_side.data(), this->size_x * this->size_y);
    cudaDeviceSynchronize();
}

void PendulumSystem::set_initial_conditions(const double time)
{
    for(int i = 0; i < this->size_x; i++){
        for(int j = 0; j < this->size_y; j++){
            set_der_phi_1(i, j, 0);
            set_der_phi_2(i, j, 0);
            set_phi_1(i, j, bounds[0] + (i + 1)*(bounds[1] - bounds[0])/(size_x + 1));
            set_phi_2(i, j, bounds[2] + (j + 1)*(bounds[3] - bounds[2])/(size_y + 1));
        }
    }
}

void PendulumSystem::write_state_to_file(double save_time, double time_step, std::string folder_name)
{
    std::filesystem::path results_path("results");
    std::filesystem::path dir_path = results_path / folder_name;
    std::filesystem::create_directories(dir_path);
    std::stringstream file_name;
    file_name << "State_" << std::setw(5) << std::setfill('0') << std::round(save_time/time_step) << ".txt";
    std::filesystem::path file_path = dir_path / file_name.str();
    
    std::fstream file;
    file.open( file_path, std::fstream::out | std::fstream::trunc );
    if(!file)
    {
        throw std::ios_base::failure("Unable to open the file: " + file_path.string());
    }

    file << std::scientific << std::setprecision(15);
    file << save_time << std::endl << std::endl;
    file << this->size_x << std::endl << this->size_y << std::endl << std::endl;
    for(int j = 0; j < size_y; j++)
    {
        for( int i = 0; i < size_x; i++ )
        {
            file << i << " "
                 << j << " "
                 << get_phi_1(i, j, save_time) << " "
                 << get_phi_2(i, j, save_time) << " "
                 << get_der_phi_1(i, j, save_time) << " "
                 << get_der_phi_2(i, j, save_time);
            file << std::endl;
        }
        file << std::endl;
    }
    file.close();
}

void PendulumSystem::record_state()
{
    this->state_history[this->time] = host_state;
}

__global__ void cuda_right_hand_side(const double* state,
                                     double* right_hand_side,
                                     int size,
                                     double mass_1,
                                     double mass_2,
                                     double length_1,
                                     double length_2)
{
    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = blockDim.x * gridDim.x;

    for ( int i = tid; i < size; i += stride)
    {
        float g = 9.81;
        double a, b, c, d, e, f;
    
        a = -(mass_1 + mass_2)*g*length_1*sin(state[i*4])
            - mass_2*length_1*length_2*pow(state[i*4 + 3], 2)*sin(state[i*4] - state[i*4 + 1]);
        b = (mass_1 + mass_2)*length_1*length_1;
        c = mass_2*length_1*length_2*cos(state[i*4] - state[i*4 + 1]);
        d = -mass_2*g*length_2*sin(state[i*4 + 1])
            + mass_2*length_1*length_2*pow(state[i*4 + 2], 2)*sin(state[i*4] - state[i*4 + 1]);
        e = mass_2*length_2*length_2;
        f = mass_2*length_1*length_2*cos(state[i*4] - state[i*4 + 1]);
    
        right_hand_side[i*4] = state[i*4 + 2];
        right_hand_side[i*4 + 1] = state[i*4 + 3];
        right_hand_side[i*4 + 2] = a/b - c/b*(d-a*f/b)/(e-c*f/b);
        right_hand_side[i*4 + 3] = (d-a*f/b)/(e-c*f/b);
    }
}