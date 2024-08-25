#include "./surface.hpp"

#include <vulkan/vulkan_core.h>

#include "GLFW/glfw3.h"
#include "glfw_controller.hpp"
#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/instance.hpp"

vki::Surface::Surface(const vki::VulkanInstance &vulkanInstance,
                      const GLFWControllerWindow &window)
    : instance{ vulkanInstance.getInstance() } {
    VkResult result = glfwCreateWindowSurface(instance, window.getGLFWWindow(),
                                              nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "glfwCreateWindowSurface");
    };
};



VkSurfaceKHR vki::Surface::getVkSurfaceKHR() const { return surface; };

vki::Surface::~Surface() { vkDestroySurfaceKHR(instance, surface, nullptr); };
