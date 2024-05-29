#include "./physical_device.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include "./base.hpp"
#include "glfw_controller.hpp"

vki::PhysicalDevice::PhysicalDevice(const VkPhysicalDevice &dev,
                                    const VkSurfaceKHR &surface)
    : device{ dev } {
    saveQueueFamilyIndexes(surface);
};

void vki::PhysicalDevice::saveQueueFamilyIndexes(const VkSurfaceKHR &surface) {
    unsigned int i = 0;
    for (const auto &queueFamily : getQueueFamiliesProperties()) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyTypeToIndex[vki::QueueFamilyType::GRAPHIC] = i;
        };
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            device, i, surface, &presentSupport);
        if (result != VK_SUCCESS) {
            throw VulkanError(result, "vkGetPhysicalDeviceSurfaceSupportKHR");
        };
        if (presentSupport) {
            queueFamilyTypeToIndex[vki::QueueFamilyType::PRESENT] = i;
        };
        i++;
    }
};

VkPhysicalDeviceProperties vki::PhysicalDevice::getProperties() const {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    return properties;
};

VkPhysicalDevice vki::PhysicalDevice::getVkDevice() const { return device; };

std::vector<VkQueueFamilyProperties>
vki::PhysicalDevice::getQueueFamiliesProperties() const {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());
    return queueFamilies;
};

bool vki::PhysicalDevice::isSuitable() const {
    return queueFamilyTypeToIndex.contains(vki::QueueFamilyType::GRAPHIC) &&
           queueFamilyTypeToIndex.contains(vki::QueueFamilyType::PRESENT);
};

unsigned int vki::PhysicalDevice::getFamilyTypeIndex(
    vki::QueueFamilyType type) const {
    return queueFamilyTypeToIndex.at(type);
};

// auto vki::PhysicalDevice::getSwapchainDetails() const {
//     auto details =
//         queryDeviceSwapChainSupportDetails(device);
//     VkSurfaceFormatKHR format = chooseFormat(details);
//     VkPresentModeKHR presentMode = choosePresentMode(details);
//     swapChainExtent = chooseSwapExtent(details.capabilities, window);
//     swapChainFormat = format.format;
// };

VkSurfaceCapabilitiesKHR vki::PhysicalDevice::getSurfaceCapabilities(
    const VkSurfaceKHR &surface) const {
    VkSurfaceCapabilitiesKHR details;
    VkResult result =
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details);
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    return details;
};

std::vector<VkSurfaceFormatKHR> vki::PhysicalDevice::getSurfaceFormats(
    const VkSurfaceKHR &surface) const {
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t formatCount;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface, &formatCount, nullptr);
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    formats.resize(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                                  formats.data());
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    return formats;
};

std::vector<VkPresentModeKHR> vki::PhysicalDevice::getSurfacePresentModes(
    const VkSurfaceKHR &surface) const {
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t modesCount;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &modesCount, nullptr);
    assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    presentModes.resize(modesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &modesCount, presentModes.data());
    assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    return presentModes;
};

vki::SwapChainSupportDetails vki::PhysicalDevice::getSwapchainDetails(
    const VkSurfaceKHR &surface) const {
    vki::SwapChainSupportDetails details;
    details.capabilities = getSurfaceCapabilities(surface);
    details.formats = getSurfaceFormats(surface);
    details.presentModes = getSurfacePresentModes(surface);
    if (details.formats.empty() || details.presentModes.empty()) {
        throw std::runtime_error("Formats or presentModes are empty");
    };
    return details;
};

VkPresentModeKHR vki::SwapChainSupportDetails::choosePresentMode() const {
    for (const auto &presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        };
    };
    return VK_PRESENT_MODE_FIFO_KHR;
};

VkSurfaceFormatKHR vki::SwapChainSupportDetails::chooseFormat() const {
    for (const auto &format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        };
    };
    throw std::runtime_error("Required surface format is not found");
};

VkExtent2D vki::SwapChainSupportDetails::chooseSwapExtent(
    const GLFWControllerWindow &window) const {
    if (capabilities.currentExtent.width <=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    };
    const auto &[width, height] = window.getFramebufferSize();
    VkExtent2D actualExtent = { static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height) };
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
    return actualExtent;
};

uint32_t vki::SwapChainSupportDetails::getImageCount() const {
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    };
    return imageCount;
};


VkPhysicalDeviceMemoryProperties vki::PhysicalDevice::getMemoryProperties() const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
    return memProperties;
};
