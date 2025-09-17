#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <array>
#include <string>

#include "System.hpp"
#include "Pendulum_system.hpp"
#include "RungeKutta.hpp"

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    void run();

    private:
    std::array<double, 4> bounds_;
    int size_x_;
    int size_y_;
    float show_time_;
    float max_time_;
    double integration_step_;
    char txt_folder_name_[64];


    void init_glfw_glad(const char* title, int width, int height);
    void init_imgui();

    GLuint create_texture();
    void update_texture(double show_time);
    void calculate();
    void save_image(double show_time);
    void save_state_to_txt_file(double save_time);
    double normalize_angle(double angle) const;

    GLFWwindow* window_{nullptr};
    GLuint texture_{0};
    PendulumSystem* system_{nullptr};

};