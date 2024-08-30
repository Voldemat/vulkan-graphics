#pragma once

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/swapchain.hpp"

namespace vki {
class LogicalDevice;
class Framebuffer {
    VkFramebuffer vkFramebuffer;
    VkDevice device;

protected:
    bool is_owner;

public:
    Framebuffer(const vki::Framebuffer &other);
    Framebuffer(vki::Framebuffer &&other);
    Framebuffer(const vki::Framebuffer &&other) = delete;
    explicit Framebuffer(const vki::LogicalDevice &logicalDevice,
                         const VkFramebufferCreateInfo &createInfo);
    VkFramebuffer getVkFramebuffer() const;
    ~Framebuffer();
};
};  // namespace vki
