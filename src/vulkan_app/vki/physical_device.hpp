#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "glfw_controller.hpp"
#include "main_utils.hpp"
#include "vulkan_app/vki/surface.hpp"

namespace vki {
enum class QueueOperationType {
    PRESENT,
    GRAPHIC = VK_QUEUE_GRAPHICS_BIT,
    COMPUTE = VK_QUEUE_COMPUTE_BIT,
    TRANSFER = VK_QUEUE_TRANSFER_BIT,
    SPARSE_BINDING = VK_QUEUE_SPARSE_BINDING_BIT,
    PROTECTED = VK_QUEUE_PROTECTED_BIT,
    VIDEO_DECODE = VK_QUEUE_VIDEO_DECODE_BIT_KHR,
    OPTICAL_FLOW = VK_QUEUE_OPTICAL_FLOW_BIT_NV,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VIDEO_ENCODE = VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
#endif
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    VkPresentModeKHR choosePresentMode() const;
    VkSurfaceFormatKHR chooseFormat() const;
    VkExtent2D chooseSwapExtent(const GLFWControllerWindow &window) const;
    uint32_t getImageCount() const;
};

std::string operationsToString(std::set<QueueOperationType> operations);

struct QueueFamily {
    unsigned int index;
    unsigned int queueCount;
    uint32_t timestamp_valid_bits;
    VkExtent3D minImageTransferGranularity;
    std::set<vki::QueueOperationType> supportedOperations;

    bool doesSupportsOperations(
        const std::set<vki::QueueOperationType> &ops) const;

    PRINTABLE_DEFINITIONS(QueueFamily)
};

template <unsigned int AvailableQueueCount, enum QueueOperationType... T>
struct QueueFamilyWithOp {
    std::shared_ptr<QueueFamily> family;
    QueueFamilyWithOp(const std::shared_ptr<QueueFamily> &queueFamily)
        : family{ queueFamily } {};
};

class PhysicalDevice {
    VkPhysicalDevice device;
    void saveQueueFamilies(const vki::Surface &surface);
    std::vector<VkQueueFamilyProperties> getQueueFamiliesProperties() const;
    std::vector<std::shared_ptr<QueueFamily>> queueFamilies;

public:
    std::vector<std::shared_ptr<QueueFamily>> getQueueFamilies() const;
    std::vector<VkExtensionProperties> getExtensions() const;
    const VkPhysicalDeviceProperties properties;
    explicit PhysicalDevice(const VkPhysicalDevice &dev,
                            const vki::Surface &surface);
    VkPhysicalDeviceProperties getProperties() const;
    VkPhysicalDevice getVkDevice() const;
    bool hasQueueFamilies(
        const std::vector<std::function<bool(const QueueFamily &)>> &funcs)
        const;
    bool hasExtensions(
        const std::vector<std::function<bool(const VkExtensionProperties &)>>
            &funcs) const;

    VkPhysicalDeviceMemoryProperties getMemoryProperties() const;

    VkSurfaceCapabilitiesKHR getSurfaceCapabilities(
        const vki::Surface &surface) const;
    std::vector<VkSurfaceFormatKHR> getSurfaceFormats(
        const vki::Surface &surface) const;
    std::vector<VkPresentModeKHR> getSurfacePresentModes(
        const vki::Surface &surface) const;
    SwapChainSupportDetails getSwapchainDetails(
        const vki::Surface &surface) const;

    PRINTABLE_DEFINITIONS(PhysicalDevice)
};
};  // namespace vki
