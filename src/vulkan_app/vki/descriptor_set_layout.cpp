#include "./descriptor_set_layout.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::DescriptorSetLayout::DescriptorSetLayout(
    const vki::LogicalDevice &logicalDevice,
    const VkDescriptorSetLayoutCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() } {
    VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr,
                                                  &vkDescriptorSetLayout);
    vki::assertSuccess(result, "vkCreateDescriptorSetLayout");
};

vki::DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(device, vkDescriptorSetLayout, nullptr);
};
