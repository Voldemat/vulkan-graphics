#pragma once

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <functional>
#include <unordered_set>

#include "glfw_controller.hpp"

namespace vki {
class VulkanInstance;
class PhysicalDevice;

struct VkSurfaceFormatKHRHasher {
    std::size_t operator()(const VkSurfaceFormatKHR &format) const noexcept {
        const auto &h1 = std::hash<VkFormat>{}(format.format);
        const auto &h2 = std::hash<VkColorSpaceKHR>{}(format.colorSpace);
        return h1 ^ (h2 << 1);
    };
};

struct VkSurfaceFormatKHRComparator {
    bool operator()(const VkSurfaceFormatKHR &first,
                    const VkSurfaceFormatKHR &second) const noexcept {
        return first.format == second.format &&
               first.colorSpace == second.colorSpace;
    };
};

using SurfaceFormatSet =
    std::unordered_set<VkSurfaceFormatKHR, VkSurfaceFormatKHRHasher,
                       VkSurfaceFormatKHRComparator>;
enum class PresentMode {
    IMMEDIATE_KHR = VK_PRESENT_MODE_IMMEDIATE_KHR,
    MAILBOX_KHR = VK_PRESENT_MODE_MAILBOX_KHR,
    FIFO_KHR = VK_PRESENT_MODE_FIFO_KHR,
    FIFO_RELAXED_KHR = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    SHARED_DEMAND_REFRESH_KHR = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
    SHARED_CONTINUOUS_REFRESH_KHR = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR
};

struct SurfaceDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    SurfaceFormatSet formats;
    std::unordered_set<PresentMode> presentModes;
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
    vki::SurfaceFormatSet getFormats(
        const vki::PhysicalDevice &physicalDevice) const;
    std::unordered_set<vki::PresentMode> getPresentModes(
        const vki::PhysicalDevice &physicalDevice) const;
    ~Surface();
};
};  // namespace vki
