#include "./framebuffer.hpp"

#include <vulkan/vulkan_core.h>


#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/swapchain.hpp"

vki::Framebuffer::Framebuffer(const vki::Swapchain &swapchain,
                              const vki::RenderPass &renderPass,
                              VkExtent2D extent,
                              const vki::LogicalDevice &logicalDevice,
                              const VkImageView imageView)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    VkImageView attachments[] = { imageView };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.getVkRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    VkResult result = vkCreateFramebuffer(
        logicalDevice.getVkDevice(), &framebufferInfo, nullptr, &vkFramebuffer);
    vki::assertSuccess(result, "vkCreateFramebuffer");
};

vki::Framebuffer::Framebuffer(const vki::Framebuffer &other)
    : device{ other.device },
      is_owner{ false },
      vkFramebuffer{ other.vkFramebuffer } {};
vki::Framebuffer::Framebuffer(vki::Framebuffer &&other)
    : device{ other.device },
      is_owner{ other.is_owner },
      vkFramebuffer{ other.vkFramebuffer } {
    other.is_owner = false;
};

VkFramebuffer vki::Framebuffer::getVkFramebuffer() const {
    return vkFramebuffer;
};

vki::Framebuffer::~Framebuffer() {
    if (is_owner) {
        vkDestroyFramebuffer(device, vkFramebuffer, nullptr);
    };
};
