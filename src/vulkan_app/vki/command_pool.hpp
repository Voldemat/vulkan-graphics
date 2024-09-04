#pragma once

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/queue_family.hpp"

namespace vki {
class CommandPool {
    VkCommandPool vkCommandPool;
    VkDevice device;

    void init(const unsigned int &queueFamilyIndex);

public:
    template <unsigned int AvailableQueueCount,
              enum vki::QueueOperationType... T>
    explicit CommandPool(
        const vki::LogicalDevice &logicalDevice,
        const vki::QueueFamilyWithOp<AvailableQueueCount,
                                     vki::QueueOperationType::GRAPHIC, T...>
            &graphicQueueFamily)
        : device{ logicalDevice.getVkDevice() } {
        init(graphicQueueFamily.family.index);
    };
    const VkCommandPool getVkCommandPool() const;
    vki::CommandBuffer createCommandBuffer() const;
    ~CommandPool();
};
};  // namespace vki
