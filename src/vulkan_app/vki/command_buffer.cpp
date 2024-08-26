#include "./command_buffer.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"

vki::CommandBuffer::CommandBuffer(const VkCommandPool &commandPool,
                                  const VkDevice &logicalDevice) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result =
        vkAllocateCommandBuffers(logicalDevice, &allocInfo, &vkCommandBuffer);
    vki::assertSuccess(result, "vkAllocateCommandBuffers");
};

const VkCommandBuffer vki::CommandBuffer::getVkCommandBuffer() const {
    return vkCommandBuffer;
};

void vki::CommandBuffer::reset() const {
    VkResult result = vkResetCommandBuffer(vkCommandBuffer, 0);
    assertSuccess(result, "vkResetCommandBuffer");
};

void vki::CommandBuffer::begin(const CommandBufferUsage &usage) const {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = static_cast<unsigned int>(usage)
    };
    VkResult result = vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
    vki::assertSuccess(result, "vkBeginCommandBuffer");
};

void vki::CommandBuffer::end() const {
    VkResult result = vkEndCommandBuffer(vkCommandBuffer);
    vki::assertSuccess(result, "vkEndCommandBuffer");
};

void vki::CommandBuffer::beginRenderPass(
    const vki::RenderPassBeginInfo &renderPassBeginInfo,
    const vki::SubpassContentsType &subpassContentsType) const {
    const auto &renderPassInfo = renderPassBeginInfo.toVkBeginInfo();
    vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo,
                         static_cast<VkSubpassContents>(subpassContentsType));
};

void vki::CommandBuffer::bindPipeline(
    const vki::GraphicsPipeline &pipeline,
    const vki::PipelineBindPointType &pipelineBindPointType) const {
    vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline.getVkPipeline());
};

void vki::CommandBuffer::endRenderPass() const {
    vkCmdEndRenderPass(vkCommandBuffer);
};

VkRenderPassBeginInfo vki::RenderPassBeginInfo::toVkBeginInfo() const {
    return { .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
             .renderPass = renderPass->getVkRenderPass(),
             .framebuffer = framebuffer->getVkFramebuffer(),
             .renderArea = renderArea,
             .clearValueCount = static_cast<unsigned int>(clearValues.size()),
             .pClearValues = clearValues.data() };
};
