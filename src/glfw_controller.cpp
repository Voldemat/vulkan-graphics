#include "./glfw_controller.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"

GLFWControllerWindow::GLFWControllerWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
};
bool GLFWControllerWindow::shouldClose() {
    return glfwWindowShouldClose(window);
};

GLFWwindow *GLFWControllerWindow::getGLFWWindow() const noexcept {
    return window;
};

GLFWControllerWindow::~GLFWControllerWindow() { glfwDestroyWindow(window); };

GLFWController::GLFWController() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("GLFW failed to initialize");
    };
};

GLFWControllerWindow GLFWController::createWindow() {
    return GLFWControllerWindow();
};

void GLFWController::pollEvents() { glfwPollEvents(); };

std::vector<std::string> GLFWController::getRequiredExtensions() {
    uint32_t extensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&extensionCount);
    std::vector<std::string> extensions;
    for (uint32_t i = 0; i < extensionCount; i++) {
        extensions.emplace_back(glfwExtensions[i]);
    };
    return extensions;
};

GLFWController::~GLFWController() { glfwTerminate(); };

std::pair<unsigned int, unsigned int> GLFWControllerWindow::getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width >= 0) {
        throw std::runtime_error("GLFW window framebuffer width must be gte 0");
    };
    if (height >= 0) {
        throw std::runtime_error("GLFW window framebuffer width must be gte 0");
    };
    return { width, height };
};

