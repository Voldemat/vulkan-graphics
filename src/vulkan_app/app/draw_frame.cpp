#include "./draw_frame.hpp"

#include <vulkan/vulkan_core.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <vector>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
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
                              const VkExtent2D &swapchainExtent) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();
    UniformBufferObject ubo = {
        .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                             glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(
            glm::radians(45.0f),
            swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f),
    };
    ubo.proj[1][1] *= -1;
    memcpy(bufferMappedMemory, &ubo, sizeof(ubo));
};

void recordCommandBuffer(
    const vki::Framebuffer &framebuffer, const vki::Swapchain &swapchain,
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass,
    const vki::GraphicsPipeline &pipeline,
    const vki::CommandBuffer &commandBuffer, const vki::Buffer &vertexBuffer,
    const vki::Buffer &indexBuffer, const vki::PipelineLayout &pipelineLayout,
    const VkDescriptorSet &descriptorSet, const uint32_t &indicesSize) {
    commandBuffer.record([&]() {
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
                    .type = VK_INDEX_TYPE_UINT16,
                });
                commandBuffer.bindDescriptorSet({
                    .bindPointType = vki::PipelineBindPointType::GRAPHICS,
                    .pipelineLayout = pipelineLayout,
                    .firstSet = 0,
                    .descriptorSets = { descriptorSet },
                    .dynamicOffsets = {},
                });
                commandBuffer.drawIndexed({ .indexCount = indicesSize,
                                            .instanceCount = 1,
                                            .firstIndex = 0,
                                            .vertexOffset = 0,
                                            .firstInstance = 0 });
            });
    });
};

void drawFrame(const vki::LogicalDevice &logicalDevice,
               const vki::Swapchain &swapchain,
               const VkExtent2D &swapchainExtent,
               const vki::RenderPass &renderPass,
               const vki::GraphicsPipeline &pipeline,
               const std::vector<vki::Framebuffer> &framebuffers,
               const vki::CommandBuffer &commandBuffer,
               const vki::Fence &inFlightFence,
               const vki::Semaphore &imageAvailableSemaphore,
               const vki::Semaphore &renderFinishedSemaphore,
               const vki::Buffer &vertexBuffer, const vki::Buffer &indexBuffer,
               const vki::GraphicsQueueMixin &graphicsQueue,
               const vki::PresentQueueMixin &presentQueue,
               const std::vector<void *> &uniformMapped,
               const vki::PipelineLayout &pipelineLayout,
               const std::vector<VkDescriptorSet> &descriptorSets,
               const uint32_t &indicesSize) {
    inFlightFence.waitAndReset();

    uint32_t imageIndex =
        swapchain.acquireNextImageKHR(imageAvailableSemaphore);
    commandBuffer.reset();
    recordCommandBuffer(framebuffers[imageIndex], swapchain, swapchainExtent,
                        renderPass, pipeline, commandBuffer, vertexBuffer,
                        indexBuffer, pipelineLayout, descriptorSets[imageIndex],
                        indicesSize);
    updateFrameUniformBuffer(uniformMapped[imageIndex], swapchainExtent);
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
