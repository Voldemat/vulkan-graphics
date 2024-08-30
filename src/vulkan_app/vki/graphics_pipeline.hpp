#pragma once

#include <vulkan/vulkan_core.h>

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
        const vki::LogicalDevice &logicalDevice,
        const VkGraphicsPipelineCreateInfo& createInfo);
    VkPipeline getVkPipeline() const;
    ~GraphicsPipeline();
};
};  // namespace vki
