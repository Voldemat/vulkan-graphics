#ifndef VKI_SHADER_MODULE
#define VKI_SHADER_MODULE

#include <vulkan/vulkan_core.h>
#include <vector>
#include "vulkan_app/vki/logical_device.hpp"
namespace vki {
class ShaderModule {
    VkShaderModule vkShaderModule;
    VkDevice device;
public:
    VkShaderModule getVkShaderModule() const;
    explicit ShaderModule(const vki::LogicalDevice& logicalDevice, const std::vector<char>& code);
    ~ShaderModule();
};
};
#endif
