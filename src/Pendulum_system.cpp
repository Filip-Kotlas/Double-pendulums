#include "Pendulum_system.hpp"

void PendulumSystem::get_right_hand_side(const double time, const std::vector<double>& state, std::vector<double>& right_hand_side)
{
    float g = 9.81;
    double a, b, c, d, e, f;
    for(int i = 0; i < this->size_x; i++){
        for(int j = 0; j < this->size_y; j++){
            a = -(mass_1 + mass_2)*g*length_1*sin(get_phi_1(i, j))
                - mass_2*length_1*length_2*pow(get_der_phi_2(i, j), 2)*sin(get_phi_1(i, j) - get_phi_2(i, j));
            b = (mass_1 + mass_2)*length_1*length_1;
            c = mass_2*length_1*length_2*cos(get_phi_1(i, j) - get_phi_2(i, j));
            d = -mass_2*g*length_2*sin(get_phi_2(i, j))
                + mass_2*length_1*length_2*pow(get_der_phi_1(i, j), 2)*sin(get_phi_1(i, j) - get_phi_2(i, j));
            e = mass_2*length_2*length_2;
            f = mass_2*length_1*length_2*cos(get_phi_1(i, j) - get_phi_2(i, j));

            right_hand_side[(j*size_x + i)*4] = get_der_phi_1(i, j);
            right_hand_side[(j*size_x + i)*4 + 1] = get_der_phi_2(i, j);
            right_hand_side[(j*size_x + i)*4 + 2] = a/b - c/b*(d-a*f/b)/(e-c*f/b);
            right_hand_side[(j*size_x + i)*4 + 3] = (d-a*f/b)/(e-c*f/b);
        }
    }
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

void PendulumSystem::write_state_to_file(int number, std::filesystem::path folder_name)
{
    std::stringstream file_name;
    file_name <<  "State_" << std::setw( 5 ) << std::setfill( '0' ) << number << ".txt";
    std::filesystem::path full_path = folder_name / file_name.str();
    
    std::fstream file;
    file.open( full_path.string(), std::fstream::out | std::fstream::trunc );
    if(!file)
    {
        throw std::ios_base::failure("Unable to open the file: " + file_name.str());
    }

    file << std::scientific << std::setprecision(15);

    file << this->time << std::endl << std::endl;
    for(int j = 0; j < size_y; j++)
    {
        for( int i = 0; i < size_x; i++ )
        {
            file << i << " "
                << j << " "
                << get_phi_1(i, j) << " "
                << get_phi_2(i, j) << " "
                << get_der_phi_1(i, j) << " "
                << get_der_phi_2(i, j);
            file << std::endl;
        }
        file << std::endl;
    }
}

void PendulumSystem::record_state()
{
    this->state_history[this->time] = state;
}

void PendulumSystem::save_history_to_folder(std::string folder_name)
{
    std::filesystem::path folder_path = "results" / std::filesystem::path(folder_name);

    if (std::filesystem::exists(folder_path)) 
        std::filesystem::remove_all(folder_path);

    std::filesystem::create_directories(folder_path);
   
    for (int i = 0; i < this->state_history.size(); i++)
    {
        write_state_to_file(i, folder_path);
    }
}