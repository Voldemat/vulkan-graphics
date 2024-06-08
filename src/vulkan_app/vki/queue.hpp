#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "./fence.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/structs.hpp"

namespace vki {
class Queue {
protected:
    VkQueue queue;

public:
    explicit Queue(const vki::LogicalDevice &device, const uint32_t queueIndex);
    const VkQueue getVkQueue() const;
};

class GraphicsQueue : public Queue {
    using Queue::Queue;
public:
    void submit(const std::vector<const vki::SubmitInfo>& submitInfos,
                const std::optional<const vki::Fence*> &fence) const;
};

class PresentQueue : public Queue {
    using Queue::Queue;
public:
    void present(const vki::PresentInfo& presentInfo) const;
};
};  // namespace vki
