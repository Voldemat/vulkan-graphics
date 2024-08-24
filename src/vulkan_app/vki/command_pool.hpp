#ifndef VKI_COMMAND_POOL
#define VKI_COMMAND_POOL

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/queue.hpp"
namespace vki {
class CommandPool {
    VkCommandPool vkCommandPool;
    VkDevice device;

public:
    explicit CommandPool(
        const vki::LogicalDevice &logicalDevice,
        const vki::Queue<vki::QueueOperationType::GRAPHIC> &physicalDevice);
    const VkCommandPool getVkCommandPool() const;
    vki::CommandBuffer createCommandBuffer() const;
    ~CommandPool();
};
};  // namespace vki
#endif
