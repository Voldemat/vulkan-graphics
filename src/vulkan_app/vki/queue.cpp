#include "./queue.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "./base.hpp"
#include "./fence.hpp"
#include "vulkan_app/vki/logical_device.hpp"

vki::Queue::Queue(const vki::LogicalDevice &logicalDevice,
                  const uint32_t queueIndex)
     {
    vkGetDeviceQueue(logicalDevice.getVkDevice(), queueIndex, 0, &queue);
};

const VkQueue vki::Queue::getVkQueue() const { return queue; };

void vki::GraphicsQueue::submit(const std::vector<VkSubmitInfo>& submitInfos,
                                const std::optional<const vki::Fence*> &fence) const {
    VkResult result = vkQueueSubmit(
        queue, submitInfos.size(), submitInfos.data(),
        fence.transform([](const auto &f) { return f->getVkFence(); })
            .value_or(nullptr));
    vki::assertSuccess(result, "vkQueueSubmit");
};


void vki::PresentQueue::present(const VkPresentInfoKHR& presentInfo) const {
    VkResult result = vkQueuePresentKHR(queue, &presentInfo);
    vki::assertSuccess(result, "vkQueuePresentKHR");
};
