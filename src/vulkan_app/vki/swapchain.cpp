#include "./swapchain.hpp"
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>
#include "glfw_controller.hpp"
#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"

vki::Swapchain::Swapchain(
    const vki::LogicalDevice& logicalDevice,
    const vki::PhysicalDevice& physicalDevice,
    const VkSurfaceKHR& surface,
    const GLFWControllerWindow& window
) : device{logicalDevice.getVkDevice()} {
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
    graphicsQueueIndex = physicalDevice.getFamilyTypeIndex(vki::QueueFamilyType::GRAPHIC);
    presentQueueIndex = physicalDevice.getFamilyTypeIndex(vki::QueueFamilyType::PRESENT);
    if (graphicsQueueIndex != presentQueueIndex) {
        std::vector<uint32_t> queueIndices = {graphicsQueueIndex, presentQueueIndex};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueIndices.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    };
    VkResult result =
        vkCreateSwapchainKHR(logicalDevice.getVkDevice(), &createInfo, nullptr, &vkSwapchain);
    assertSuccess(result, "vkCreateSwapchainKHR");

    uint32_t swapChainImageCount;
    result = vkGetSwapchainImagesKHR(logicalDevice.getVkDevice(), vkSwapchain, &swapChainImageCount,
                                     nullptr);
    assertSuccess(result, "vkGetSwapchainImagesKHR");
    swapChainImages.resize(swapChainImageCount);
    result = vkGetSwapchainImagesKHR(logicalDevice.getVkDevice(), vkSwapchain, &swapChainImageCount,
                                     swapChainImages.data());
    assertSuccess(result, "vkGetSwapchainImagesKHR");

};

vki::Swapchain::~Swapchain() {
    vkDestroySwapchainKHR(device, vkSwapchain, nullptr);
};

const VkSwapchainKHR vki::Swapchain::getVkSwapchain() const {
    return vkSwapchain;
};

