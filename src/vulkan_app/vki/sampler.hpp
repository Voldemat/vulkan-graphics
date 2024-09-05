#pragma once

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/logical_device.hpp"
namespace vki {
class Sampler {
    VkSampler sampler;
    VkDevice device;

protected:
    bool is_owner;

public:
    Sampler(vki::Sampler&& other);
    explicit Sampler(const vki::LogicalDevice &logicalDevice,
                     const VkSamplerCreateInfo &createInfo);
    inline const VkSampler getVkSampler() const { return sampler; };
    ~Sampler();
};
};  // namespace vki
