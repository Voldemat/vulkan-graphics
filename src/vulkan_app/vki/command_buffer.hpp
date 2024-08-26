#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/render_pass.hpp"

namespace vki {
enum class CommandBufferUsage {
    ONE_TIME_SUBMIT = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    RENDER_PASS_CONTINUE = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
    SIMULTANEOUS_USE = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
};

enum class SubpassContentsType {
    INLINE = VK_SUBPASS_CONTENTS_INLINE,
    SECONDARY_COMMAND_BUFFERS = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
};

enum class PipelineBindPointType {
    GRAPHICS = VK_PIPELINE_BIND_POINT_GRAPHICS,
    COMPUTE = VK_PIPELINE_BIND_POINT_COMPUTE,
    RAY_TRACING_KHR = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
    SUBPASS_SHADING_HUAWEI = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI,
};

struct RenderPassBeginInfo {
    std::shared_ptr<vki::RenderPass> renderPass;
    std::shared_ptr<vki::Framebuffer> framebuffer;
    std::vector<VkClearValue> clearValues;
    VkRect2D renderArea;
    VkRenderPassBeginInfo toVkBeginInfo() const;
};

class CommandBuffer {
    VkCommandBuffer vkCommandBuffer;

public:
    explicit CommandBuffer(const VkCommandPool &commandPool,
                           const VkDevice &logicalDevice);
    const VkCommandBuffer getVkCommandBuffer() const;
    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(const CommandBuffer &&) = delete;
    void reset() const;
    void begin(const CommandBufferUsage &usage =
                   CommandBufferUsage::ONE_TIME_SUBMIT) const;
    void end() const;
    void beginRenderPass(
        const vki::RenderPassBeginInfo &renderPassBeginInfo,
        const vki::SubpassContentsType &subpassContentsType) const;
    void bindPipeline(
        const vki::GraphicsPipeline &pipeline,
        const vki::PipelineBindPointType &pipelineBindPointType) const;
    void endRenderPass() const;
};
};  // namespace vki
