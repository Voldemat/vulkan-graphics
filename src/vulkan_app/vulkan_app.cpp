#include "./vulkan_app.hpp"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_metal.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <ios>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"
#include "glfw_controller.hpp"

std::string physicalDeviceTypeToString(const VkPhysicalDeviceType &type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            return "MAX_ENUM";
    };
};
void VulkanApplication::createInstance(std::vector<const char *> extensions) {
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pNext = nullptr;

    const char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation",
    };
    createInfo = new VkInstanceCreateInfo();
    createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo->pApplicationInfo = &appInfo;
    createInfo->enabledExtensionCount = (uint32_t)extensions.size();
    createInfo->ppEnabledExtensionNames = extensions.data();
    createInfo->enabledLayerCount = 1;
    createInfo->ppEnabledLayerNames = validationLayers;
    createInfo->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    VkResult result = vkCreateInstance(createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "vkCreateInstance");
    };
};

void VulkanApplication::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find gpu with vulkan support");
    };
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        std::cout << std::format(
                         "Name: {}\nType: {}", properties.deviceName,
                         physicalDeviceTypeToString(properties.deviceType))
                  << std::endl;
        std::cout << "Geometry shader: " << std::boolalpha
                  << (bool)deviceFeatures.geometryShader << std::endl;
        std::cout << "independentBlend: " << std::boolalpha
                  << (bool)deviceFeatures.independentBlend << std::endl;
    };
    physicalDevice = devices[0];
};

void VulkanApplication::pickQueueFamilies() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilies.data());
    uint32_t i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
        };
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice, i, windowSurface, &presentSupport);
        if (result != VK_SUCCESS) {
            throw VulkanError(result, "vkGetPhysicalDeviceSurfaceSupportKHR");
        };
        if (presentSupport) {
            presentQueueIndex = i;
        };
        i++;
    }
    if (!graphicsQueueIndex.has_value()) {
        throw std::runtime_error(
            "Suitable queue graphics family was not found");
    };
};

void VulkanApplication::createLogicalDevice() {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
    graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueIndex.value();
    graphicsQueueCreateInfo.queueCount = 1;
    graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfoArray;
    queueCreateInfoArray.push_back(graphicsQueueCreateInfo);
    if (presentQueueIndex != graphicsQueueIndex) {
        VkDeviceQueueCreateInfo presentQueueCreateInfo{};
        presentQueueCreateInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentQueueCreateInfo.queueFamilyIndex = presentQueueIndex.value();
        presentQueueCreateInfo.queueCount = 1;
        presentQueueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfoArray.push_back(presentQueueCreateInfo);
    };

    std::vector<const char *> deviceExtensions;
    deviceExtensions.push_back("VK_KHR_portability_subset");
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfoArray.size();
    createInfo.pQueueCreateInfos = queueCreateInfoArray.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult result =
        vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "vkCreateDevice");
    };
    vkGetDeviceQueue(device, graphicsQueueIndex.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueIndex.value(), 0, &presentQueue);
};

void VulkanApplication::createSwapChain(const GLFWControllerWindow &window) {
    auto details = queryDeviceSwapChainSupportDetails(physicalDevice);
    VkSurfaceFormatKHR format = chooseFormat(details);
    VkPresentModeKHR presentMode = choosePresentMode(details);
    swapChainExtent = chooseSwapExtent(details.capabilities, window);
    swapChainFormat = format.format;
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    };
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = windowSurface;
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
    if (graphicsQueueIndex != presentQueueIndex) {
        std::vector<uint32_t> queueIndices;
        queueIndices.push_back(graphicsQueueIndex.value());
        queueIndices.push_back(presentQueueIndex.value());
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueIndices.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    };
    VkResult result =
        vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
    assertSuccess(result);

    uint32_t swapChainImageCount;
    result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount,
                                     nullptr);
    assertSuccess(result);
    swapChainImages.resize(swapChainImageCount);
    result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount,
                                     swapChainImages.data());
    assertSuccess(result);
};

void assertSuccess(const VkResult &result) {
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    };
};

SwapChainSupportDetails VulkanApplication::queryDeviceSwapChainSupportDetails(
    const VkPhysicalDevice &device) {
    SwapChainSupportDetails details;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, windowSurface, &details.capabilities);
    assertSuccess(result);
    uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface,
                                                  &formatCount, nullptr);
    assertSuccess(result);
    details.formats.resize(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, windowSurface, &formatCount, details.formats.data());
    assertSuccess(result);

    uint32_t modesCount;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface,
                                                       &modesCount, nullptr);
    assertSuccess(result);
    details.presentModes.resize(modesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, windowSurface, &modesCount, details.presentModes.data());
    assertSuccess(result);
    if (details.formats.empty() || details.presentModes.empty()) {
        throw std::runtime_error("Formats or presentModes are empty");
    };
    return details;
};

VkPresentModeKHR VulkanApplication::choosePresentMode(
    const SwapChainSupportDetails &details) {
    for (const auto &presentMode : details.presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        };
    };
    return VK_PRESENT_MODE_FIFO_KHR;
};

VkSurfaceFormatKHR VulkanApplication::chooseFormat(
    const SwapChainSupportDetails &details) {
    for (const auto &format : details.formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        };
    };
    throw std::runtime_error("Required surface format is not found");
};

VkExtent2D VulkanApplication::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities,
    const GLFWControllerWindow &window) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    };
    int width, height;
    glfwGetFramebufferSize(window.getGLFWWindow(), &width, &height);

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

void VulkanApplication::createWindowSurface(
    const GLFWControllerWindow &window) {
    VkResult result = glfwCreateWindowSurface(instance, window.getGLFWWindow(),
                                              nullptr, &windowSurface);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "glfwCreateWindowSurface");
    };
};

VulkanApplication::VulkanApplication(std::vector<const char *> extensions,
                                     const GLFWControllerWindow &window) {
    createInstance(std::move(extensions));
    createWindowSurface(window);
    pickPhysicalDevice();
    pickQueueFamilies();
    createLogicalDevice();
    createSwapChain(window);
    createImageViews();
};

void VulkanApplication::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    int index = 0;
    for (const auto &image : swapChainImages) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        VkResult result = vkCreateImageView(device, &createInfo, nullptr,
                                            &swapChainImageViews[index]);
        assertSuccess(result);
        index++;
    };
};

VulkanApplication::~VulkanApplication() {
    for (const auto& imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    };
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, windowSurface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
};
