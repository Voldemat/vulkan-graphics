#include "./buffer.hpp"

#include <vulkan/vulkan_core.h>

#include <optional>
#include <utility>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/memory.hpp"

vki::Buffer::Buffer(const vki::Buffer &other)
    : device{ other.device }, vkBuffer{ other.vkBuffer }, is_owner{ false } {
};

vki::Buffer::Buffer(vki::Buffer &&other)
    : device{ other.device },
      vkBuffer{ other.vkBuffer },
      is_owner{ other.is_owner },
      memory{ std::move(other.memory) } {
    other.is_owner = false;
};

vki::Buffer::Buffer(const vki::LogicalDevice &logicalDevice,
                    VkBufferCreateInfo createInfo)
    : device{ logicalDevice.getVkDevice() }, is_owner{ true } {
    VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &vkBuffer);
    vki::assertSuccess(result, "vkCreateBuffer");
};

VkBuffer vki::Buffer::getVkBuffer() const { return vkBuffer; };

VkMemoryRequirements vki::Buffer::getMemoryRequirements() const {
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vkBuffer, &memRequirements);
    return memRequirements;
};

void vki::Buffer::bindMemory(vki::Memory &&newMemory) {
    memory.emplace(newMemory);
    vkBindBufferMemory(device, vkBuffer, newMemory.getVkMemory(), 0);
};

vki::Buffer::~Buffer() {
    if (is_owner) {
        vkDestroyBuffer(device, vkBuffer, nullptr);
    };
};
