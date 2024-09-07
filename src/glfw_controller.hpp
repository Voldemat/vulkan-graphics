#pragma once
#include <functional>
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
                         const unsigned int &height, bool enableKeyCallbacks);

public:
    bool shouldClose() const;
    GLFWwindow *getGLFWWindow() const noexcept;
    std::pair<unsigned int, unsigned int> getFramebufferSize() const;
    std::pair<unsigned int, unsigned int> getWindowSize() const;
    void setKeyCallback(const std::function<void(GLFWwindow *, int, int, int,
                                                 int)> &callback) const;
    GLFWControllerWindow(const GLFWControllerWindow &other) = delete;
    void registerKeyCallback(
        const std::function<void(int, int, int, int)> &callback) const;
    void unregisterKeyCallback() const;
    ~GLFWControllerWindow();
};

class GLFWController {
public:
    GLFWController();
    GLFWController(const GLFWController &other) = delete;
    GLFWControllerWindow createWindow(const std::string &name,
                                      const unsigned int &width,
                                      const unsigned int &height,
                                      bool enableKeyCallbacks);
    void pollEvents() const;
    std::vector<std::string> getRequiredExtensions();

    ~GLFWController();
};
