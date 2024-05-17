#ifndef VKI_PHYSICAL_DEVICE
#define VKI_PHYSICAL_DEVICE

#include <vulkan/vulkan_core.h>

#include <map>
#include <vector>

namespace vki {
enum class QueueFamilyType { GRAPHIC, PRESENT };

class PhysicalDevice {
    VkPhysicalDevice device;
    void saveQueueFamilyIndexes(const VkSurfaceKHR &surface);
    std::vector<VkQueueFamilyProperties> getQueueFamiliesProperties() const;
    std::map<QueueFamilyType, unsigned int> queueFamilyTypeToIndex;

public:
    explicit PhysicalDevice(const VkPhysicalDevice &dev,
                            const VkSurfaceKHR &surface);
    VkPhysicalDeviceProperties getProperties() const;
    VkPhysicalDevice getVkDevice() const;
    bool isSuitable() const;
    unsigned int getFamilyTypeIndex(QueueFamilyType type) const;
};
};  // namespace vki
#endif
