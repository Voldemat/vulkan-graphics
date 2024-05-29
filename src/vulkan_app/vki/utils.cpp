#include "./utils.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <stdexcept>

uint32_t vki::utils::findMemoryType(
    uint32_t typeFilter, VkPhysicalDeviceMemoryProperties memProperties,
    VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
            memProperties.memoryTypes[i].propertyFlags & properties) {
            return i;
        };
    };
    throw std::runtime_error("failed to find suitable memory type!");
};
