#ifndef VKI_INSTANCE
#define VKI_INSTANCE

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "./vki_base.hpp"
#include "./vki_physical_device.hpp"
#include "glfw_controller.hpp"

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
    VkSurfaceKHR surface;

public:
    VulkanInstance(const VulkanInstance &instance) = delete;
    VulkanInstance(VulkanInstanceParams params, const GLFWControllerWindow &window);
    std::vector<vki::PhysicalDevice> getPhysicalDevices() const;
    const VkInstance getInstance() const noexcept;
    const VkSurfaceKHR getSurface() const noexcept;
    ~VulkanInstance();
};
};  // namespace vki

#endif
