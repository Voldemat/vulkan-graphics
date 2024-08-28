#pragma once
#include <string>
#include <utility>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class GLFWController;

class GLFWControllerWindow {
    GLFWwindow *window;
    friend class GLFWController;
    GLFWControllerWindow(const std::string &name, const unsigned int &width,
                         const unsigned int &height);

public:
    bool shouldClose() const;
    GLFWwindow *getGLFWWindow() const noexcept;
    std::pair<unsigned int, unsigned int> getFramebufferSize() const;
    std::pair<unsigned int, unsigned int> getWindowSize() const;
    GLFWControllerWindow(const GLFWControllerWindow &other) = delete;
    ~GLFWControllerWindow();
};

class GLFWController {
public:
    GLFWController();
    GLFWController(const GLFWController &other) = delete;
    GLFWControllerWindow createWindow(const std::string &name,
                                      const unsigned int &width,
                                      const unsigned int &height);
    void pollEvents() const;
    std::vector<std::string> getRequiredExtensions();

    ~GLFWController();
};
