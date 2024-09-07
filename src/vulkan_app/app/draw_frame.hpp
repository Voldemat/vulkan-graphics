#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

#include "vulkan_app/app/frame_state.hpp"
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
#include "vulkan_app/vki/swapchain.hpp"

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
               const uint32_t &indicesSize,
               const FrameState& frameState);
