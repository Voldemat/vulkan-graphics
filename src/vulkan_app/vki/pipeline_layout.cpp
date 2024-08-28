#include "./pipeline_layout.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::PipelineLayout::PipelineLayout(const vki::LogicalDevice &logicalDevice)
    : device{ logicalDevice.getVkDevice() } {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    VkResult result = vkCreatePipelineLayout(logicalDevice.getVkDevice(),
                                             &pipelineLayoutCreateInfo, nullptr,
                                             &vkPipelineLayout);
    vki::assertSuccess(result, "vkCreatePipelineLayout");
};

vki::PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(device, vkPipelineLayout, nullptr);
};

VkPipelineLayout vki::PipelineLayout::getVkPipelineLayout() const {
    return vkPipelineLayout;
};
