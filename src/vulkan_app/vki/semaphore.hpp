#ifndef VKI_SEMAPHORE
#define VKI_SEMAPHORE
#include <vulkan/vulkan_core.h>
#include "vulkan_app/vki/logical_device.hpp"
namespace vki {
class Semaphore {
    VkSemaphore vkSemaphore;
    VkDevice device;
public:
    explicit Semaphore(const vki::LogicalDevice& logicalDevice);
    const VkSemaphore getVkSemaphore() const;
    ~Semaphore();
};
};
#endif
