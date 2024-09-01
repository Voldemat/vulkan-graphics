#include "./descriptor_pool.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::DescriptorPool::DescriptorPool(const vki::DescriptorPool &other)
    : device{ other.device },
      vkDescriptorPool{ other.vkDescriptorPool },
      is_owner{ false } {};

vki::DescriptorPool::DescriptorPool(
    const vki::LogicalDevice &logicalDevice,
    const VkDescriptorPoolCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    VkResult result = vkCreateDescriptorPool(
        logicalDevice.getVkDevice(), &createInfo, nullptr, &vkDescriptorPool);
    vki::assertSuccess(result, "vkCreateDescriptorPool");
};

vki::DescriptorPool::~DescriptorPool() {
    if (is_owner) {
        vkDestroyDescriptorPool(device, vkDescriptorPool, nullptr);
    };
};
