#include "./physical_device.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "./base.hpp"

vki::PhysicalDevice::PhysicalDevice(const VkPhysicalDevice &dev,
                                    const VkSurfaceKHR &surface)
    : device{ dev } {
    saveQueueFamilyIndexes(surface);
};

void vki::PhysicalDevice::saveQueueFamilyIndexes(
    const VkSurfaceKHR &surface) {
    unsigned int i = 0;
    for (const auto &queueFamily : getQueueFamiliesProperties()) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyTypeToIndex[vki::QueueFamilyType::GRAPHIC] = i;
        };
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            device, i, surface, &presentSupport);
        if (result != VK_SUCCESS) {
            throw VulkanError(result, "vkGetPhysicalDeviceSurfaceSupportKHR");
        };
        if (presentSupport) {
            queueFamilyTypeToIndex[vki::QueueFamilyType::PRESENT] = i;
        };
        i++;
    }
};

VkPhysicalDeviceProperties vki::PhysicalDevice::getProperties() const {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    return properties;
};

VkPhysicalDevice vki::PhysicalDevice::getVkDevice() const { return device; };

std::vector<VkQueueFamilyProperties>
vki::PhysicalDevice::getQueueFamiliesProperties() const {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());
    return queueFamilies;
};

bool vki::PhysicalDevice::isSuitable() const {
    return queueFamilyTypeToIndex.contains(vki::QueueFamilyType::GRAPHIC) &&
           queueFamilyTypeToIndex.contains(vki::QueueFamilyType::PRESENT);
};

unsigned int vki::PhysicalDevice::getFamilyTypeIndex(vki::QueueFamilyType type) const {
    return queueFamilyTypeToIndex.at(type);
};

