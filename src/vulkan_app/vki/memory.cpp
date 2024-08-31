#include "./memory.hpp"

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstring>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::Memory::Memory(vki::Memory &other)
    : vki::Borrowable(other),
      device{ other.device },
      vkMemory{ other.vkMemory } {};

vki::Memory::Memory(const vki::Memory &other)
    : vki::Borrowable(other),
      device{ other.device },
      vkMemory{ other.vkMemory } {};

vki::Memory::Memory(vki::Memory &&other)
    : vki::Borrowable(other),
      device{ other.device },
      vkMemory{ other.vkMemory } {};

vki::Memory::Memory(const vki::LogicalDevice &logicalDevice,
                    VkMemoryAllocateInfo allocInfo)
    : vki::Borrowable(), device{ logicalDevice.getVkDevice() } {
    VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &vkMemory);
    vki::assertSuccess(result, "vkAllocateMemory");
};

VkDeviceMemory vki::Memory::getVkMemory() const { return vkMemory; };

void vki::Memory::mapMemory(VkDeviceSize size, void **buffer) const {
    VkResult result = vkMapMemory(device, vkMemory, 0, size, 0, buffer);
    vki::assertSuccess(result, "vkMapMemory");
};

void vki::Memory::unmapMemory() const { vkUnmapMemory(device, vkMemory); };

void vki::Memory::write(const VkDeviceSize &size, void *data) const {
    void *mappedMemory;
    mapMemory(size, &mappedMemory);
    memcpy(mappedMemory, data, static_cast<std::size_t>(size));
    unmapMemory();
};

vki::Memory::~Memory() {
    if (is_owner) {
        vkFreeMemory(device, vkMemory, nullptr);
    };
};
