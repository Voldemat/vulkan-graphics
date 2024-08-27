#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "glfw_controller.hpp"
namespace vki {
class VulkanInstance;
class PhysicalDevice;

struct SurfaceDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Surface {
    VkInstance instance;
    VkSurfaceKHR surface;

protected:
    bool is_owner;

public:
    Surface(const Surface &other);
    explicit Surface(const vki::VulkanInstance &instance,
                     const GLFWControllerWindow &window);

    VkSurfaceKHR getVkSurfaceKHR() const;
    vki::SurfaceDetails getDetails(
        const vki::PhysicalDevice &physicalDevice) const;
    VkSurfaceCapabilitiesKHR getCapabilities(
        const vki::PhysicalDevice &physicalDevice) const;
    std::vector<VkSurfaceFormatKHR> getFormats(
        const vki::PhysicalDevice &physicalDevice) const;
    std::vector<VkPresentModeKHR> getPresentModes(
        const vki::PhysicalDevice &physicalDevice) const;
    ~Surface();
};
};  // namespace vki
