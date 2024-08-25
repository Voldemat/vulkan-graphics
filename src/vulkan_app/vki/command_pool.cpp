#include "./command_pool.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/command_buffer.hpp"

void vki::CommandPool::init(const unsigned int &queueFamilyIndex) {
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags =
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VkResult result = vkCreateCommandPool(device, &commandPoolCreateInfo,
                                          nullptr, &vkCommandPool);
    vki::assertSuccess(result, "vkCreateCommandPool");
};

const VkCommandPool vki::CommandPool::getVkCommandPool() const {
    return vkCommandPool;
};

vki::CommandBuffer vki::CommandPool::createCommandBuffer() const {
    return vki::CommandBuffer(vkCommandPool, device);
};

vki::CommandPool::~CommandPool() {
    vkDestroyCommandPool(device, vkCommandPool, nullptr);
};
