#ifndef GLFW_CONTROLLER
#define GLFW_CONTROLLER
#include <string>
#include <utility>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class GLFWControllerWindow {
    GLFWwindow *window;

public:
    GLFWControllerWindow();
    bool shouldClose();
    GLFWwindow* getGLFWWindow() const noexcept;
    std::pair<unsigned int, unsigned int> getFramebufferSize() const;
    GLFWControllerWindow(const GLFWControllerWindow &other) = delete;
    ~GLFWControllerWindow();
};

class GLFWController {
public:
    GLFWController();
    GLFWController(const GLFWController &other) = delete;
    GLFWControllerWindow createWindow();
    void pollEvents();
    std::vector<std::string> getRequiredExtensions();

    ~GLFWController();
};
#endif
