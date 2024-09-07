#pragma once

#include <vulkan/vulkan_core.h>

#include "easylogging++.h"
#include "vulkan_app/vki/descriptor_set_layout.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/render_pass.hpp"

vki::PipelineLayout createPipelineLayout(
    const vki::LogicalDevice &logicalDevice,
    const vki::DescriptorSetLayout &descriptorSetLayout);

vki::GraphicsPipeline createGraphicsPipeline(
    const vki::LogicalDevice &logicalDevice, el::Logger &logger,
    VkExtent2D swapchainExtent, const vki::RenderPass &renderPass,
    const vki::PipelineLayout &pipelineLayout,
    const VkSampleCountFlagBits &sampleCount);
