#include "./command_buffer.hpp"

#include <vulkan/vulkan_core.h>
#include "vulkan_app/vki/base.hpp"

vki::CommandBuffer::CommandBuffer(const VkCommandPool &commandPool,
                                  const VkDevice &logicalDevice){

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(logicalDevice,
                                               &allocInfo, &vkCommandBuffer);
    vki::assertSuccess(result, "vkAllocateCommandBuffers");
};

const VkCommandBuffer vki::CommandBuffer::getVkCommandBuffer() const {
    return vkCommandBuffer;
};
