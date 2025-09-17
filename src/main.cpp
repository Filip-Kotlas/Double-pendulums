#include "Window.hpp"
#include <iostream>

int main() {
    try {
        Window w(800, 600, "Menu + Image");
        w.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
