#ifndef VKI_SWAPCHAIN
#define VKI_SWAPCHAIN

#include <vulkan/vulkan_core.h>
#include <vector>
#include "glfw_controller.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"

namespace vki {

class Swapchain {
    const VkDevice device;
    VkSwapchainKHR vkSwapchain;
    void createImageViews(const vki::LogicalDevice& logicalDevice, const VkFormat& format);
public:
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    explicit Swapchain(
        const vki::LogicalDevice& logicalDevice,
        const vki::PhysicalDevice& physicalDevice,
        const VkSurfaceKHR& surface,
        const GLFWControllerWindow& window
    );
    const VkSwapchainKHR getVkSwapchain() const;
    ~Swapchain();
};

};

#endif
