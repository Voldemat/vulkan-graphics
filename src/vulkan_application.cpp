#include "./vulkan_application.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

VulkanApplication::VulkanApplication(std::vector<const char *> extensions) {
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    createInfo = new VkInstanceCreateInfo();
    createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo->pApplicationInfo = &appInfo;
    createInfo->enabledExtensionCount = (uint32_t)extensions.size();
    createInfo->ppEnabledExtensionNames = extensions.data();
    createInfo->enabledLayerCount = 0;
    createInfo->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    VkResult result = vkCreateInstance(createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("VK instance creation failed");
    };
};
VulkanApplication::~VulkanApplication() {
    vkDestroyInstance(instance, nullptr);
};
