#pragma once

#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class RenderPass {
    VkRenderPass vkRenderPass;
    const vki::LogicalDevice &device;
protected:
    bool is_owner;

public:
    RenderPass(const RenderPass &other);
    explicit RenderPass(VkFormat format,
                        const vki::LogicalDevice &logicalDevice);
    const VkRenderPass getVkRenderPass() const;
    ~RenderPass();
};
};  // namespace vki
