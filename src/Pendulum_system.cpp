#include "Pendulum_system.hpp"

PendulumSystem::PendulumSystem(std::string folder_name)
: mass_1(1),
mass_2(1),
length_1(1),
length_2(1)
{
    std::string folder_path = "results\\" + folder_name;
    std::stringstream file_name;
    for (int i = 0; i <= 100; i++) {
        file_name.str("");
        file_name << "\\State_" << std::setw(5) << std::setfill('0') << i << ".txt";
        this->add_state_to_history_from_file(folder_path + file_name.str());
    }
}

PendulumSystem::PendulumSystem(const PendulumSystem& original, int start_y, int end_y)
    : size_x(original.size_x),
      size_y(end_y - start_y),
      mass_1(original.mass_1),
      mass_2(original.mass_2),
      length_1(original.length_1),
      length_2(original.length_2),
      bounds(original.bounds) 
{
    // Každý bod má 4 stupně volnosti
    this->degrees_of_freedom = size_x * size_y * 4;

    // Čas začíná stejně jako v originálu
    this->time = original.time;

    // Alokujeme state pro podmřížku
    state.resize(this->degrees_of_freedom, 0.0);

    // Překopírujeme data z originálu (jen řádky start_y .. end_y-1)
    for (int j = start_y; j < end_y; ++j) {
        for (int i = 0; i < size_x; ++i) {
            int local_j = j - start_y;

            set<Component::phi_1>(i, local_j, original.get<Component::phi_1>(i, j));
            set<Component::phi_2>(i, local_j, original.get<Component::phi_2>(i, j));
            set<Component::der_phi_1>(i, local_j, original.get<Component::der_phi_1>(i, j));
            set<Component::der_phi_2>(i, local_j, original.get<Component::der_phi_2>(i, j));
        }
    }
}


void PendulumSystem::add_state_to_history_from_file(std::string file_name)
{
    std::ifstream file(file_name);
    if (!file)
    {
        std::cout << "File cannot be opened. File: " << file_name << std::endl;
    }

    std::string line;
    int count = 0;
    std::getline(file, line);
    std::istringstream(line) >> this->time;
    std::getline(file, line);
    std::getline(file, line);
    std::istringstream(line) >> this->size_x;
    std::getline(file, line);
    std::istringstream(line) >> this->size_y;
    this->degrees_of_freedom = size_x * size_y * 4;
    state.resize(this->degrees_of_freedom, 0);

    int i, j;
    double phi_1, phi_2, der_phi_1, der_phi_2;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        if (!(iss >> i >> j >> phi_1 >> phi_2 >> der_phi_1 >> der_phi_2)) {
            throw std::runtime_error("Error while loading line: " + line + "\n");
        }

        if (count >= this->size_x * this->size_y) {
            throw std::runtime_error("The file contains more data than anticipated.\n");
        }

        set<Component::phi_1>(i, j, phi_1);
        set<Component::phi_2>(i, j, phi_2);
        set<Component::der_phi_1>(i, j, der_phi_1);
        set<Component::der_phi_2>(i, j, der_phi_2);

        count++;
        }

    if (count < this->size_x * this->size_y) {
        throw std::runtime_error("The file constains less data than anticipated.\n");
    }
    file.close();
    this->record_state();
}

