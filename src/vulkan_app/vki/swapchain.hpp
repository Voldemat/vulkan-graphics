#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/surface.hpp"

namespace vki {
class LogicalDevice;

struct SwapchainCreateInfoInput {
    vki::Surface surface;
    VkExtent2D extent;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR format;
    uint32_t minImageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
    vki::QueueFamily const *graphicsQueueFamily;
    vki::QueueFamily const *presentQueueFamily;
};

struct SwapchainCreateInfo {
    vki::Surface surface;
    VkExtent2D extent;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR format;
    uint32_t minImageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
    std::optional<std::vector<unsigned int>> queueIndices;

    SwapchainCreateInfo(const vki::SwapchainCreateInfoInput &input);
    VkSwapchainCreateInfoKHR toVkCreateInfo() const;
};

class Swapchain {
    const VkDevice device;
    VkSwapchainKHR vkSwapchain;
    void createImageViews(const vki::LogicalDevice &logicalDevice,
                          const VkFormat &format);
public:
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    explicit Swapchain(const vki::LogicalDevice &logicalDevice,
                       const vki::SwapchainCreateInfo &createInfo);
    const VkSwapchainKHR getVkSwapchain() const;
    uint32_t acquireNextImageKHR(const vki::Semaphore &semaphore) const;
    ~Swapchain();
};

};  // namespace vki
