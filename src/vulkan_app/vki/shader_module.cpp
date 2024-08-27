#include "./shader_module.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::ShaderModule::ShaderModule(const vki::LogicalDevice &logicalDevice,
                                const std::vector<char> &code)
    : device{ logicalDevice.getVkDevice() } {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
    VkResult result = vkCreateShaderModule(
        logicalDevice.getVkDevice(), &createInfo, nullptr, &vkShaderModule);
    vki::assertSuccess(result, "vkCreateShaderModule");
};

VkShaderModule vki::ShaderModule::getVkShaderModule() const {
    return vkShaderModule;
};

vki::ShaderModule::~ShaderModule() {
    vkDestroyShaderModule(device, vkShaderModule, nullptr);
};
