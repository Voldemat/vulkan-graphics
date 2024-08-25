#pragma once

#include <vulkan/vulkan_core.h>

#include <array>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/swapchain.hpp"
namespace vki {

struct SubmitInfoInputData {
    std::vector<const vki::Semaphore *> waitSemaphores;
    std::vector<const vki::Semaphore *> signalSemaphores;
    std::vector<const vki::CommandBuffer *> commandBuffers;
    std::vector<VkPipelineStageFlags> waitStages;
};

struct SubmitInfo {
    std::vector<VkPipelineStageFlags> waitStages;
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSemaphore> signalSemaphores;
    std::vector<VkCommandBuffer> commandBuffers;

    explicit SubmitInfo(const SubmitInfoInputData &&data);
    const VkSubmitInfo getVkSubmitInfo() const;
};

struct PresentInfoInputData {
    std::vector<const vki::Semaphore *> waitSemaphores;
    std::vector<const vki::Swapchain *> swapchains;
    std::vector<uint32_t> imageIndices;
};

struct PresentInfo {
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<uint32_t> imageIndices;

    explicit PresentInfo(const PresentInfoInputData &&data);
    const VkPresentInfoKHR getVkPresentInfo() const;
};

enum class QueueFlag { PROTECTED = VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT };

template <unsigned int QueueCount = 1, enum QueueOperationType... T>
struct QueueCreateInfo {
    static_assert(QueueCount >= 1, "QueueCount must be gte 1");
    QueueFamilyWithOp<T...> queueFamily;
    unsigned int queueCount = QueueCount;
    std::array<float, QueueCount> queuePriorities = { 1.0f };
    std::unordered_set<QueueFlag> flags;
    const VkDeviceQueueCreateInfo getVkCreateInfo() const {
        VkDeviceQueueCreateFlags createFlags = 0;
        for (const auto &f : flags) {
            switch (f) {
                case vki::QueueFlag::PROTECTED:
                    createFlags |= VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT;
                    break;
            };
        };
        return (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .flags = createFlags,
            .queueFamilyIndex = queueFamily.family->index,
            .queueCount = queueCount,
            .pQueuePriorities = queuePriorities.data()
        };
    };
};

};  // namespace vki
