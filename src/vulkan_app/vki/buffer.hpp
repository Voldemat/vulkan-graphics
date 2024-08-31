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
protected:
    bool is_owner;
public:
    Buffer(const Buffer &&other) = delete;
    Buffer(Buffer &&other);
    Buffer(const Buffer &other);
    explicit Buffer(const vki::LogicalDevice &logicalDevice,
                    VkBufferCreateInfo createInfo);
    VkBuffer getVkBuffer() const;
    void bindMemory(vki::Memory &&memory);
    const std::optional<vki::Memory> getMemory() const;
    VkMemoryRequirements getMemoryRequirements() const;
    ~Buffer();
};
};  // namespace vki
