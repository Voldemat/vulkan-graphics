#include "./vulkan_application.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <format>
#include <ios>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

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

VulkanApplication::VulkanApplication(std::vector<const char *> extensions) {
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
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
    createInfo->enabledLayerCount = 0;
    createInfo->enabledLayerCount = 1;
    createInfo->ppEnabledLayerNames = validationLayers;
    createInfo->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    VkResult result = vkCreateInstance(createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("VK instance creation failed");
    };
    VkPhysicalDevice choosenDevice = VK_NULL_HANDLE;
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
    choosenDevice = devices[0];

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(choosenDevice, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(choosenDevice, &queueFamilyCount,
                                             queueFamilies.data());
    int i = 0;
    std::optional<uint32_t> graphicsFamilyIndex;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamilyIndex = i;
        }
        i++;
    }
    if (!graphicsFamilyIndex.has_value()) {
        throw std::runtime_error(
            "Suitable queue graphics family was not found");
    };
};
VulkanApplication::~VulkanApplication() {
    vkDestroyInstance(instance, nullptr);
};
