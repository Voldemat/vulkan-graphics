#include "./pipeline_layout.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::PipelineLayout::PipelineLayout(
    const vki::LogicalDevice &logicalDevice,
    const VkPipelineLayoutCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() } {
    VkResult result = vkCreatePipelineLayout(
        logicalDevice.getVkDevice(), &createInfo, nullptr, &vkPipelineLayout);
    vki::assertSuccess(result, "vkCreatePipelineLayout");
};

vki::PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(device, vkPipelineLayout, nullptr);
};

VkPipelineLayout vki::PipelineLayout::getVkPipelineLayout() const {
    return vkPipelineLayout;
};
