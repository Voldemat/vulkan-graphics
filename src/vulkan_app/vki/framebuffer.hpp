#ifndef VKI_FRAMEBUFFER
#define VKI_FRAMEBUFFER

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/swapchain.hpp"
namespace vki {
class Framebuffer {
    VkFramebuffer vkFramebuffer;
    VkDevice device;

public:
    explicit Framebuffer(const vki::Swapchain &swapchain,
                         const vki::RenderPass &renderPass, VkExtent2D extent,
                         const vki::LogicalDevice &logicalDevice,
                         const VkImageView imageView);
    VkFramebuffer getVkFramebuffer() const;
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer(const Framebuffer&&) = delete;
    ~Framebuffer();
};
};  // namespace vki
#endif
