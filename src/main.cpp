#include <vulkan/vulkan_core.h>

#include "./glfw_controller.hpp"
#include "./vulkan_app/vulkan_app.hpp"
#include "vulkan_app/vki/instance.hpp"

int main() {
    GLFWController controller;
    GLFWControllerWindow window = controller.createWindow();
    vki::VulkanInstanceParams params = {
        .extensions = controller.getRequiredExtensions(),
        .appName = "Hello triangle",
        .appVersion = { 1, 0, 0 },
        .apiVersion = VK_API_VERSION_1_3,
        .layers = { "VK_LAYER_KHRONOS_validation" },
    };
    VulkanApplication application(params, controller, window);
    return 0;
};
