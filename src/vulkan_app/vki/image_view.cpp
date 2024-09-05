#include "./image_view.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::ImageView::ImageView(vki::ImageView &&other)
    : device{ other.device },
      imageView{ other.imageView },
      is_owner{ other.is_owner } {
    other.is_owner = false;
};

vki::ImageView::ImageView(const vki::LogicalDevice &logicalDevice,
                          const VkImageViewCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    VkResult result =
        vkCreateImageView(device, &createInfo, nullptr, &imageView);
    vki::assertSuccess(result, "vkCreateImageView");
};

vki::ImageView::~ImageView() {
    if (is_owner) {
        vkDestroyImageView(device, imageView, nullptr);
    };
};
