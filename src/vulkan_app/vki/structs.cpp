#include "./structs.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <ranges>
#include <utility>
#include <vector>

vki::PresentInfo::PresentInfo(const PresentInfoInputData &&data)
    : waitSemaphores{ data.waitSemaphores |
                      std::views::transform([](const auto &semaphore) {
                          return semaphore->getVkSemaphore();
                      }) |
                      std::ranges::to<std::vector>() },
      swapchains{ data.swapchains | std::views::transform([](const auto &s) {
                      return s->getVkSwapchain();
                  }) |
                  std::ranges::to<std::vector>() },
      imageIndices{ std::move(data.imageIndices) } {};

const VkPresentInfoKHR vki::PresentInfo::getVkPresentInfo() const {
    return { .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
             .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
             .pWaitSemaphores = waitSemaphores.data(),
             .swapchainCount = static_cast<uint32_t>(swapchains.size()),
             .pSwapchains = swapchains.data(),
             .pImageIndices = imageIndices.data() };
};

vki::SubmitInfo::SubmitInfo(const SubmitInfoInputData &&data)
    : waitSemaphores{ data.waitSemaphores |
                      std::views::transform([](const auto &semaphore) {
                          return semaphore->getVkSemaphore();
                      }) |
                      std::ranges::to<std::vector>() },
      signalSemaphores{ data.signalSemaphores |
                        std::views::transform([](const auto &semaphore) {
                            return semaphore->getVkSemaphore();
                        }) |
                        std::ranges::to<std::vector>() },
      commandBuffers{ data.commandBuffers |
                      std::views::transform([](const auto &b) {
                          return b->getVkCommandBuffer();
                      }) |
                      std::ranges::to<std::vector>() },
      waitStages{ std::move(data.waitStages) } {};

const VkSubmitInfo vki::SubmitInfo::getVkSubmitInfo() const {
    return { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
             .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
             .pWaitSemaphores = waitSemaphores.data(),
             .pWaitDstStageMask = waitStages.data(),
             .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
             .pCommandBuffers = commandBuffers.data(),
             .signalSemaphoreCount =
                 static_cast<uint32_t>(signalSemaphores.size()),
             .pSignalSemaphores = signalSemaphores.data() };
};

