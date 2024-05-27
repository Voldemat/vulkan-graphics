#ifndef VKI_COMMAND_BUFFER
#define VKI_COMMAND_BUFFER

#include <vulkan/vulkan_core.h>
namespace vki {
class CommandBuffer {
    VkCommandBuffer vkCommandBuffer;
public:
    explicit CommandBuffer(const VkCommandPool& commandPool, const VkDevice& logicalDevice);
    const VkCommandBuffer getVkCommandBuffer() const;
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer(const CommandBuffer&&) = delete;
};
};
#endif
