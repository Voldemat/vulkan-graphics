#include <vulkan/vulkan_core.h>
#include <string>

#include "./glfw_controller.hpp"
#include "./vulkan_app/vulkan_app.hpp"
#include "vulkan_app/vki/instance.hpp"

#define ELPP_STL_LOGGING
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main() {
    auto& mainLogger = *el::Loggers::getLogger("main");
    mainLogger.info("Creating GLFWController...");
    GLFWController controller;
    mainLogger.info("Created GLFWController");
    mainLogger.info("Obtaining GLFWControllerWindow...");
    GLFWControllerWindow window = controller.createWindow();
    mainLogger.info("Obtained GLFWControllerWindow...");
    const auto& requiredExtensions = controller.getRequiredExtensions();
    mainLogger.info("GLFW Required extensions: ");
    mainLogger.info(requiredExtensions);
    vki::VulkanInstanceParams params = {
        .extensions = requiredExtensions,
        .appName = "Hello triangle",
        .appVersion = { 1, 0, 0 },
        .apiVersion = VK_API_VERSION_1_3,
        .layers = { "VK_LAYER_KHRONOS_validation" },
    };
    VulkanApplication application(params, controller, window);
    return 0;
};
