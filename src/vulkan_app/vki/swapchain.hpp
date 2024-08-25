#ifndef VKI_SWAPCHAIN
#define VKI_SWAPCHAIN

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "glfw_controller.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/surface.hpp"

namespace vki {
class LogicalDevice;
class Swapchain {
    const VkDevice device;
    VkSwapchainKHR vkSwapchain;
    VkFormat format;
    VkExtent2D extent;
    void createImageViews(const vki::LogicalDevice &logicalDevice,
                          const VkFormat &format);

public:
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    const VkFormat getFormat() const;
    const VkExtent2D getExtent() const;
    explicit Swapchain(const vki::LogicalDevice &logicalDevice,
                       const vki::PhysicalDevice &physicalDevice,
                       const vki::QueueFamily &graphicsQueueFamily,
                       const vki::QueueFamily &presentQueueFamily,
                       const vki::Surface &surface,
                       const GLFWControllerWindow &window);
    const VkSwapchainKHR getVkSwapchain() const;
    uint32_t acquireNextImageKHR(const vki::Semaphore &semaphore) const;
    ~Swapchain();
};

};  // namespace vki

#endif
