#pragma once

#include <vulkan/vulkan_core.h>

#include <functional>
#include <vector>

#include "main_utils.hpp"
#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/surface.hpp"

namespace vki {
class PhysicalDevice {
    VkPhysicalDevice device;
    void saveQueueFamilies(const vki::Surface &surface);
    std::vector<VkQueueFamilyProperties> getQueueFamiliesProperties() const;
    std::vector<QueueFamily> queueFamilies;

public:
    std::vector<QueueFamily> getQueueFamilies() const;
    std::vector<VkExtensionProperties> getExtensions() const;
    const VkPhysicalDeviceProperties properties;
    explicit PhysicalDevice(const VkPhysicalDevice &dev,
                            const vki::Surface &surface);
    VkPhysicalDeviceProperties getProperties() const;
    VkPhysicalDevice getVkDevice() const;
    bool hasQueueFamilies(
        const std::vector<std::function<bool(const QueueFamily &)>> &funcs)
        const;
    bool hasExtensions(
        const std::vector<std::function<bool(const VkExtensionProperties &)>>
            &funcs) const;

    VkPhysicalDeviceMemoryProperties getMemoryProperties() const;

    PRINTABLE_DEFINITIONS(PhysicalDevice)
};
};  // namespace vki
