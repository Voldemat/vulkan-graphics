#ifndef VKI_BUFFER
#define VKI_BUFFER
#include <vulkan/vulkan_core.h>
#include <memory>
#include <optional>

#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/memory.hpp"

namespace vki {
class Buffer {
    VkBuffer vkBuffer;
    VkDevice device;
    std::optional<std::shared_ptr<vki::Memory>> memory;

public:
    explicit Buffer(const vki::LogicalDevice &logicalDevice,
                    VkBufferCreateInfo createInfo);
    VkBuffer getVkBuffer() const;
    void bindMemory(const std::shared_ptr<vki::Memory>& memory);
    VkMemoryRequirements getMemoryRequirements() const;
    ~Buffer();
};
};  // namespace vki
#endif
