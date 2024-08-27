#include "./queue_family.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <format>
#include <set>
#include <string>
#include "magic_enum.hpp"

std::set<vki::QueueOperationType> vki::operationsFromFlags(
    const VkQueueFlags &flags) {
    std::set<vki::QueueOperationType> operations;
    if (flags & VK_QUEUE_GRAPHICS_BIT) {
        operations.insert(vki::QueueOperationType::GRAPHIC);
    } else if (flags & VK_QUEUE_COMPUTE_BIT) {
        operations.insert(vki::QueueOperationType::COMPUTE);
    } else if (flags & VK_QUEUE_TRANSFER_BIT) {
        operations.insert(vki::QueueOperationType::TRANSFER);
    } else if (flags & VK_QUEUE_PROTECTED_BIT) {
        operations.insert(vki::QueueOperationType::PROTECTED);
    } else if (flags & VK_QUEUE_SPARSE_BINDING_BIT) {
        operations.insert(vki::QueueOperationType::SPARSE_BINDING);
    } else if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
        operations.insert(vki::QueueOperationType::OPTICAL_FLOW);
    } else if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
        operations.insert(vki::QueueOperationType::VIDEO_DECODE);
    }
#ifdef VK_ENABLE_BETA_EXTENSIONS
    else if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
        operations.insert(vki::QueueOperationType::VIDEO_ENCODE);
    }
#endif
    return operations;
};

vki::QueueFamily::operator std::string() const {
    return std::format(
        "QueueFamily(index: {}, queueCount: {}, timestamp_valid_bits: {}, "
        "minImageTransferGranularity: VkExtent3D(height: {}, width: {}, "
        "height: {}), supportedOperations: {})",
        index, queueCount, timestamp_valid_bits,
        minImageTransferGranularity.height, minImageTransferGranularity.width,
        minImageTransferGranularity.height,
        operationsToString(supportedOperations));
};

bool vki::QueueFamily::doesSupportsOperations(
    const std::set<vki::QueueOperationType> &ops) const {
    return std::includes(supportedOperations.begin(), supportedOperations.end(),
                         ops.begin(), ops.end());
};

std::string vki::operationsToString(
    std::set<vki::QueueOperationType> operations) {
    std::string str = "{";
    unsigned int index = 0;
    for (const auto &op : operations) {
        str += magic_enum::enum_name(op);
        index++;
        if (index < operations.size()) {
            str += ",";
        };
    };
    str += "}";
    return str;
};
