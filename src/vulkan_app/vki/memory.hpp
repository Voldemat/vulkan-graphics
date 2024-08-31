#pragma once
#include <vulkan/vulkan_core.h>

namespace vki {
class LogicalDevice;

class Borrowable {
protected:
    bool is_owner;
    Borrowable(): is_owner{true} {};
    Borrowable(const Borrowable& other): is_owner{false} {};
    Borrowable(Borrowable& other): is_owner{other.is_owner} {
        other.is_owner = false;
    };
    Borrowable(Borrowable&& other): is_owner{other.is_owner} {
        other.is_owner = false;
    };
};

class Memory : Borrowable {
    VkDeviceMemory vkMemory;
    VkDevice device;
public:
    Memory(vki::Memory&& other);
    Memory(vki::Memory& other);
    explicit Memory(const vki::LogicalDevice &device,
                    VkMemoryAllocateInfo allocInfo);
    VkDeviceMemory getVkMemory() const;
    void mapMemory(VkDeviceSize size, void **buffer) const;
    void unmapMemory() const;
    void write(const VkDeviceSize& size, void* data) const;
    ~Memory();
};
};  // namespace vki
