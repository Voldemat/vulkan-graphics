#include "./surface.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <ranges>
#include <unordered_set>
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

vki::SurfaceFormatSet vki::Surface::getFormats(
    const vki::PhysicalDevice &physicalDevice) const {
    uint32_t formatCount;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice.getVkDevice(), surface, &formatCount, nullptr);
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    std::vector<VkSurfaceFormatKHR> formatsData(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getVkDevice(),
                                                  surface, &formatCount,
                                                  formatsData.data());
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    SurfaceFormatSet formats(formatsData.begin(), formatsData.end());
    return formats;
};

std::unordered_set<vki::PresentMode> vki::Surface::getPresentModes(
    const vki::PhysicalDevice &physicalDevice) const {
    uint32_t modesCount;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice.getVkDevice(), surface, &modesCount, nullptr);
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    std::vector<VkPresentModeKHR> presentModes(modesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice.getVkDevice(), surface, &modesCount,
        presentModes.data());
    vki::assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    return presentModes | std::views::transform([](const auto &v) {
               return vki::PresentMode(v);
           }) |
           std::ranges::to<std::unordered_set>();
};

vki::SurfaceDetails vki::Surface::getDetails(
    const vki::PhysicalDevice &physicalDevice) const {
    return {
        .capabilities = getCapabilities(physicalDevice),
        .formats = getFormats(physicalDevice),
        .presentModes = getPresentModes(physicalDevice),
    };
};
