#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <unordered_set>
#include <vector>

namespace vki {
class LogicalDevice;

enum class AttachmentLoadOp {
    LOAD = VK_ATTACHMENT_LOAD_OP_LOAD,
    CLEAR = VK_ATTACHMENT_LOAD_OP_CLEAR,
    DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    NONE_EXT = VK_ATTACHMENT_LOAD_OP_NONE_EXT
};

enum class AttachmentStoreOp {
    STORE = VK_ATTACHMENT_STORE_OP_STORE,
    DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    NONE = VK_ATTACHMENT_STORE_OP_NONE,
    NONE_KHR = VK_ATTACHMENT_STORE_OP_NONE_KHR,
    NONE_QCOM = VK_ATTACHMENT_STORE_OP_NONE_QCOM,
    NONE_EXT = VK_ATTACHMENT_STORE_OP_NONE_EXT
};

enum class AttachmentDescriptionFlag {
    MAY_ALIAS = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT,
};

struct AttachmentDescription {
    std::unordered_set<vki::AttachmentDescriptionFlag> flags;
    VkFormat format;
    VkSampleCountFlagBits samples;
    vki::AttachmentLoadOp loadOp;
    vki::AttachmentStoreOp storeOp;
    vki::AttachmentLoadOp stencilLoadOp;
    vki::AttachmentStoreOp stencilStoreOp;
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;

    VkAttachmentDescription toVkDescription() const;
};

struct RenderPassCreateInfo {
    VkRenderPassCreateFlags flags;
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;

    VkRenderPassCreateInfo toVkCreateInfo() const {
        return {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = static_cast<uint32_t>(subpasses.size()),
            .pSubpasses = subpasses.data(),
            .dependencyCount = static_cast<uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data(),
        };
    };
};

class RenderPass {
    VkRenderPass vkRenderPass;
    VkDevice device;

protected:
    bool is_owner;

public:
    RenderPass(const RenderPass &other);
    explicit RenderPass(const vki::LogicalDevice &logicalDevice,
                        const vki::RenderPassCreateInfo &createInfo);
    const VkRenderPass getVkRenderPass() const;
    ~RenderPass();
};
};  // namespace vki
