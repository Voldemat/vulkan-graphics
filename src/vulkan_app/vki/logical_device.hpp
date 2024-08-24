#ifndef VKI_LOGICAL_DEVICE
#define VKI_LOGICAL_DEVICE

#include <vulkan/vulkan_core.h>

#include <cstdint>

#include "./physical_device.hpp"
#include "vulkan_app/vki/queue.hpp"

namespace vki {
class LogicalDevice {
    VkDevice device;
    LogicalDevice(const LogicalDevice &other) = delete;

public:
    uint32_t graphicsQueueIndex;
    uint32_t presentQueueIndex;
    explicit LogicalDevice(const vki::PhysicalDevice &physicalDevice,
                           const vki::QueueFamily &graphicsQueueFamily,
                           const vki::QueueFamily &presentQueueFamily);
    LogicalDevice(LogicalDevice &&other);
    template <enum vki::QueueOperationType T>
    vki::Queue<T> getQueue(const vki::QueueFamilyWithOp<T> &queueFamily) const;
    vki::Queue<vki::QueueOperationType::PRESENT> getQueue(
        const vki::QueueFamilyWithOp<vki::QueueOperationType::PRESENT>
            &presentQueueFamily) const;
    vki::Queue<vki::QueueOperationType::GRAPHIC> getQueue(
        const vki::QueueFamilyWithOp<vki::QueueOperationType::GRAPHIC>
            &graphicQueueFamily) const;
    const VkDevice getVkDevice() const noexcept;
    void waitIdle() const;
    ~LogicalDevice();
};

};  // namespace vki

#endif
