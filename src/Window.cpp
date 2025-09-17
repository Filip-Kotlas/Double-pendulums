#include "Window.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <stdexcept>

constexpr double PI = 3.141592653589793;

Window::Window(int width, int height, const char* title)
    : bounds_{-PI, PI, -PI, PI},
      size_x_{256}, size_y_{256},
      show_time_{0.f}, max_time_{10.f}, integration_step_{0.01} {

    init_glfw_glad(title, width, height);
    init_imgui();

    // inicializace systému a textury
    system_ = new PendulumSystem(size_x_, size_y_, bounds_, 1.0, 1.0, 1.0, 1.0);
    texture_ = create_texture();
    txt_folder_name_[0] = '\0';
}

Window::~Window() {
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
    return angle - std::floor(angle / (2.0 * PI)) * 2.0 * PI;
}

void Window::update_texture(double show_time) {
    if (!texture_ || !system_) return;
    glBindTexture(GL_TEXTURE_2D, texture_);

    int dof = system_->get_degrees_of_freedom();
    std::vector<unsigned char> data(dof * 3);

    for (int i = 0; i < system_->get_size()[0]; i++) {
        for (int j = 0; j < system_->get_size()[1]; j++) {
            int index = 3 * ((system_->get_size()[1] - 1 - j) * system_->get_size()[0] + i);

            data[index] = 0;        // R
            data[index + 1] = 0;    // G
            data[index + 2] = 0;    // B

            double phi1 = normalize_angle(system_->get_phi_1(i, j, show_time));
            double phi2 = normalize_angle(system_->get_phi_2(i, j, show_time));

            if (phi1 <= PI && phi2 <= PI) {
                data[index] = 255;
                data[index + 1] = 255;
            }
            else if (phi1 <= PI && phi2 > PI) {
                data[index] = 255;
            }
            else if (phi1 > PI && phi2 <= PI){
                data[index + 2] = 255;
            }
            else{
                data[index + 1] = 255;
            }
        }
    }

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        system_->get_size()[0],
        system_->get_size()[1],
        GL_RGB,
        GL_UNSIGNED_BYTE,
        data.data()
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Window::calculate() {
    if (system_) {
        RungeKutta solver;
        int step_count = 100;
        double time_step = max_time_ / step_count;
        solver.set_up(system_, time_step, integration_step_);
        solver.solve(max_time_);
    }
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

void Window::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save image")) {
                    save_image(show_time_);
                }
                if (ImGui::MenuItem("Save calculations")) {
                    save_state_to_txt_file(show_time_);
                }
                if (ImGui::MenuItem("Run computation")) {
                    if (texture_) { glDeleteTextures(1, &texture_); texture_ = 0; }
                    delete system_;
                    system_ = new PendulumSystem(size_x_, size_y_, bounds_, 1.0, 1.0, 1.0, 1.0);
                    calculate();
                    texture_ = create_texture();
                    update_texture(show_time_);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Parameters")) {
                ImGui::InputDouble("Left bound",  &bounds_[0]);
                ImGui::InputDouble("Right bound", &bounds_[1]);
                ImGui::InputDouble("Lower bound", &bounds_[2]);
                ImGui::InputDouble("Upper bound", &bounds_[3]);
                ImGui::InputInt("Size in x direction", &size_x_);
                ImGui::InputInt("Size in y direction", &size_y_);
                ImGui::InputFloat("Maximum time", &max_time_);
                ImGui::InputDouble("Integration step", &integration_step_);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::SliderFloat("Time", &show_time_, 0, max_time_)) {
                    update_texture(show_time_);
                }
                ImGui::InputText("Input file name", txt_folder_name_, sizeof(txt_folder_name_));
                if (ImGui::MenuItem("Load from folder")) {
                    if (texture_) { glDeleteTextures(1, &texture_); texture_ = 0; }
                    delete system_;
                    system_ = new PendulumSystem(std::string(txt_folder_name_));
                    max_time_ = static_cast<float>(system_->get_time());
                    texture_ = create_texture();
                    update_texture(0.0);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

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

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window_);
    }
}