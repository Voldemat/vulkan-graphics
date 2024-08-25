#pragma once

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "./base.hpp"
#include "./physical_device.hpp"
#include "vulkan_app/vki/surface.hpp"

namespace vki {

struct VulkanInstanceParams {
    std::vector<std::string> extensions;
    std::optional<std::string> appName;
    SemVer appVersion;
    SemVer engineVersion;
    uint32_t apiVersion = VK_API_VERSION_1_3;
    std::vector<std::string> layers;

    std::vector<const char *> getLayers() const noexcept;
    std::vector<const char *> getExtensions() const noexcept;
};

class VulkanInstance {
    VkInstance instance;

public:
    VulkanInstance(const VulkanInstance &instance) = delete;
    VulkanInstance(VulkanInstanceParams params);
    std::vector<vki::PhysicalDevice> getPhysicalDevices(
        const vki::Surface &surface) const;
    const VkInstance getInstance() const noexcept;
    ~VulkanInstance();
};
};  // namespace vki
