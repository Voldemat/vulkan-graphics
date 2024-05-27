#ifndef VKI_PIPELINE_LAYOUT
#define VKI_PIPELINE_LAYOUT

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/logical_device.hpp"
namespace vki {
class PipelineLayout {
    VkPipelineLayout vkPipelineLayout;
    VkDevice device;
public:
    VkPipelineLayout getVkPipelineLayout() const;
    explicit PipelineLayout(const vki::LogicalDevice &logicalDevice);
    ~PipelineLayout();
};
};  // namespace vki
#endif
