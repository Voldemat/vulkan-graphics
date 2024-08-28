#include "./glfw_controller.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"

GLFWControllerWindow::GLFWControllerWindow(const std::string &name,
                                           const unsigned int &width,
                                           const unsigned int &height) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
};
bool GLFWControllerWindow::shouldClose() const {
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

GLFWControllerWindow GLFWController::createWindow(const std::string &name,
                                                  const unsigned int &width,
                                                  const unsigned int &height) {
    return GLFWControllerWindow(name, width, height);
};

void GLFWController::pollEvents() const { glfwPollEvents(); };

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

std::pair<unsigned int, unsigned int> GLFWControllerWindow::getFramebufferSize()
    const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width >= 0) {
        throw std::runtime_error("GLFW window framebuffer width must be gte 0");
    };
    if (height >= 0) {
        throw std::runtime_error("GLFW window framebuffer height must be gte 0");
    };
    return { width, height };
};

std::pair<unsigned int, unsigned int> GLFWControllerWindow::getWindowSize()
    const {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    if (width >= 0) {
        throw std::runtime_error("GLFW window width must be gte 0");
    };
    if (height >= 0) {
        throw std::runtime_error("GLFW window height must be gte 0");
    };
    return { width, height };
};
