#include "./sampler.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::Sampler::Sampler(vki::Sampler &&other)
    : device{ other.device },
      sampler{ other.sampler },
      is_owner{ other.is_owner } {
    other.is_owner = false;
};

vki::Sampler::Sampler(const vki::LogicalDevice &logicalDevice,
                      const VkSamplerCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    VkResult result = vkCreateSampler(device, &createInfo, nullptr, &sampler);
    vki::assertSuccess(result, "vkCreateSampler");
};

vki::Sampler::~Sampler() {
    if (is_owner) {
        vkDestroySampler(device, sampler, nullptr);
    };
};
