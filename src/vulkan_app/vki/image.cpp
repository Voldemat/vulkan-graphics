#include "./image.hpp"

#include <vulkan/vulkan_core.h>

#include <utility>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/memory.hpp"

vki::Image::Image(vki::Image &&other)
    : is_owner{ other.is_owner },
      device{ other.device },
      image{ other.image },
      memory{ std::move(other.memory) } {
    other.is_owner = false;
};

vki::Image::Image(const vki::LogicalDevice &logicalDevice,
                  const VkImageCreateInfo &createInfo)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    VkResult result = vkCreateImage(device, &createInfo, nullptr, &image);
    vki::assertSuccess(result, "vkCreateImage");
};

VkMemoryRequirements vki::Image::getMemoryRequirements() const {
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, image, &requirements);
    return requirements;
};

void vki::Image::bindMemory(vki::Memory &&newMemory) {
    VkResult result =
        vkBindImageMemory(device, image, newMemory.getVkMemory(), 0);
    vki::assertSuccess(result, "vkBindImageMemory");
    memory.emplace(newMemory);
};

vki::Image::~Image() {
    if (is_owner) {
        vkDestroyImage(device, image, nullptr);
    };
};
