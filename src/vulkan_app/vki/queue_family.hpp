#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <set>
#include <string>

#include "main_utils.hpp"

namespace vki {
enum class QueueOperationType {
    PRESENT,
    GRAPHIC = VK_QUEUE_GRAPHICS_BIT,
    COMPUTE = VK_QUEUE_COMPUTE_BIT,
    TRANSFER = VK_QUEUE_TRANSFER_BIT,
    SPARSE_BINDING = VK_QUEUE_SPARSE_BINDING_BIT,
    PROTECTED = VK_QUEUE_PROTECTED_BIT,
    VIDEO_DECODE = VK_QUEUE_VIDEO_DECODE_BIT_KHR,
    OPTICAL_FLOW = VK_QUEUE_OPTICAL_FLOW_BIT_NV,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VIDEO_ENCODE = VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
#endif
};
std::string operationsToString(std::set<QueueOperationType> operations);
std::set<vki::QueueOperationType> operationsFromFlags(
    const VkQueueFlags &flags);

struct QueueFamily {
    unsigned int index;
    unsigned int queueCount;
    uint32_t timestamp_valid_bits;
    VkExtent3D minImageTransferGranularity;
    std::set<vki::QueueOperationType> supportedOperations;

    bool doesSupportsOperations(
        const std::set<vki::QueueOperationType> &ops) const;

    PRINTABLE_DEFINITIONS(QueueFamily)
};

template <unsigned int AvailableQueueCount, enum QueueOperationType... T>
struct QueueFamilyWithOp {
    const QueueFamily family;
    QueueFamilyWithOp(const QueueFamily queueFamily)
        : family{ queueFamily } {};
};
};  // namespace vki
