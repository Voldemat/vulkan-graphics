#include "./physical_device.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdint>
#include <format>
#include <functional>
#include <set>
#include <string>
#include <vector>

#include "./base.hpp"
#include "magic_enum.hpp"
#include "vulkan_app/vki/queue_family.hpp"
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

bool vki::PhysicalDevice::hasQueueFamilies(
    const std::vector<std::function<bool(const QueueFamily &)>> &funcs) const {
    return std::ranges::all_of(
        funcs.begin(), funcs.end(), [this](const auto &func) -> bool {
            return std::ranges::any_of(
                queueFamilies.begin(), queueFamilies.end(),
                [&func](const auto &queueFamily) -> bool {
                    return func(queueFamily);
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
        auto supportedOperations =
            vki::operationsFromFlags(queueFamily.queueFlags);
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            device, i, surface.getVkSurfaceKHR(), &presentSupport);
        if (result != VK_SUCCESS) {
            throw VulkanError(result, "vkGetPhysicalDeviceSurfaceSupportKHR");
        };
        if (presentSupport) {
            supportedOperations.insert(vki::QueueOperationType::PRESENT);
        };
        queueFamilies.push_back((vki::QueueFamily){
            .index = i,
            .queueCount = queueFamily.queueCount,
            .timestamp_valid_bits = queueFamily.timestampValidBits,
            .minImageTransferGranularity =
                queueFamily.minImageTransferGranularity,
            .supportedOperations = supportedOperations });
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

std::vector<vki::QueueFamily> vki::PhysicalDevice::getQueueFamilies() const {
    return queueFamilies;
};

