#pragma once
#include <vulkan/vulkan_core.h>

#include <optional>

#include "vulkan_app/vki/memory.hpp"

namespace vki {
class LogicalDevice;
class Buffer {
    VkBuffer vkBuffer;
    VkDevice device;
    std::optional<vki::Memory> memory;

public:
    Buffer(const Buffer &other) = delete;
    explicit Buffer(const vki::LogicalDevice &logicalDevice,
                    VkBufferCreateInfo createInfo);
    VkBuffer getVkBuffer() const;
    void bindMemory(const vki::Memory &memory);
    VkMemoryRequirements getMemoryRequirements() const;
    ~Buffer();
};
};  // namespace vki
