#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/surface.hpp"

namespace vki {
class LogicalDevice;

enum class ImageUsage {
    TRANSFER_SRC = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    TRANSFER_DST = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    SAMPLED = VK_IMAGE_USAGE_SAMPLED_BIT,
    STORAGE = VK_IMAGE_USAGE_STORAGE_BIT,
    COLOR_ATTACHMENT = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    TRANSIENT_ATTACHMENT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    INPUT_ATTACHMENT = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
    VIDEO_DECODE_DST_KHR = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
    VIDEO_DECODE_SRC_KHR = VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
    VIDEO_DECODE_DPB_KHR = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
    FRAGMENT_DENSITY_MAP_EXT = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
    FRAGMENT_SHADING_RATE_ATTACHMENT_KHR =
        VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
    ATTACHMENT_FEEDBACK_LOOP_EXT =
        VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT,
    INVOCATION_MASK_HUAWEI = VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI,
    SAMPLE_WEIGHT_QCOM = VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM,
    SAMPLE_BLOCK_MATCH_QCOM = VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM,
    SHADING_RATE_IMAGE_NV = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
};

enum class CompositeAlpha {
    OPAQUE_BIT_KHR = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    PRE_MULTIPLIED_BIT_KHR = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    POST_MULTIPLIED_BIT_KHR = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    INHERIT_BIT_KHR = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
};
class Swapchain;

VkImageUsageFlags imageUsageToVk(
    const std::unordered_set<vki::ImageUsage> &imageUsages);

struct SwapchainSharingInfo {
    std::optional<std::vector<unsigned int>> queueIndices;
    explicit SwapchainSharingInfo(
        const vki::QueueFamily& graphicsQueueFamily,
        const vki::QueueFamily& presentQueueFamily);
};

struct SwapchainCreateInfo {
    vki::Surface surface;
    VkExtent2D extent;
    vki::PresentMode presentMode;
    VkSurfaceFormatKHR format;
    uint32_t minImageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
    unsigned int imageArrayLayers = 1;
    std::unordered_set<vki::ImageUsage> imageUsage;
    vki::CompositeAlpha compositeAlpha;
    bool isClipped;
    std::optional<vki::Swapchain *> oldSwapchain;
    vki::SwapchainSharingInfo sharingInfo;

    VkSwapchainCreateInfoKHR toVkCreateInfo() const;
};

class Swapchain {
    const VkDevice device;
    VkSwapchainKHR vkSwapchain;
    void createImageViews(const vki::LogicalDevice &logicalDevice,
                          const VkFormat &format);

public:
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    explicit Swapchain(const vki::LogicalDevice &logicalDevice,
                       const vki::SwapchainCreateInfo &createInfo);
    const VkSwapchainKHR getVkSwapchain() const;
    uint32_t acquireNextImageKHR(const vki::Semaphore &semaphore) const;
    ~Swapchain();
};

};  // namespace vki
