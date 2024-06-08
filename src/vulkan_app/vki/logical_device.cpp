#include "./logical_device.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/physical_device.hpp"

vki::LogicalDevice::LogicalDevice(const vki::PhysicalDevice &physicalDevice) {
    float queuePriority = 1.0f;
    graphicsQueueIndex =
        physicalDevice.getFamilyTypeIndex(vki::QueueFamilyType::GRAPHIC);
    presentQueueIndex =
        physicalDevice.getFamilyTypeIndex(vki::QueueFamilyType::PRESENT);

    VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
    graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    graphicsQueueCreateInfo.queueCount = 1;
    graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfoArray;
    queueCreateInfoArray.push_back(graphicsQueueCreateInfo);
    if (presentQueueIndex != graphicsQueueIndex) {
        VkDeviceQueueCreateInfo presentQueueCreateInfo{};
        presentQueueCreateInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentQueueCreateInfo.queueFamilyIndex = presentQueueIndex;
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

    VkResult result = vkCreateDevice(physicalDevice.getVkDevice(), &createInfo,
                                     nullptr, &device);

    if (result != VK_SUCCESS) {
        throw VulkanError(result, "vkCreateDevice");
    };
};

vki::LogicalDevice::~LogicalDevice() { vkDestroyDevice(device, nullptr); };

vki::LogicalDevice::LogicalDevice(vki::LogicalDevice &&other) {
    device = other.device;
};

const VkDevice vki::LogicalDevice::getVkDevice() const noexcept {
    return device;
};


void vki::LogicalDevice::waitIdle() const {
    vkDeviceWaitIdle(device);
};
