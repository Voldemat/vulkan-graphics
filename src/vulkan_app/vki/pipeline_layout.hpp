#pragma once

#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class PipelineLayout {
    VkPipelineLayout vkPipelineLayout;
    VkDevice device;
protected:
    bool is_owner;
public:
    PipelineLayout(const PipelineLayout& other);
    VkPipelineLayout getVkPipelineLayout() const;
    explicit PipelineLayout(const vki::LogicalDevice &logicalDevice,
                            const VkPipelineLayoutCreateInfo &createInfo);
    ~PipelineLayout();
};
};  // namespace vki
