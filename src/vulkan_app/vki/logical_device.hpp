#pragma once

#include <vulkan/vulkan_core.h>

#include <tuple>
#include <vector>

#include "./physical_device.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/queue_family.hpp"
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

    template <unsigned int QueueIndex, unsigned int QueueCount,
              unsigned int AvailableQueueCount,
              enum vki::QueueOperationType... T>
    vki::Queue<T...> getQueue(
        const vki::QueueCreateInfo<QueueCount, AvailableQueueCount, T...>
            &createInfo) const

    {
        static_assert(QueueIndex < QueueCount,
                      "QueueIndex must be less than QueueCount");
        unsigned int queueFamilyIndex = createInfo.queueFamily.family->index;
        VkQueue queue;
        vkGetDeviceQueue(device, queueFamilyIndex, QueueIndex, &queue);
        return vki::Queue<T...>(queue, queueFamilyIndex);
    };

    LogicalDevice(LogicalDevice &&other);
    const VkDevice getVkDevice() const noexcept;
    void waitIdle() const;
    std::vector<VkDescriptorSet> allocateDescriptorSets(
        const VkDescriptorSetAllocateInfo &allocInfo) const;
    void updateWriteDescriptorSets(
        const std::vector<VkWriteDescriptorSet> &writeInfos) const;
    ~LogicalDevice();
};

};  // namespace vki
