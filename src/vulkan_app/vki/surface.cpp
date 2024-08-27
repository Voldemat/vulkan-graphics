#include "./surface.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "GLFW/glfw3.h"
#include "glfw_controller.hpp"
#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/physical_device.hpp"

vki::Surface::Surface(const Surface &other)
    : is_owner{ false },
      instance{ other.instance },
      surface{ other.surface } {};

vki::Surface::Surface(const vki::VulkanInstance &vulkanInstance,
                      const GLFWControllerWindow &window)
    : instance{ vulkanInstance.getInstance() }, is_owner{ true } {
    VkResult result = glfwCreateWindowSurface(instance, window.getGLFWWindow(),
                                              nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "glfwCreateWindowSurface");
    };
};

VkSurfaceKHR vki::Surface::getVkSurfaceKHR() const { return surface; };

vki::Surface::~Surface() {
    if (is_owner) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    };
};

VkSurfaceCapabilitiesKHR vki::Surface::getCapabilities(
    const vki::PhysicalDevice &physicalDevice) const {
    VkSurfaceCapabilitiesKHR details;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice.getVkDevice(), surface, &details);
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    return details;
};

std::vector<VkSurfaceFormatKHR> vki::Surface::getFormats(
    const vki::PhysicalDevice &physicalDevice) const {
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t formatCount;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice.getVkDevice(), surface, &formatCount, nullptr);
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    formats.resize(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice.getVkDevice(), surface, &formatCount, formats.data());
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    return formats;
};

std::vector<VkPresentModeKHR> vki::Surface::getPresentModes(
    const vki::PhysicalDevice &physicalDevice) const {
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t modesCount;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice.getVkDevice(), surface, &modesCount, nullptr);
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    presentModes.resize(modesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice.getVkDevice(), surface, &modesCount,
        presentModes.data());
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    return presentModes;
};

vki::SurfaceDetails vki::Surface::getDetails(
    const vki::PhysicalDevice &physicalDevice) const {
    return {
        .capabilities = getCapabilities(physicalDevice),
        .formats = getFormats(physicalDevice),
        .presentModes = getPresentModes(physicalDevice),
    };
};
