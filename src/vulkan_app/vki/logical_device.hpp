#ifndef VKI_LOGICAL_DEVICE
#define VKI_LOGICAL_DEVICE

#include <vulkan/vulkan_core.h>

#include <tuple>
#include <vector>

#include "./physical_device.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/structs.hpp"

namespace vki {
class LogicalDevice {
    VkDevice device;
    LogicalDevice(const LogicalDevice &other) = delete;
    void init(const vki::PhysicalDevice &physicalDevice,
              const std::vector<VkDeviceQueueCreateInfo> &queueCreateInfoArray);

public:
    template <typename... T>
    explicit LogicalDevice(const vki::PhysicalDevice &physicalDevice,
                           const std::tuple<T...> &queueInfoArray) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfoArray;
        std::apply(
            [&queueCreateInfoArray](auto &&...args) {
                ((queueCreateInfoArray.push_back(args.getVkCreateInfo()), ...));
            },
            queueInfoArray);
        init(physicalDevice, queueCreateInfoArray);
    };
    LogicalDevice(LogicalDevice &&other);
    template <unsigned int QueueIndex, enum vki::QueueOperationType... T>
    vki::Queue<T...> getQueue(
        const vki::QueueCreateInfo<QueueIndex + 1, T...> &createInfo) const {
        unsigned int queueFamilyIndex = createInfo.queueFamily.family->index;
        VkQueue queue;
        vkGetDeviceQueue(device, queueFamilyIndex, QueueIndex, &queue);
        return vki::Queue<T...>(queue, queueFamilyIndex);
    };
    const VkDevice getVkDevice() const noexcept;
    void waitIdle() const;
    ~LogicalDevice();
};
};  // namespace vki

#endif
