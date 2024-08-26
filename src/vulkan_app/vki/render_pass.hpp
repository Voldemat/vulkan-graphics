#ifndef VKI_RENDER_PASS
#define VKI_RENDER_PASS
#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class RenderPass {
    VkRenderPass vkRenderPass;
    const vki::LogicalDevice& device;

public:
    RenderPass(const RenderPass& other) = delete;
    explicit RenderPass(VkFormat format,
                        const vki::LogicalDevice &logicalDevice);
    const VkRenderPass getVkRenderPass() const;
    ~RenderPass();
};
};  // namespace vki
#endif
