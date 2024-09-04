#pragma once

#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class Fence {
    VkFence vkFence;
    VkDevice device;

public:
    explicit Fence(const vki::LogicalDevice &logicalDevice, const bool& initialState);
    Fence(const Fence &) = delete;
    Fence(const Fence &&) = delete;
    const VkFence getVkFence() const;
    void wait() const;
    void reset() const;
    void waitAndReset() const;
    ~Fence();
};
};  // namespace vki
