#pragma once

#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class PipelineLayout {
    VkPipelineLayout vkPipelineLayout;
    VkDevice device;

public:
    VkPipelineLayout getVkPipelineLayout() const;
    explicit PipelineLayout(const vki::LogicalDevice &logicalDevice);
    ~PipelineLayout();
};
};  // namespace vki
