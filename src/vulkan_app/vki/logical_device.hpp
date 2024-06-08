#ifndef VKI_LOGICAL_DEVICE
#define VKI_LOGICAL_DEVICE

#include <vulkan/vulkan_core.h>
#include <cstdint>

#include "./physical_device.hpp"

namespace vki {
class LogicalDevice {
    VkDevice device;
    LogicalDevice(const LogicalDevice &other) = delete;
public:
    uint32_t graphicsQueueIndex;
    uint32_t presentQueueIndex;
    explicit LogicalDevice(const vki::PhysicalDevice &physicalDevice);
    LogicalDevice(LogicalDevice &&other);
    const VkDevice getVkDevice() const noexcept;
    void waitIdle() const;
    ~LogicalDevice();
};

};  // namespace vki

#endif
