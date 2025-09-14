#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <string>

#include "System.hpp"
#include "Pendulum_system.hpp"
#include "Runge-Kutta.hpp"

constexpr double PI = 3.141592653589793;

GLuint create_texture(PendulumSystem* system) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    int dof = system->get_degrees_of_freedom();
    unsigned char* data = new unsigned char[dof * 3];
    for(int i = 0; i < system->get_degrees_of_freedom() * 3; i += 3){
        data[i] = 255;      // R
        data[i + 1] = 255;    // G
        data[i + 2] = 255;    // B
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, system->get_size()[0], system->get_size()[1], 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;
    return tex;
}

double normalize_angle(double angle)
{
    double normal_angle = angle - floor(angle/(2*PI))*2*PI;
    return angle - floor(angle/(2*PI))*2*PI;
}

void update_texture(GLuint texture, PendulumSystem* system, double show_time) {
    glBindTexture(GL_TEXTURE_2D, texture);

    int dof = system->get_degrees_of_freedom();
    unsigned char* data = new unsigned char[dof * 3];

    int index = 0;
    for (int i = 0; i < system->get_size()[0]; i++) {
        for (int j = 0; j < system->get_size()[1]; j++) {
            index = 3*((system->get_size()[1] - 1 - j)*system->get_size()[0] + i);

            data[index] = 0;        // R
            data[index + 1] = 0;    // G
            data[index + 2] = 0;    // B
            
            if (normalize_angle(system->get_phi_1(i, j, show_time)) <= PI &&
                normalize_angle(system->get_phi_2(i, j, show_time)) <= PI) {
                data[index] = 255;
                data[index + 1] = 255;
            }
            else if (normalize_angle(system->get_phi_1(i, j, show_time)) <= PI &&
                     normalize_angle(system->get_phi_2(i, j, show_time)) > PI) {
                data[index] = 255;
            }
            else if (normalize_angle(system->get_phi_1(i, j, show_time)) > PI &&
                     normalize_angle(system->get_phi_2(i, j, show_time)) <= PI){
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
        system->get_size()[0], 
        system->get_size()[1], 
        GL_RGB, 
        GL_UNSIGNED_BYTE, 
        data
    );

    delete[] data;
    glBindTexture(GL_TEXTURE_2D, 0);
}

void calculate(PendulumSystem* system, double max_time, double integration_step) {
    RungeKutta solver;
    int step_count = max_time * 10;
    double time_step = 0.1;
    solver.set_up(system, time_step, integration_step);
    solver.solve(max_time);
}

void save_image(GLuint texture, PendulumSystem* system, double show_time) {
    std::vector<unsigned char> pixels(system->get_size()[0] * system->get_size()[1] * 3);

    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    std::string filename = "Images/output" + std::to_string(show_time) + ".png";
    stbi_write_png(filename.c_str(),
                   system->get_size()[0],
                   system->get_size()[1],
                   3,
                   pixels.data(),
                   system->get_size()[0] * 3);
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Menu + Image", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::array<double, 4> bounds = {-PI, PI, -PI, PI};
    int size_x = 256;
    int size_y = 256;
    float show_time = 0;
    float max_time = 1;
    double integration_step = 0.01;
    PendulumSystem system(size_x, size_y, bounds, 1.0, 1.0, 1.0, 1.0);
    GLuint texture = create_texture(&system);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save Image")) {
                    save_image(texture, &system, show_time);
                }
                if (ImGui::MenuItem("Reset Image")) {
                    glDeleteTextures(1, &texture);
                    system = PendulumSystem(size_x, size_y, bounds, 1.0, 1.0, 1.0, 1.0);
                    calculate(&system, max_time, integration_step);
                    texture = create_texture(&system);
                    update_texture(texture, &system, show_time);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Parameters")) {
                ImGui::InputDouble("Left bound",  &bounds[0]);
                ImGui::InputDouble("Right bound", &bounds[1]);
                ImGui::InputDouble("Lower bound", &bounds[2]);
                ImGui::InputDouble("Upper bound", &bounds[3]);
                ImGui::InputInt("Size in x direction", &size_x);
                ImGui::InputInt("Size in y direction", &size_y);
                ImGui::InputFloat("Maximum time", &max_time);
                ImGui::InputDouble("Integration step", &integration_step);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::SliderFloat("Time", &show_time, 0, max_time)) {
                    update_texture(texture, &system, show_time);
                }
                if (ImGui::MenuItem("Animation")) {
                    
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
        float imageAspect  = (float)system.get_size()[0] / (float)system.get_size()[1];

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

        ImGui::Image((void*)(intptr_t)texture, imageSize);
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glDeleteTextures(1, &texture);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
