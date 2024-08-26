#pragma once

#include <vulkan/vulkan_core.h>

#include <optional>
#include <type_traits>
#include <vector>

#include "./fence.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/structs.hpp"

namespace vki {
template <enum QueueOperationType...>
class EmptyQueueMixin {};

class BaseQueue {
public:
    virtual const VkQueue getVkQueue() const = 0;
};

class GraphicsQueueMixin : public BaseQueue {
public:
    void submit(const std::vector<vki::SubmitInfo> &submitInfos,
                const std::optional<const vki::Fence *> &fence) const;
};

class PresentQueueMixin : public BaseQueue {
public:
    void present(const vki::PresentInfo &presentInfo) const;
};

template <enum QueueOperationType Expected, enum QueueOperationType... Args>
struct is_queue_op_type_present {
    static constexpr bool value{ ((Expected == Args) || ...) };
};

template <enum QueueOperationType... T>
class Queue
    : public std::conditional_t<
          is_queue_op_type_present<QueueOperationType::GRAPHIC, T...>::value,
          GraphicsQueueMixin, EmptyQueueMixin<T...>>,
      public std::conditional_t<
          is_queue_op_type_present<QueueOperationType::PRESENT, T...>::value,
          PresentQueueMixin, EmptyQueueMixin<T...>> {
protected:
    VkQueue queue;

public:
    const unsigned int a = 0;
    const unsigned int queueFamilyIndex;
    explicit Queue(const VkQueue &queue, const unsigned int &queueFamilyIndex)
        : queue{ queue }, queueFamilyIndex{ queueFamilyIndex } {};
    inline const VkQueue getVkQueue() const { return queue; };
};
};  // namespace vki
