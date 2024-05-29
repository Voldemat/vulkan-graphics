#include "./buffer.hpp"

#include <vulkan/vulkan_core.h>
#include <memory>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/memory.hpp"

vki::Buffer::Buffer(const vki::LogicalDevice &logicalDevice,
                    VkBufferCreateInfo createInfo)
    : device{ logicalDevice.getVkDevice() } {
    VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &vkBuffer);
    vki::assertSuccess(result, "vkCreateBuffer");
};

VkBuffer vki::Buffer::getVkBuffer() const { return vkBuffer; };

VkMemoryRequirements vki::Buffer::getMemoryRequirements() const {
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vkBuffer, &memRequirements);
    return memRequirements;
};

void vki::Buffer::bindMemory(const std::shared_ptr<vki::Memory> &newMemory) {
    memory = newMemory;
    vkBindBufferMemory(device, vkBuffer, newMemory->getVkMemory(), 0);
};

vki::Buffer::~Buffer() { vkDestroyBuffer(device, vkBuffer, nullptr); };
