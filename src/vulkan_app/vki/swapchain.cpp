#include "./swapchain.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <unordered_set>
#include <vector>

#include "./base.hpp"
#include "./logical_device.hpp"
#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/surface.hpp"

vki::SwapchainSharingInfo::SwapchainSharingInfo(
    const vki::QueueFamily &graphicsQueueFamily,
    const vki::QueueFamily &presentQueueFamily) {
    if (graphicsQueueFamily.index != presentQueueFamily.index) {
        queueIndices = { graphicsQueueFamily.index, presentQueueFamily.index };
    };
};

VkImageUsageFlags vki::imageUsageToVk(
    const std::unordered_set<vki::ImageUsage> &imageUsages) {
    VkImageUsageFlags flags;
    for (const auto &usage : imageUsages) {
        flags |= static_cast<VkImageUsageFlagBits>(usage);
    };
    return flags;
};

VkSwapchainCreateInfoKHR vki::SwapchainCreateInfo::toVkCreateInfo() const {
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface.getVkSurfaceKHR(),
        .minImageCount = minImageCount,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = imageArrayLayers,
        .imageUsage = imageUsageToVk(imageUsage),
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = preTransform,
        .compositeAlpha =
            static_cast<VkCompositeAlphaFlagBitsKHR>(compositeAlpha),
        .presentMode = static_cast<VkPresentModeKHR>(presentMode),
        .clipped = isClipped ? VK_TRUE : VK_FALSE,
        .oldSwapchain = oldSwapchain
                            .transform([](const vki::Swapchain *oldSwapchain) {
                                return oldSwapchain->getVkSwapchain();
                            })
                            .value_or(VK_NULL_HANDLE),
    };
    if (sharingInfo.queueIndices.has_value()) {
        const auto &queueIndices = sharingInfo.queueIndices.value();
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = queueIndices.size();
        createInfo.pQueueFamilyIndices = queueIndices.data();
    };
    return createInfo;
};

vki::Swapchain::Swapchain(const vki::LogicalDevice &logicalDevice,
                          const vki::SwapchainCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() } {
    const auto &vkCreateInfo = createInfo.toVkCreateInfo();
    VkResult result = vkCreateSwapchainKHR(
        logicalDevice.getVkDevice(), &vkCreateInfo, nullptr, &vkSwapchain);
    assertSuccess(result, "vkCreateSwapchainKHR");

    uint32_t swapChainImageCount;
    result = vkGetSwapchainImagesKHR(logicalDevice.getVkDevice(), vkSwapchain,
                                     &swapChainImageCount, nullptr);
    assertSuccess(result, "vkGetSwapchainImagesKHR");
    swapChainImages.resize(swapChainImageCount);
    result =
        vkGetSwapchainImagesKHR(logicalDevice.getVkDevice(), vkSwapchain,
                                &swapChainImageCount, swapChainImages.data());
    assertSuccess(result, "vkGetSwapchainImagesKHR");
    createImageViews(logicalDevice, vkCreateInfo.imageFormat);
};

void vki::Swapchain::createImageViews(const vki::LogicalDevice &logicalDevice,
                                      const VkFormat &format) {
    swapChainImageViews.resize(swapChainImages.size());
    int index = 0;
    for (const auto &image : swapChainImages) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        VkResult result =
            vkCreateImageView(logicalDevice.getVkDevice(), &createInfo, nullptr,
                              &swapChainImageViews[index]);
        assertSuccess(result, "vkCreateImageView");
        index++;
    };
};

uint32_t vki::Swapchain::acquireNextImageKHR(
    const vki::Semaphore &semaphore) const {
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device, vkSwapchain, std::numeric_limits<uint32_t>::max(),
        semaphore.getVkSemaphore(), VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUBOPTIMAL_KHR) {
        assertSuccess(result, "vkAcquireNextImageKHR");
    };
    return imageIndex;
};

vki::Swapchain::~Swapchain() {
    for (const auto &imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    };
    vkDestroySwapchainKHR(device, vkSwapchain, nullptr);
};

const VkSwapchainKHR vki::Swapchain::getVkSwapchain() const {
    return vkSwapchain;
};
