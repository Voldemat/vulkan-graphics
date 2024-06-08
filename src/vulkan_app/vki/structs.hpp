#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <ranges>
#include <utility>
#include <vector>

#include "vulkan_app/vki/command_buffer.hpp"
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
    std::vector<const uint32_t> imageIndices;
};

struct PresentInfo {
    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<const uint32_t> imageIndices;

    explicit PresentInfo(const PresentInfoInputData &&data);
    const VkPresentInfoKHR getVkPresentInfo() const;
};

};  // namespace vki
