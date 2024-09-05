#pragma once

#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;
class ImageView {
    VkImageView imageView;
    VkDevice device;

protected:
    bool is_owner;

public:
    ImageView(vki::ImageView &&other);
    explicit ImageView(const vki::LogicalDevice &logicalDevice,
                       const VkImageViewCreateInfo &createInfo);
    inline const VkImageView getVkImageView() const { return imageView; };
    ~ImageView();
};
};  // namespace vki
