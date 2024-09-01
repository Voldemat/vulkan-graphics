#include "./pipeline_layout.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::PipelineLayout::PipelineLayout(const vki::PipelineLayout &other)
    : device{ other.device },
      is_owner{ false },
      vkPipelineLayout{ other.vkPipelineLayout } {};

vki::PipelineLayout::PipelineLayout(
    const vki::LogicalDevice &logicalDevice,
    const VkPipelineLayoutCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    VkResult result = vkCreatePipelineLayout(
        logicalDevice.getVkDevice(), &createInfo, nullptr, &vkPipelineLayout);
    vki::assertSuccess(result, "vkCreatePipelineLayout");
};

vki::PipelineLayout::~PipelineLayout() {
    if (is_owner) {
        vkDestroyPipelineLayout(device, vkPipelineLayout, nullptr);
    };
};

VkPipelineLayout vki::PipelineLayout::getVkPipelineLayout() const {
    return vkPipelineLayout;
};
