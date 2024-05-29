#include "./memory.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::Memory::Memory(const vki::LogicalDevice &logicalDevice,
                    VkMemoryAllocateInfo allocInfo)
    : device{ logicalDevice.getVkDevice() } {
    VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &vkMemory);
    vki::assertSuccess(result, "vkAllocateMemory");
};

VkDeviceMemory vki::Memory::getVkMemory() const { return vkMemory; };

void vki::Memory::mapMemory(VkDeviceSize size, void **buffer) const {
    VkResult result = vkMapMemory(device, vkMemory, 0, size, 0, buffer);
    vki::assertSuccess(result, "vkMapMemory");
};

void vki::Memory::unmapMemory() const { vkUnmapMemory(device, vkMemory); };

vki::Memory::~Memory() { vkFreeMemory(device, vkMemory, nullptr); };
