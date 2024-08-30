#include "./semaphore.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::Semaphore::Semaphore(const vki::LogicalDevice &logicalDevice)
    : device{ logicalDevice.getVkDevice() } {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkResult result = vkCreateSemaphore(logicalDevice.getVkDevice(),
                                        &semaphoreInfo, nullptr, &vkSemaphore);
    vki::assertSuccess(result, "vkCreateSemaphore");
};

const VkSemaphore vki::Semaphore::getVkSemaphore() const {
    return vkSemaphore;
};

vki::Semaphore::~Semaphore() {
    vkDestroySemaphore(device, vkSemaphore, nullptr);
};
