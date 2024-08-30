#include "./render_pass.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::RenderPass::RenderPass(const vki::RenderPass &other)
    : is_owner{ false },
      device{ other.device },
      vkRenderPass{ other.vkRenderPass } {};

VkAttachmentDescription vki::AttachmentDescription::toVkDescription() const {
    return {
        .format = format,
        .samples = samples,
        .loadOp = static_cast<VkAttachmentLoadOp>(loadOp),
        .storeOp = static_cast<VkAttachmentStoreOp>(storeOp),
        .stencilLoadOp = static_cast<VkAttachmentLoadOp>(stencilLoadOp),
        .stencilStoreOp = static_cast<VkAttachmentStoreOp>(stencilStoreOp),
        .initialLayout = initialLayout,
        .finalLayout = finalLayout,
    };
};

vki::RenderPass::RenderPass(const vki::LogicalDevice &logicalDevice,
                            const vki::RenderPassCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    const auto &renderPassCreateInfo = createInfo.toVkCreateInfo();

    VkResult result =
        vkCreateRenderPass(logicalDevice.getVkDevice(), &renderPassCreateInfo,
                           nullptr, &vkRenderPass);
    vki::assertSuccess(result, "vkCreateRenderPass");
};

const VkRenderPass vki::RenderPass::getVkRenderPass() const {
    return vkRenderPass;
};

vki::RenderPass::~RenderPass() {
    if (is_owner) {
        vkDestroyRenderPass(device, vkRenderPass, nullptr);
    };
};
