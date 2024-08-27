#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>

#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/shader_module.hpp"

namespace vki {
class LogicalDevice;
class GraphicsPipeline {
    VkPipeline vkPipeline;
    VkDevice device;

public:
    explicit GraphicsPipeline(
        const vki::ShaderModule &vertShader,
        const vki::ShaderModule &fragmentShader, VkExtent2D extent,
        const vki::PipelineLayout &pipelineLayout,
        const std::shared_ptr<vki::RenderPass> &renderPass,
        const vki::LogicalDevice &logicalDevice,
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo);
    VkPipeline getVkPipeline() const;
    ~GraphicsPipeline();
};
};  // namespace vki
