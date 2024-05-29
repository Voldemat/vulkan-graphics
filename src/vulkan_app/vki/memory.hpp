#ifndef VKI_MEMORY
#define VKI_MEMORY

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/logical_device.hpp"

namespace vki {
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

#endif
