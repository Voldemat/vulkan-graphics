#ifndef SRC_VULKAN_APPLICATION
#define SRC_VULKAN_APPLICATION

#include <vulkan/vulkan_core.h>

#include <vector>

class VulkanApplication {
public:
    VkInstance instance;
    VkApplicationInfo appInfo;
    VkInstanceCreateInfo* createInfo;
    VulkanApplication(const VulkanApplication &other) = delete;
    VulkanApplication(std::vector<const char *> extensions);
    ~VulkanApplication();
};
#endif
