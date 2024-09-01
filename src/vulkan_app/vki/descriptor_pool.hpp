#pragma once

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/logical_device.hpp"
namespace vki {
class DescriptorPool {
    VkDescriptorPool vkDescriptorPool;
    VkDevice device;

protected:
    bool is_owner;

public:
    DescriptorPool(const DescriptorPool &other);
    explicit DescriptorPool(const vki::LogicalDevice &logicalDevice,
                            const VkDescriptorPoolCreateInfo &createInfo);
    inline const VkDescriptorPool getVkDescriptorPool() const {
        return vkDescriptorPool;
    };
    ~DescriptorPool();
};
};  // namespace vki
