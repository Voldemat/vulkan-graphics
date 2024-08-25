#include "./physical_device.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "./base.hpp"
#include "glfw_controller.hpp"
#include "magic_enum.hpp"
#include "vulkan_app/vki/surface.hpp"

vki::PhysicalDevice::operator std::string() const {
    return std::format(
        "PhysicalDevice(name: {}, type: {}, vendorID: {}, deviceID: {}, "
        "apiVersion: {}, driverVersion: {})",
        std::string(properties.deviceName),
        magic_enum::enum_name(properties.deviceType), properties.vendorID,
        properties.deviceID, properties.apiVersion, properties.driverVersion);
};

vki::PhysicalDevice::PhysicalDevice(const VkPhysicalDevice &dev,
                                    const vki::Surface &surface)
    : device{ dev }, properties{ getProperties() } {
    saveQueueFamilies(surface);
};

std::set<vki::QueueOperationType> operationsFromFlags(
    const VkQueueFlags &flags) {
    std::set<vki::QueueOperationType> operations;
    if (flags & VK_QUEUE_GRAPHICS_BIT) {
        operations.insert(vki::QueueOperationType::GRAPHIC);
    } else if (flags & VK_QUEUE_COMPUTE_BIT) {
        operations.insert(vki::QueueOperationType::COMPUTE);
    } else if (flags & VK_QUEUE_TRANSFER_BIT) {
        operations.insert(vki::QueueOperationType::TRANSFER);
    } else if (flags & VK_QUEUE_PROTECTED_BIT) {
        operations.insert(vki::QueueOperationType::PROTECTED);
    } else if (flags & VK_QUEUE_SPARSE_BINDING_BIT) {
        operations.insert(vki::QueueOperationType::SPARSE_BINDING);
    } else if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
        operations.insert(vki::QueueOperationType::OPTICAL_FLOW);
    } else if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
        operations.insert(vki::QueueOperationType::VIDEO_DECODE);
    }
#ifdef VK_ENABLE_BETA_EXTENSIONS
    else if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
        operations.insert(vki::QueueOperationType::VIDEO_ENCODE);
    }
#endif
    return operations;
};

vki::QueueFamily::operator std::string() const {
    return std::format(
        "QueueFamily(index: {}, queueCount: {}, timestamp_valid_bits: {}, "
        "minImageTransferGranularity: VkExtent3D(height: {}, width: {}, "
        "height: {}), supportedOperations: {})",
        index, queueCount, timestamp_valid_bits,
        minImageTransferGranularity.height, minImageTransferGranularity.width,
        minImageTransferGranularity.height,
        operationsToString(supportedOperations));
};

bool vki::QueueFamily::doesSupportsOperations(
    const std::set<vki::QueueOperationType> &ops) const {
    return std::includes(supportedOperations.begin(), supportedOperations.end(),
                         ops.begin(), ops.end());
};

bool vki::PhysicalDevice::hasQueueFamilies(
    const std::vector<std::function<bool(const QueueFamily &)>> &funcs) const {
    return std::ranges::all_of(
        funcs.begin(), funcs.end(), [this](const auto &func) -> bool {
            return std::ranges::any_of(
                queueFamilies.begin(), queueFamilies.end(),
                [&func](const auto &queueFamily) -> bool {
                    return func(*queueFamily);
                });
        });
};

bool vki::PhysicalDevice::hasExtensions(
    const std::vector<std::function<bool(const VkExtensionProperties &)>>
        &funcs) const {
    const auto &extensions = getExtensions();
    return std::ranges::all_of(
        funcs.begin(), funcs.end(), [&extensions](const auto &func) -> bool {
            return std::ranges::any_of(extensions.begin(), extensions.end(),
                                       [&func](const auto &extension) -> bool {
                                           return func(extension);
                                       });
        });
};

void vki::PhysicalDevice::saveQueueFamilies(const vki::Surface &surface) {
    unsigned int i = 0;
    for (const auto &queueFamily : getQueueFamiliesProperties()) {
        auto supportedOperations = operationsFromFlags(queueFamily.queueFlags);
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            device, i, surface.getVkSurfaceKHR(), &presentSupport);
        if (result != VK_SUCCESS) {
            throw VulkanError(result, "vkGetPhysicalDeviceSurfaceSupportKHR");
        };
        if (presentSupport) {
            supportedOperations.insert(vki::QueueOperationType::PRESENT);
        };
        queueFamilies.push_back(std::make_shared<vki::QueueFamily>(
            i, queueFamily.queueCount, queueFamily.timestampValidBits,
            queueFamily.minImageTransferGranularity, supportedOperations));
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

// auto vki::PhysicalDevice::getSwapchainDetails() const {
//     auto details =
//         queryDeviceSwapChainSupportDetails(device);
//     VkSurfaceFormatKHR format = chooseFormat(details);
//     VkPresentModeKHR presentMode = choosePresentMode(details);
//     swapChainExtent = chooseSwapExtent(details.capabilities, window);
//     swapChainFormat = format.format;
// };

VkSurfaceCapabilitiesKHR vki::PhysicalDevice::getSurfaceCapabilities(
    const vki::Surface &surface) const {
    VkSurfaceCapabilitiesKHR details;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, surface.getVkSurfaceKHR(), &details);
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    return details;
};

std::vector<VkSurfaceFormatKHR> vki::PhysicalDevice::getSurfaceFormats(
    const vki::Surface &surface) const {
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t formatCount;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface.getVkSurfaceKHR(), &formatCount, nullptr);
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    formats.resize(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface.getVkSurfaceKHR(), &formatCount, formats.data());
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    return formats;
};

std::vector<VkPresentModeKHR> vki::PhysicalDevice::getSurfacePresentModes(
    const vki::Surface &surface) const {
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t modesCount;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface.getVkSurfaceKHR(), &modesCount, nullptr);
    assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    presentModes.resize(modesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface.getVkSurfaceKHR(), &modesCount, presentModes.data());
    assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    return presentModes;
};

vki::SwapChainSupportDetails vki::PhysicalDevice::getSwapchainDetails(
    const vki::Surface &surface) const {
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

VkPhysicalDeviceMemoryProperties vki::PhysicalDevice::getMemoryProperties()
    const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
    return memProperties;
};

std::vector<VkExtensionProperties> vki::PhysicalDevice::getExtensions() const {
    uint32_t extensionCount;
    VkResult result = vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS) {
        throw vki::VulkanError(result, "vkEnumerateDeviceExtensionProperties");
    };

    std::vector<VkExtensionProperties> extensions(extensionCount);
    result = vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extensionCount, extensions.data());
    if (result != VK_SUCCESS) {
        throw vki::VulkanError(result, "vkEnumerateDeviceExtensionProperties");
    };
    return extensions;
};

std::vector<std::shared_ptr<vki::QueueFamily>>
vki::PhysicalDevice::getQueueFamilies() const {
    return queueFamilies;
};

std::string vki::operationsToString(
    std::set<vki::QueueOperationType> operations) {
    std::string str = "{";
    unsigned int index = 0;
    for (const auto &op : operations) {
        str += magic_enum::enum_name(op);
        index++;
        if (index < operations.size()) {
            str += ",";
        };
    };
    str += "}";
    return str;
};
