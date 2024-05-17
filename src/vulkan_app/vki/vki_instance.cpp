#include "./vki_instance.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <ranges>
#include <vector>

#include "./vki_base.hpp"
#include "./vki_physical_device.hpp"
#include "GLFW/glfw3.h"
#include "glfw_controller.hpp"

std::vector<const char *> vki::VulkanInstanceParams::getLayers()
    const noexcept {
    return layers |
           std::views::transform([](const auto &l) { return l.c_str(); }) |
           std::ranges::to<std::vector>();
};

std::vector<const char *> vki::VulkanInstanceParams::getExtensions()
    const noexcept {
    return extensions |
           std::views::transform([](const auto &e) { return e.c_str(); }) |
           std::ranges::to<std::vector>();
};

vki::VulkanInstance::VulkanInstance(VulkanInstanceParams params,
                                    const GLFWControllerWindow &window) {
    params.extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    params.extensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = c_str_or_nullptr(params.appName),
        .applicationVersion = params.appVersion.to_vk_repr(),
        .engineVersion = params.engineVersion.to_vk_repr(),
        .apiVersion = params.apiVersion,
    };

    const auto &layers = params.getLayers();
    const auto &extensions = params.getExtensions();
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = 0 | VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "vkCreateInstance");
    };

    result = glfwCreateWindowSurface(instance, window.getGLFWWindow(), nullptr,
                                     &surface);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "glfwCreateWindowSurface");
    };
};

const VkInstance vki::VulkanInstance::getInstance() const noexcept {
    return instance;
};

const VkSurfaceKHR vki::VulkanInstance::getSurface() const noexcept {
    return surface;
};

vki::VulkanInstance::~VulkanInstance() {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
};

std::vector<vki::PhysicalDevice> vki::VulkanInstance::getPhysicalDevices()
    const {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devicesArray(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devicesArray.data());
    return devicesArray |
           std::views::transform(
               [this](const VkPhysicalDevice &device) -> vki::PhysicalDevice {
                   return vki::PhysicalDevice(device, surface);
               }) |
           std::ranges::to<std::vector>();
};
