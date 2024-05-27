#include "./swapchain.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "./base.hpp"
#include "./logical_device.hpp"
#include "./physical_device.hpp"
#include "glfw_controller.hpp"

vki::Swapchain::Swapchain(const vki::LogicalDevice &logicalDevice,
                          const vki::PhysicalDevice &physicalDevice,
                          const VkSurfaceKHR &surface,
                          const GLFWControllerWindow &window)
    : device{ logicalDevice.getVkDevice() } {
    auto details = physicalDevice.getSwapchainDetails(surface);
    auto format = details.chooseFormat();
    auto presentMode = details.choosePresentMode();
    VkExtent2D swapChainExtent = details.chooseSwapExtent(window);
    VkFormat swapChainFormat = format.format;
    uint32_t imageCount = details.getImageCount();
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    unsigned int graphicsQueueIndex, presentQueueIndex;
    graphicsQueueIndex =
        physicalDevice.getFamilyTypeIndex(vki::QueueFamilyType::GRAPHIC);
    presentQueueIndex =
        physicalDevice.getFamilyTypeIndex(vki::QueueFamilyType::PRESENT);
    if (graphicsQueueIndex != presentQueueIndex) {
        std::vector<uint32_t> queueIndices = { graphicsQueueIndex,
                                               presentQueueIndex };
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueIndices.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    };
    VkResult result = vkCreateSwapchainKHR(logicalDevice.getVkDevice(),
                                           &createInfo, nullptr, &vkSwapchain);
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
    createImageViews(logicalDevice, format.format);
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

vki::Swapchain::~Swapchain() {
    for (const auto &imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    };
    vkDestroySwapchainKHR(device, vkSwapchain, nullptr);
};

const VkSwapchainKHR vki::Swapchain::getVkSwapchain() const {
    return vkSwapchain;
};
