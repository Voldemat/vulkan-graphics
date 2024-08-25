#pragma once

#include <vulkan/vulkan_core.h>
#include "glfw_controller.hpp"
namespace vki {
class VulkanInstance;
class Surface {
    VkInstance instance;
    VkSurfaceKHR surface;
public:
    Surface(const Surface& other) = delete;
    explicit Surface(const vki::VulkanInstance &instance,
                     const GLFWControllerWindow &window);

    VkSurfaceKHR getVkSurfaceKHR() const;
    ~Surface();
};
};  // namespace vki
