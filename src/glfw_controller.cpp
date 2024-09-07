#include "./glfw_controller.hpp"

#include <cassert>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"

std::unordered_map<GLFWwindow *, std::function<void(int, int, int, int)>>
    keyCallbacks;

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
    if (!keyCallbacks.contains(window)) return;
    const auto &callback = keyCallbacks.at(window);
    callback(key, scancode, action, mods);
};

GLFWControllerWindow::GLFWControllerWindow(const std::string &name,
                                           const unsigned int &width,
                                           const unsigned int &height,
                                           bool enableKeyCallbacks) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (enableKeyCallbacks) {
        glfwSetKeyCallback(window, key_callback);
    };
};

void GLFWControllerWindow::registerKeyCallback(
    const std::function<void(int, int, int, int)> &callback) const {
    keyCallbacks[window] = callback;
};

void GLFWControllerWindow::unregisterKeyCallback() const {
    keyCallbacks.erase(window);
};

bool GLFWControllerWindow::shouldClose() const {
    return glfwWindowShouldClose(window);
};

GLFWwindow *GLFWControllerWindow::getGLFWWindow() const noexcept {
    return window;
};

GLFWControllerWindow::~GLFWControllerWindow() {
    unregisterKeyCallback();
    glfwDestroyWindow(window);
};

GLFWController::GLFWController() {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("GLFW failed to initialize");
    };
};

GLFWControllerWindow GLFWController::createWindow(const std::string &name,
                                                  const unsigned int &width,
                                                  const unsigned int &height,
                                                  bool enableKeyCallbacks) {
    return GLFWControllerWindow(name, width, height, enableKeyCallbacks);
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
        throw std::runtime_error(
            "GLFW window framebuffer height must be gte 0");
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
