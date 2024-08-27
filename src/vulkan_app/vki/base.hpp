#pragma once

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <exception>
#include <optional>
#include <string>

namespace vki {

class VulkanError : public std::exception {
    std::string resultString;
    std::string message;
    std::string finalMessage;

public:
    explicit VulkanError(const VkResult &result, std::string msg);
    const char *what() const noexcept;
};

struct SemVer {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;

    uint32_t to_vk_repr() const noexcept;
};

const char *c_str_or_nullptr(const std::optional<std::string> &opt);

void assertSuccess(const VkResult &result, const std::string message);
};  // namespace vki