void PendulumSystem::get_right_hand_side(const double time, const std::vector<double> &state, std::vector<double> &right_hand_side)
{
    float g = 9.81;
    double a, b, c, d, e, f;
    for(int i = 0; i < this->size_x; i++){
        for(int j = 0; j < this->size_y; j++){
            a = -(mass_1 + mass_2)*g*length_1*sin(get<Component::phi_1>(i, j))
                - mass_2*length_1*length_2*pow(get<Component::der_phi_2>(i, j), 2)*sin(get<Component::phi_1>(i, j) - get<Component::phi_2>(i, j));
            b = (mass_1 + mass_2)*length_1*length_1;
            c = mass_2*length_1*length_2*cos(get<Component::phi_1>(i, j) - get<Component::phi_2>(i, j));
            d = -mass_2*g*length_2*sin(get<Component::phi_2>(i, j))
                + mass_2*length_1*length_2*pow(get<Component::der_phi_1>(i, j), 2)*sin(get<Component::phi_1>(i, j) - get<Component::phi_2>(i, j));
            e = mass_2*length_2*length_2;
            f = mass_2*length_1*length_2*cos(get<Component::phi_1>(i, j) - get<Component::phi_2>(i, j));

            right_hand_side[(j*size_x + i)*4] = get<Component::der_phi_1>(i, j);
            right_hand_side[(j*size_x + i)*4 + 1] = get<Component::der_phi_2>(i, j);
            right_hand_side[(j*size_x + i)*4 + 2] = a/b - c/b*(d-a*f/b)/(e-c*f/b);
            right_hand_side[(j*size_x + i)*4 + 3] = (d-a*f/b)/(e-c*f/b);
        }
    }
}

void PendulumSystem::set_initial_conditions(const double time)
{
    for(int i = 0; i < this->size_x; i++){
        for(int j = 0; j < this->size_y; j++){
            set<Component::der_phi_1>(i, j, 0);
            set<Component::der_phi_2>(i, j, 0);
            set<Component::phi_1>(i, j, bounds[0] + (i + 1)*(bounds[1] - bounds[0])/(size_x + 1));
            set<Component::phi_2>(i, j, bounds[2] + (j + 1)*(bounds[3] - bounds[2])/(size_y + 1));
        }
    }
}

void PendulumSystem::write_state_to_file(double save_time, std::string folder_name)
{
    std::stringstream file_path;
    file_path <<  folder_name << "\\State_" << std::setw( 5 ) << std::setfill( '0' ) << save_time << ".txt";
    
    std::fstream file;
    file.open( file_path.str(), std::fstream::out | std::fstream::trunc );
    if(!file)
    {
        throw std::ios_base::failure("Unable to open the file: " + file_path.str());
    }

    file << std::scientific << std::setprecision(15);

    file << this->size_x << std::endl << this->size_y << std::endl << std::endl;
    for(int j = 0; j < size_y; j++)
    {
        for( int i = 0; i < size_x; i++ )
        {
            file << i << " "
                 << j << " "
                 << get<Component::phi_1>(i, j, save_time) << " "
                 << get<Component::phi_2>(i, j, save_time) << " "
                 << get<Component::der_phi_1>(i, j, save_time) << " "
                 << get<Component::der_phi_2>(i, j, save_time);
            file << std::endl;
        }
        file << std::endl;
    }
}

void PendulumSystem::record_state()
{
    this->state_history[this->time] = state;
}

void PendulumSystem::merge(const PendulumSystem& part, int start_y) {
    // projdeme všechny uložené stavy v part.state_history
    for (const auto& [time_key, state_vec] : part.state_history) {
        // pokud ještě nemáme záznam pro tenhle čas, vytvoříme nový
        auto& target_state = this->state_history[time_key];

        // pokud je to poprvé, musíme mít správnou velikost
        if (target_state.empty()) {
            target_state.resize(this->degrees_of_freedom, 0.0);
        }

        // zkopírujeme blok hodnot z part.state_history do target_state
        for (int j = 0; j < part.size_y; ++j) {
            for (int i = 0; i < part.size_x; ++i) {
                int global_j = start_y + j;

                target_state[(global_j * size_x + i) * 4 + 0] = state_vec[(j * part.size_x + i) * 4 + 0];
                target_state[(global_j * size_x + i) * 4 + 1] = state_vec[(j * part.size_x + i) * 4 + 1];
                target_state[(global_j * size_x + i) * 4 + 2] = state_vec[(j * part.size_x + i) * 4 + 2];
                target_state[(global_j * size_x + i) * 4 + 3] = state_vec[(j * part.size_x + i) * 4 + 3];
            }
        }
    }

    // přeneseme také čas (systémy běží synchronně)
    this->time = part.time;
}

