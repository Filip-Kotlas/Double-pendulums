#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <future>

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
    int steps_per_second_;
    char txt_folder_name_[64];

    std::future<void> worker_;
    bool computing_ = false;
    std::atomic<bool> cancel_{false};
    std::atomic<float> progress_{0.0f};
    std::atomic<bool> finished_{false};

    void init_glfw_glad(const char* title, int width, int height);
    void init_imgui();

    void process_events();
    void render_main_menu();
    void render_image_window();
    void render_frame();

    GLuint create_texture();
    void update_texture(double show_time);
    std::array<unsigned char, 3> determine_pixel_color(int i, int j, double show_time) const;
    void calculate();
    void compute_task();

    void save_image(double show_time);
    void save_state_to_txt_file(double save_time);

    double normalize_angle(double angle) const;

    GLFWwindow* window_{nullptr};
    GLuint texture_{0};
    PendulumSystem* system_{nullptr};

};