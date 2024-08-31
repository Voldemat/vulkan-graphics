#pragma once

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/logical_device.hpp"
namespace vki {
class DescriptorSetLayout {
    VkDevice device;
    VkDescriptorSetLayout vkDescriptorSetLayout;

public:
    DescriptorSetLayout(const vki::LogicalDevice &logicalDevice,
                        const VkDescriptorSetLayoutCreateInfo &createInfo);
    inline const VkDescriptorSetLayout getVkDescriptorSetLayout() const {
        return vkDescriptorSetLayout;
    };
    ~DescriptorSetLayout();
};
};  // namespace vki
