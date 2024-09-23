#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <functional>
#include <vector>

#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
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
    vki::RenderPass renderPass;
    vki::Framebuffer framebuffer;
    std::vector<VkClearValue> clearValues;
    VkRect2D renderArea;
    VkRenderPassBeginInfo toVkBeginInfo() const;
};

struct DrawArgs {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex = 0;
    uint32_t firstInstance = 0;
};

struct DrawIndexedArgs {
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
    uint32_t firstInstance;
};

struct BindVertexBuffersArgs {
    unsigned int firstBinding;
    unsigned int bindingCount;
    std::vector<vki::Buffer> buffers;
    std::vector<VkDeviceSize> offsets;
};

struct BindIndexBufferArgs {
    vki::Buffer buffer;
    VkDeviceSize offset;
    VkIndexType type;
};

struct BindDescriptorSetsArgs {
    vki::PipelineBindPointType bindPointType;
    vki::PipelineLayout pipelineLayout;
    unsigned int firstSet;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<unsigned int> dynamicOffsets;
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
    void copyBuffer(const vki::Buffer &srcBuffer, const vki::Buffer dstBuffer,
                    const std::vector<VkBufferCopy> &copyRegions) const;
    void end() const;
    void beginRenderPass(
        const vki::RenderPassBeginInfo &renderPassBeginInfo,
        const vki::SubpassContentsType &subpassContentsType) const;
    void bindPipeline(
        const vki::GraphicsPipeline &pipeline,
        const vki::PipelineBindPointType &pipelineBindPointType) const;
    void endRenderPass() const;
    void draw(const vki::DrawArgs &args) const;
    void drawIndexed(const vki::DrawIndexedArgs &args) const;
    void bindVertexBuffers(const vki::BindVertexBuffersArgs &args) const;
    void bindIndexBuffer(const vki::BindIndexBufferArgs &args) const;
    void bindDescriptorSet(const vki::BindDescriptorSetsArgs &args) const;
    void record(const std::function<void()> &func) const;
    void withRenderPass(const vki::RenderPassBeginInfo &renderPassBeginInfo,
                        const vki::SubpassContentsType &subpassContentsType,
                        const std::function<void()> &func) const;
};
};  // namespace vki
