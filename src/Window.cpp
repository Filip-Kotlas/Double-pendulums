#include "Window.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <numeric>
#include "Pendulum_system.hpp"

//constexpr double PI = 3.141592653589793;

Window::Window(int width, int height, const char* title)
    : bounds_{-PI<6>, PI<6>, -PI<6>, PI<6>},
      size_x_{256},
      size_y_{256},
      show_time_{0.f},
      max_time_{10.f},
      integration_step_{0.01},
      steps_per_second_{10}{

    init_glfw_glad(title, width, height);
    init_imgui();

    // inicializace systému a textury
    system_ = new PendulumSystem(size_x_, size_y_, bounds_, 1.0, 1.0, 1.0, 1.0);
    texture_ = create_texture();
    txt_folder_name_[0] = '\0';
    num_threads_ = std::max(1, std::min(static_cast<int>(std::thread::hardware_concurrency()), this->size_y_));
}

Window::~Window() {
    cancel_ = true;

    if (worker_.valid()) {
        worker_.get();
    }    

    if (texture_)
        glDeleteTextures(1, &texture_);
    delete system_;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

void Window::init_glfw_glad(const char* title, int width, int height) {
    if (!glfwInit())
        throw std::runtime_error("Failed to init GLFW");

    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        throw std::runtime_error("Failed to init GLAD");
    }
}

void Window::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

GLuint Window::create_texture() {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    int dof = system_->get_degrees_of_freedom();
    std::vector<unsigned char> data(dof * 3);
    for (size_t i = 0; i < data.size(); i += 3) {
        data[i] = 255;        // R
        data[i + 1] = 255;    // G
        data[i + 2] = 255;    // B
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, system_->get_size()[0], system_->get_size()[1], 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

double Window::normalize_angle(double angle) const
{
    // normalizuje úhel do rozsahu [0, 2*PI)
    return angle - std::floor(angle / (2.0 * PI<6>)) * 2.0 * PI<6>;
}

void Window::update_texture(double show_time) {
    if (!texture_ || !system_) return;
    glBindTexture(GL_TEXTURE_2D, texture_);

    int width  = system_->get_size()[0];
    int height = system_->get_size()[1];
    std::vector<unsigned char> data(width * height * 3);

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int index = 3 * ((height - 1 - j) * width + i);
            auto color = determine_pixel_color(i, j, show_time);
            data[index]     = color[0];
            data[index + 1] = color[1];
            data[index + 2] = color[2];
        }
    }

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        width,
        height,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        data.data()
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}


void Window::save_image(double show_time) {
    if (!texture_ || !system_) return;

    std::vector<unsigned char> pixels(system_->get_size()[0] * system_->get_size()[1] * 3);

    glBindTexture(GL_TEXTURE_2D, texture_);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    std::string filename = std::string("Images/output") + std::to_string(show_time) + ".png";
    stbi_write_png(filename.c_str(),
                   system_->get_size()[0],
                   system_->get_size()[1],
                   3,
                   pixels.data(),
                   system_->get_size()[0] * 3);
}

void Window::save_state_to_txt_file(double save_time)
{
    if (!system_) return;
    system_->write_state_to_file(save_time, "results");
}

void Window::process_events() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::render_main_menu() {
    if (ImGui::BeginMainMenuBar()) {
        this->render_file_menu();
        this->render_parameters_menu();
        this->render_view_menu();
        ImGui::EndMainMenuBar();
    }
}

void Window::render_file_menu() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save image")) {
            save_image(show_time_);
        }
        if (ImGui::MenuItem("Save calculations")) {
            save_state_to_txt_file(show_time_);
        }
        if (!computing_) {
            if (ImGui::MenuItem("Run computation")) {
                if (texture_) {
                    glDeleteTextures(1, &texture_); texture_ = 0;
                }
                delete system_;
                system_ = new PendulumSystem(size_x_, size_y_, bounds_, 1.0, 1.0, 1.0, 1.0);

                in_memory_ = false;
    
                worker_ = std::async(std::launch::async, &Window::compute_task, this);
            }
        } else {
            if (ImGui::MenuItem("Cancel computation")) {
                cancel_ = true;
            }
            float progress = std::accumulate(thread_progress_.begin(), thread_progress_.end(), 0.0f)
                             / static_cast<float>(thread_progress_.size());
            ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        }
        ImGui::EndMenu();
    }
}


void Window::render_parameters_menu() {
    if (ImGui::BeginMenu("Parameters")) {
        ImGui::InputDouble("Left bound",  &bounds_[0]);
        ImGui::InputDouble("Right bound", &bounds_[1]);
        ImGui::InputDouble("Lower bound", &bounds_[2]);
        ImGui::InputDouble("Upper bound", &bounds_[3]);
        ImGui::InputInt("Size in x direction", &size_x_);
        ImGui::InputInt("Size in y direction", &size_y_);
        ImGui::InputFloat("Maximum time", &max_time_);
        ImGui::InputDouble("Integration step", &integration_step_);
        ImGui::SliderInt("Frames per second", &steps_per_second_, 1, 60);
        ImGui::SliderInt("Threads",
                         &num_threads_,
                         1,
                         std::min(2*static_cast<int>(std::thread::hardware_concurrency()), this->size_y_));
        ImGui::EndMenu();
    }
}

