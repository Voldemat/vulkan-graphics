#ifndef VKI_FENCE
#define VKI_FENCE
#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class Fence {
    VkFence vkFence;
    VkDevice device;
public:
    explicit Fence(const vki::LogicalDevice& logicalDevice);
    Fence(const Fence&) = delete;
    Fence(const Fence&&) = delete;
    const VkFence getVkFence() const;
    void wait() const;
    ~Fence();
};
};
#endif
