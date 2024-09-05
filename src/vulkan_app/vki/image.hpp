#pragma once

#include <vulkan/vulkan_core.h>
#include <optional>

#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/memory.hpp"

namespace vki {
class Image {
    VkImage image;
    VkDevice device;
    std::optional<vki::Memory> memory;
protected:
    bool is_owner;
public:
    Image(vki::Image&& other);
    explicit Image(const vki::LogicalDevice &logicalDevice,
                   const VkImageCreateInfo &createInfo);
    inline const VkImage getVkImage() const { return image; };
    VkMemoryRequirements getMemoryRequirements() const;
    void bindMemory(vki::Memory&& memory);
    ~Image();
};
};  // namespace vki
