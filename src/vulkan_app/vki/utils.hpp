#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>

namespace vki {
namespace utils {
uint32_t findMemoryType(uint32_t typeFilter,
                        VkPhysicalDeviceMemoryProperties memProperties,
                        VkMemoryPropertyFlags properties);
};
};  // namespace vki
