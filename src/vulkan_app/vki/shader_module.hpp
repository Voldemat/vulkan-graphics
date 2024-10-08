#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

namespace vki {
class LogicalDevice;
class ShaderModule {
    VkShaderModule vkShaderModule;
    VkDevice device;

public:
    VkShaderModule getVkShaderModule() const;
    explicit ShaderModule(const vki::LogicalDevice &logicalDevice,
                          const std::vector<char> &code);
    ~ShaderModule();
};
};  // namespace vki
