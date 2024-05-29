#ifndef VKI_PHYSICAL_DEVICE
#define VKI_PHYSICAL_DEVICE

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <map>
#include <vector>
#include "glfw_controller.hpp"

namespace vki {
enum class QueueFamilyType { GRAPHIC, PRESENT };

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    VkPresentModeKHR choosePresentMode() const;
    VkSurfaceFormatKHR chooseFormat() const;
    VkExtent2D chooseSwapExtent(const GLFWControllerWindow &window) const;
    uint32_t getImageCount() const;
};


class PhysicalDevice {
    VkPhysicalDevice device;
    void saveQueueFamilyIndexes(const VkSurfaceKHR &surface);
    std::vector<VkQueueFamilyProperties> getQueueFamiliesProperties() const;
    std::map<QueueFamilyType, unsigned int> queueFamilyTypeToIndex;

public:
    explicit PhysicalDevice(const VkPhysicalDevice &dev,
                            const VkSurfaceKHR &surface);
    VkPhysicalDeviceProperties getProperties() const;
    VkPhysicalDevice getVkDevice() const;
    bool isSuitable() const;
    unsigned int getFamilyTypeIndex(QueueFamilyType type) const;

    VkPhysicalDeviceMemoryProperties getMemoryProperties() const;

    VkSurfaceCapabilitiesKHR getSurfaceCapabilities(const VkSurfaceKHR &surface) const;
    std::vector<VkSurfaceFormatKHR> getSurfaceFormats(const VkSurfaceKHR& surface) const;
    std::vector<VkPresentModeKHR> getSurfacePresentModes(const VkSurfaceKHR& surface) const;
    SwapChainSupportDetails getSwapchainDetails(const VkSurfaceKHR &surface) const;
};
};  // namespace vki
#endif