void Window::render_view_menu() {
    if (ImGui::BeginMenu("View")) {
        if (in_memory_) {
            if (ImGui::MenuItem("Render results")) {
                if (texture_) { glDeleteTextures(1, &texture_); texture_ = 0; }
                texture_ = create_texture();
                update_texture(show_time_);
            }
            if (ImGui::SliderFloat("Time", &show_time_, 0, max_time_)) {
                update_texture(show_time_);
            }
        }
        ImGui::InputText("Input file name", txt_folder_name_, sizeof(txt_folder_name_));
        if (ImGui::MenuItem("Load from folder")) {
            cancel_ = true;
            in_memory_ = false;
            if (worker_.valid())
                worker_.get();
                
            if (texture_) {
                glDeleteTextures(1, &texture_); texture_ = 0;
            }
            delete system_;

            system_ = new PendulumSystem(std::string(txt_folder_name_));
            if (system) {
                max_time_ = static_cast<float>(system_->get_time());
                texture_ = create_texture();
                update_texture(0.0);
                in_memory_ = true;
            }
        }
        ImGui::EndMenu();
    }
}

void Window::compute_task() {
    computing_ = true;
    cancel_    = false;

    if (!system_) {
        computing_ = false;
        return;
    }

    thread_progress_.clear();
    thread_progress_.resize(num_threads_);
    for (auto& p : thread_progress_)
        p = 0.0f;


    // Rozdělení řádků po vláknech rovnoměrně (s případným zbytkem)
    const int base_rows = size_y_ / num_threads_;
    const int rem_rows  = size_y_ % num_threads_;

    // Každé vlákno dostane vlastní kopii podmřížky
    std::vector<PendulumSystem> subsystems;
    subsystems.reserve(num_threads_);

    std::vector<std::future<void>> futures;
    futures.reserve(num_threads_);

    // Vytvoření sub-systémů a spuštění výpočtů (bez lambd)
    int y = 0;
    for (int t = 0; t < num_threads_; ++t) {
        const int rows    = base_rows + (t < rem_rows ? 1 : 0);
        const int start_y = y;
        const int end_y   = y + rows;
        y = end_y;

        // Konstruktor subgridu: PendulumSystem(const PendulumSystem& original, int start_y, int end_y)
        subsystems.emplace_back(*system_, start_y, end_y);

        // Spustíme výpočet bloku: compute_block(PendulumSystem& sub)
        futures.emplace_back(std::async(std::launch::async,
                                        &Window::compute_block, this,
                                        std::ref(subsystems.back()),
                                        std::ref(thread_progress_[t])));
    }

    // Čekání na dokončení vláken + hrubý progress
    int done = 0;
    for (auto& f : futures) {
        f.get();
    }

    // Sloučení výsledků zpět do hlavního systému
    y = 0;
    for (int t = 0; t < num_threads_; ++t) {
        const int rows    = base_rows + (t < rem_rows ? 1 : 0);
        const int start_y = y;
        const int end_y   = y + rows;
        y = end_y;

        // Merge: system_->merge(const PendulumSystem& part, int start_y)
        system_->merge(subsystems[t], start_y);
    }

    in_memory_ = true;   // GUI teď může přes menu „Generate texture“ vytvořit texturu
    computing_ = false;
}

void Window::compute_block(PendulumSystem& sub, float& prog) {

    double time_step = 1.0 / steps_per_second_;

    Solver<RungeKutta_4<double>,double> solver(&sub, time_step, integration_step_, &prog, &cancel_);

    solver.solve(max_time_);
}

void Window::render_image_window() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowSize(ImVec2(viewportSize.x, viewportSize.y - ImGui::GetFrameHeight()));
    ImGui::Begin("Obrazek", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float windowAspect = avail.x / avail.y;
    float imageAspect  = static_cast<float>(system_->get_size()[0]) / static_cast<float>(system_->get_size()[1]);

    ImVec2 imageSize;
    if (windowAspect > imageAspect) {
        imageSize.y = avail.y;
        imageSize.x = imageAspect * avail.y;
    } else {
        imageSize.x = avail.x;
        imageSize.y = avail.x / imageAspect;
    }

    ImVec2 cursorPos = ImGui::GetCursorPos();
    ImGui::SetCursorPosX(cursorPos.x + (avail.x - imageSize.x) * 0.5f);
    ImGui::SetCursorPosY(cursorPos.y + (avail.y - imageSize.y) * 0.5f);

    ImGui::Image((void*)(intptr_t)texture_, imageSize);
    ImGui::End();
}

void Window::render_frame() {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);
}

void Window::run() {
    while (!glfwWindowShouldClose(window_)) {
        process_events();
        render_main_menu();
        render_image_window();
        render_frame();
    }
}

std::array<unsigned char, 3> Window::determine_pixel_color(int i, int j, double show_time) const {
    double phi_1 = normalize_angle(system_->get<PendulumSystem::Component::phi_1>(i, j, show_time));
    double phi_2 = normalize_angle(system_->get<PendulumSystem::Component::phi_2>(i, j, show_time));

    // Výchozí černá
    std::array<unsigned char, 3> color = {0, 0, 0};

    if (phi_1 <= PI<6> && phi_2 <= PI<6>) {
        color = {255, 255, 0}; // žlutá (R+G)
    }
    else if (phi_1 <= PI<6> && phi_2 > PI<6>) {
        color = {255, 0, 0};   // červená
    }
    else if (phi_1 > PI<6> && phi_2 <= PI<6>) {
        color = {0, 0, 255};   // modrá
    }
    else {
        color = {0, 255, 0};   // zelená
    }
    return color;
}
