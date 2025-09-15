#include "Pendulum_system.hpp"

PendulumSystem::PendulumSystem(std::string file_name)
: mass_1(1),
mass_2(1),
length_1(1),
length_2(1)
{
    std::ifstream file("results\\" + file_name);
    if (!file)
    {
        std::cout << "File cannot be opened. File: " << file_name << std::endl;
    }

    std::string line;
    int count = 0;
    std::getline(file, line);
    std::istringstream(line) >> this->size_x;
    std::getline(file, line);
    std::istringstream(line) >> this->size_y;
    this->degrees_of_freedom = size_x * size_y * 4;
    this->time = 0;
    state.resize(this->degrees_of_freedom, 0);

    int i, j;
    double phi_1, phi_2, der_phi_1, der_phi_2;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        if (!(iss >> i >> j >> phi_1 >> phi_2 >> der_phi_1 >> der_phi_2)) {
            std::cout << "Error while loading line: " << line << std::endl;
        }

        if (count >= this->size_x * this->size_y) {
            std::cout << "The file contains more data than anticipated." << std::endl;
        }

        set_phi_1(i, j, phi_1);
        set_phi_2(i, j, phi_2);
        set_der_phi_1(i, j, der_phi_1);
        set_der_phi_2(i, j, der_phi_2);

        count++;
        }

    if (count < this->size_x * this->size_y) {
        std::cout << "The file constains less data than anticipated." << std::endl;
    }
    this->record_state();
}

void PendulumSystem::get_right_hand_side(const double time, const std::vector<double> &state, std::vector<double> &right_hand_side)
{
    float g = 9.81;
    double a, b, c, d, e, f;
    for(int j = 0; j < this->size_y; j++){
        for(int i = 0; i < this->size_x; i++){
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
    for(int j = 0; j < this->size_y; j++){
        for(int i = 0; i < this->size_x; i++){
            set_der_phi_1(i, j, 0);
            set_der_phi_2(i, j, 0);
            set_phi_1(i, j, bounds[0] + (i + 1)*(bounds[1] - bounds[0])/(size_x + 1));
            set_phi_2(i, j, bounds[2] + (j + 1)*(bounds[3] - bounds[2])/(size_y + 1));
        }
    }
}

void PendulumSystem::write_state_to_file(double save_time, std::string folder_name)
{
    std::stringstream file_path;
    file_path << "State_" << std::setw( 5 ) << std::setfill( '0' ) << save_time << ".txt";
    
    std::fstream file;
    file.open( file_path.str(), std::fstream::out | std::fstream::trunc );
    if(!file)
    {
        throw std::ios_base::failure("Unable to open the file: " + file_path.str());
    }

    file << std::scientific << std::setprecision(15);

    file << this->size_x << std::endl << this->size_y << std::endl << std::endl;
    for(int j = 0; j < size_y; j++) {
        for( int i = 0; i < size_x; i++ ) {
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
}

void PendulumSystem::record_state()
{
    this->state_history[this->time] = state;
}