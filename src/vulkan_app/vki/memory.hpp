#pragma once
#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class Memory {
    VkDeviceMemory vkMemory;
    const VkDevice device;

public:
    explicit Memory(const vki::LogicalDevice &device,
                    VkMemoryAllocateInfo allocInfo);
    VkDeviceMemory getVkMemory() const;
    void mapMemory(VkDeviceSize size, void **buffer) const;
    void unmapMemory() const;
    ~Memory();
};
};  // namespace vki
