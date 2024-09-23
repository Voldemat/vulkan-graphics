#include "./draw_frame.hpp"

#include <MacTypes.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <vector>

#include "vulkan_app/app/data_aggregator.hpp"
#include "vulkan_app/app/frame_state.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "vulkan_app/app/uniform_buffer_object.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/fence.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/structs.hpp"
#include "vulkan_app/vki/swapchain.hpp"

void updateFrameUniformBuffer(void *bufferMappedMemory,
                              const FrameState &frameState) {
    UniformBufferObject ubo = {
        .model = { { 1.0f, 0.0f, 0.0f, 0.0f },
                   { 0.0f, 1.0f, 0.0f, 0.0f },
                   { 0.0f, 0.0f, 1.0f, 0.0f },
                   { 0.0f, 0.0f, 0.0f, 1.0f } },
        .view = glm::lookAt(frameState.cameraPos,
                            frameState.cameraPos + frameState.cameraFront,
                            frameState.cameraUp),
        .proj = frameState.projection,
    };
    memcpy(bufferMappedMemory, &ubo, sizeof(ubo));
};

void recordCommandBuffer(
    const vki::Framebuffer &framebuffer, const vki::Swapchain &swapchain,
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass,
    const vki::GraphicsPipeline &pipeline,
    const vki::CommandBuffer &commandBuffer, const vki::Buffer &vertexBuffer,
    const vki::Buffer &indexBuffer, const vki::PipelineLayout &pipelineLayout,
    const VkDescriptorSet &descriptorSet,
    const DataAggregator &dataAggregator) {
    commandBuffer.record([&]() {
        //vkCmdUpdateBuffer(commandBuffer.getVkCommandBuffer(),
        //                  vertexBuffer.getVkBuffer(), 0,
        //                  sizeof(dataAggregator.vertexArray[0]) *
        //                      dataAggregator.vertexArray.size(),
        //                  dataAggregator.vertexArray.data());
        //vkCmdUpdateBuffer(commandBuffer.getVkCommandBuffer(),
        //                  indexBuffer.getVkBuffer(), 0,
        //                  sizeof(dataAggregator.indexArray[0]) *
        //                      dataAggregator.indexArray.size(),
        //                  dataAggregator.indexArray.data());
        VkClearValue clearColor = { .color = { .float32 = { 0.0f, 0.0f, 0.0f,
                                                            1.0f } } };
        VkClearValue clearDepth = { .depthStencil = { 1.0f, 0 } };
        vki::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass,
            .framebuffer = framebuffer,
            .clearValues = { clearColor, clearDepth },
            .renderArea = { .offset = { 0, 0 }, .extent = swapchainExtent }
        };
        commandBuffer.withRenderPass(
            renderPassBeginInfo, vki::SubpassContentsType::INLINE, [&]() {
                commandBuffer.bindPipeline(
                    pipeline, vki::PipelineBindPointType::GRAPHICS);
                commandBuffer.bindVertexBuffers({
                    .firstBinding = 0,
                    .bindingCount = 1,
                    .buffers = { vertexBuffer },
                    .offsets = { 0 },
                });
                commandBuffer.bindIndexBuffer({
                    .buffer = indexBuffer,
                    .offset = 0,
                    .type = VK_INDEX_TYPE_UINT32,
                });
                commandBuffer.bindDescriptorSet({
                    .bindPointType = vki::PipelineBindPointType::GRAPHICS,
                    .pipelineLayout = pipelineLayout,
                    .firstSet = 0,
                    .descriptorSets = { descriptorSet },
                    .dynamicOffsets = {},
                });
                for (const auto &shapeData : dataAggregator.shapes) {
                    commandBuffer.drawIndexed({
                        .indexCount = shapeData.indexCount,
                        .instanceCount = 1,
                        .firstIndex = shapeData.indexOffset,
                        .vertexOffset = static_cast<int32_t>(shapeData.vertexOffset),
                        .firstInstance = 0,
                    });
                };
            });
    });
};

void drawFrame(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass,
    const vki::GraphicsPipeline &pipeline,
    const std::vector<vki::Framebuffer> &framebuffers,
    const vki::CommandBuffer &commandBuffer, const vki::Fence &inFlightFence,
    const vki::Semaphore &imageAvailableSemaphore,
    const vki::Semaphore &renderFinishedSemaphore,
    const vki::Buffer &vertexBuffer, const vki::Buffer &indexBuffer,
    const vki::GraphicsQueueMixin &graphicsQueue,
    const vki::PresentQueueMixin &presentQueue,
    const std::vector<void *> &uniformMapped,
    const vki::PipelineLayout &pipelineLayout,
    const std::vector<VkDescriptorSet> &descriptorSets,
    const FrameState &frameState, const DataAggregator &dataAggregator) {
    inFlightFence.waitAndReset();

    uint32_t imageIndex =
        swapchain.acquireNextImageKHR(imageAvailableSemaphore);
    commandBuffer.reset();
    recordCommandBuffer(framebuffers[imageIndex], swapchain, swapchainExtent,
                        renderPass, pipeline, commandBuffer, vertexBuffer,
                        indexBuffer, pipelineLayout, descriptorSets[imageIndex],
                        dataAggregator);
    updateFrameUniformBuffer(uniformMapped[imageIndex], frameState);
    const vki::SubmitInfo submitInfo(
        { .waitSemaphores = { &imageAvailableSemaphore },
          .signalSemaphores = { &renderFinishedSemaphore },
          .commandBuffers = { &commandBuffer },
          .waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT } });
    graphicsQueue.submit({ submitInfo }, &inFlightFence);

    vki::PresentInfo presentInfo(
        { .waitSemaphores = { &renderFinishedSemaphore },
          .swapchains = { &swapchain },
          .imageIndices = { imageIndex } });
    presentQueue.present(presentInfo);
};
