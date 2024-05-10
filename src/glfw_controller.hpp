#ifndef GLFW_CONTROLLER
#define GLFW_CONTROLLER
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class GLFWControllerWindow {
    GLFWwindow *window;

public:
    GLFWControllerWindow();
    bool shouldClose();
    GLFWControllerWindow(const GLFWControllerWindow &other) = delete;
    ~GLFWControllerWindow();
};

class GLFWController {
public:
    GLFWController();
    GLFWController(const GLFWController &other) = delete;
    GLFWControllerWindow createWindow();
    void pollEvents();
    std::vector<const char *> getExtensions();
    ~GLFWController();
};
#endif
