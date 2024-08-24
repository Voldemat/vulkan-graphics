#ifndef VKI_SWAPCHAIN
#define VKI_SWAPCHAIN

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "glfw_controller.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/semaphore.hpp"

namespace vki {
class LogicalDevice;
class Swapchain {
    const VkDevice device;
    VkSwapchainKHR vkSwapchain;
    void createImageViews(const vki::LogicalDevice &logicalDevice,
                          const VkFormat &format);

public:
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    explicit Swapchain(const vki::LogicalDevice &logicalDevice,
                       const vki::PhysicalDevice &physicalDevice,
                       const vki::QueueFamily &graphicsQueueFamily,
                       const vki::QueueFamily &presentQueueFamily,
                       const VkSurfaceKHR &surface,
                       const GLFWControllerWindow &window);
    const VkSwapchainKHR getVkSwapchain() const;
    uint32_t acquireNextImageKHR(const vki::Semaphore &semaphore) const;
    ~Swapchain();
};

};  // namespace vki

#endif
