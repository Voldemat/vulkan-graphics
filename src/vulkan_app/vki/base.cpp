#include "./base.hpp"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <format>
#include <optional>
#include <string>

const char *vki::c_str_or_nullptr(const std::optional<std::string> &opt) {
    return opt.transform([](const auto &c) { return c.c_str(); })
        .value_or(nullptr);
};

vki::VulkanError::VulkanError(const VkResult &result, std::string msg) {
    resultString = string_VkResult(result);
    message = msg;
    finalMessage = std::format("{}: {}", msg, resultString);
};

const char *vki::VulkanError::what() const noexcept {
    return finalMessage.data();
};

uint32_t vki::SemVer::to_vk_repr() const noexcept {
    return VK_MAKE_VERSION(major, minor, patch);
};

void vki::assertSuccess(const VkResult &result, const std::string message) {
    if (result != VK_SUCCESS) {
        throw vki::VulkanError(result, message);
    };
};


