#ifndef VKI_RENDER_PASS
#define VKI_RENDER_PASS
#include <vulkan/vulkan_core.h>

#include "./logical_device.hpp"

namespace vki {
class RenderPass {
    VkRenderPass vkRenderPass;
    const vki::LogicalDevice& device;

public:
    explicit RenderPass(VkFormat format,
                        const vki::LogicalDevice &logicalDevice);
    const VkRenderPass getVkRenderPass() const;
    ~RenderPass();
};
};  // namespace vki
#endif
