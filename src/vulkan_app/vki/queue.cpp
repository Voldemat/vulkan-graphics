#include "./queue.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <ranges>
#include <vector>

#include "./base.hpp"
#include "./fence.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/structs.hpp"

vki::Queue::Queue(const vki::LogicalDevice &logicalDevice,
                  const uint32_t queueIndex) {
    vkGetDeviceQueue(logicalDevice.getVkDevice(), queueIndex, 0, &queue);
};

const VkQueue vki::Queue::getVkQueue() const { return queue; };

void vki::GraphicsQueue::submit(
    const std::vector<const vki::SubmitInfo> &infoArray,
    const std::optional<const vki::Fence *> &fence) const {
    const auto &finalInfo = infoArray |
                            std::views::transform([](const auto &info) {
                                return info.getVkSubmitInfo();
                            }) |
                            std::ranges::to<std::vector>();
    VkResult result = vkQueueSubmit(
        queue, finalInfo.size(), finalInfo.data(),
        fence.transform([](const auto &f) { return f->getVkFence(); })
            .value_or(nullptr));
    vki::assertSuccess(result, "vkQueueSubmit");
};

void vki::PresentQueue::present(const vki::PresentInfo &presentInfo) const {
    const auto& finalInfo = presentInfo.getVkPresentInfo();
    VkResult result = vkQueuePresentKHR(queue, &finalInfo);
    vki::assertSuccess(result, "vkQueuePresentKHR");
};
