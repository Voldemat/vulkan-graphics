#ifndef VKI_SEMAPHORE
#define VKI_SEMAPHORE
#include <vulkan/vulkan_core.h>
namespace vki {
class LogicalDevice;
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
