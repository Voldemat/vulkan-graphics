#include "./logical_device.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/physical_device.hpp"

void vki::LogicalDevice::init(
    const vki::PhysicalDevice &physicalDevice,
    const VkPhysicalDeviceFeatures& features,
    const std::vector<VkDeviceQueueCreateInfo> &queueCreateInfoArray) {
    std::vector<const char *> deviceExtensions;
    deviceExtensions.push_back("VK_KHR_portability_subset");
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfoArray.size();
    createInfo.pQueueCreateInfos = queueCreateInfoArray.data();
    createInfo.pEnabledFeatures = &features;
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

void vki::LogicalDevice::waitIdle() const { vkDeviceWaitIdle(device); };

std::vector<VkDescriptorSet> vki::LogicalDevice::allocateDescriptorSets(
    const VkDescriptorSetAllocateInfo &allocInfo) const {
    std::vector<VkDescriptorSet> descriptorSets(allocInfo.descriptorSetCount);
    VkResult result =
        vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
    vki::assertSuccess(result, "vkAllocateDescriptorSets");
    return descriptorSets;
};

void vki::LogicalDevice::updateWriteDescriptorSets(
    const std::vector<VkWriteDescriptorSet> &writeInfos) const {
    vkUpdateDescriptorSets(device, writeInfos.size(), writeInfos.data(), 0,
                           nullptr);
};
