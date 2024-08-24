#include "./queue.hpp"

#include <vulkan/vulkan_core.h>

#include <optional>
#include <ranges>
#include <vector>

#include "./base.hpp"
#include "./fence.hpp"
#include "vulkan_app/vki/structs.hpp"

void vki::GraphicsQueueMixin::submit(
    const std::vector<vki::SubmitInfo> &infoArray,
    const std::optional<const vki::Fence *> &fence) const {
    const auto &finalInfo = infoArray |
                            std::views::transform([](const auto &info) {
                                return info.getVkSubmitInfo();
                            }) |
                            std::ranges::to<std::vector>();
    VkResult result = vkQueueSubmit(
        getVkQueue(), finalInfo.size(), finalInfo.data(),
        fence.transform([](const auto &f) { return f->getVkFence(); })
            .value_or(nullptr));
    vki::assertSuccess(result, "vkQueueSubmit");
};

void vki::PresentQueueMixin::present(
    const vki::PresentInfo &presentInfo) const {
    const auto &finalInfo = presentInfo.getVkPresentInfo();
    VkResult result = vkQueuePresentKHR(getVkQueue(), &finalInfo);
    vki::assertSuccess(result, "vkQueuePresentKHR");
};
