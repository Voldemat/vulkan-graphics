#include <vulkan/vulkan_core.h>

#include "./glfw_controller.hpp"
#include "./vulkan_app/vulkan_app.hpp"

int main() {
    GLFWController controller;
    GLFWControllerWindow window = controller.createWindow();
    VulkanApplication application(controller.getExtensions(), window);
    // while (!window.shouldClose()) {
    //     controller.pollEvents();
    // };
    return 0;
};
