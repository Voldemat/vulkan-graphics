#include "./fence.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::Fence::Fence(const vki::LogicalDevice &logicalDevice)
    : device{ logicalDevice.getVkDevice() } {
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    VkResult result = vkCreateFence(logicalDevice.getVkDevice(), &fenceInfo,
                                    nullptr, &vkFence);
    vki::assertSuccess(result, "vkCreateFence");
};

const VkFence vki::Fence::getVkFence() const { return vkFence; };

void vki::Fence::wait() const {
    VkResult result = vkWaitForFences(device, 1, &vkFence, VK_TRUE, UINT64_MAX);
    assertSuccess(result, "vkWaitForFences");
    result = vkResetFences(device, 1, &vkFence);
    assertSuccess(result, "vkResetFences");
};

vki::Fence::~Fence() { vkDestroyFence(device, vkFence, nullptr); };
