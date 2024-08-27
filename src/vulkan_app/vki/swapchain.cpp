#include "./swapchain.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "./base.hpp"
#include "./logical_device.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/surface.hpp"

vki::SwapchainCreateInfo::SwapchainCreateInfo(
    const vki::SwapchainCreateInfoInput &input)
    : surface{ input.surface },
      extent{ input.extent },
      presentMode{ input.presentMode },
      format{ input.format },
      minImageCount{ input.minImageCount },
      preTransform{ input.preTransform } {
    if (input.graphicsQueueFamily->index != input.presentQueueFamily->index) {
        queueIndices = { input.graphicsQueueFamily->index,
                         input.presentQueueFamily->index };
    };
};

VkSwapchainCreateInfoKHR vki::SwapchainCreateInfo::toVkCreateInfo() const {
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface.getVkSurfaceKHR(),
        .minImageCount = minImageCount,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = preTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    if (queueIndices.has_value()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueIndices.value().data();
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
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
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
    assertSuccess(result, "vkAcquireNextImageKHR");
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
