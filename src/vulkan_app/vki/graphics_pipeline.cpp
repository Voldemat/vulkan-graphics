#include "./graphics_pipeline.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/shader_module.hpp"

vki::GraphicsPipeline::GraphicsPipeline(
    const vki::LogicalDevice &logicalDevice,
    const VkGraphicsPipelineCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() } {
    VkResult result =
        vkCreateGraphicsPipelines(logicalDevice.getVkDevice(), VK_NULL_HANDLE,
                                  1, &createInfo, nullptr, &vkPipeline);
    vki::assertSuccess(result, "vkCreateGraphicsPipelines");
};

VkPipeline vki::GraphicsPipeline::getVkPipeline() const { return vkPipeline; };

vki::GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(device, vkPipeline, nullptr);
};
