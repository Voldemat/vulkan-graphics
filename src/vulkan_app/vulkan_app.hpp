#ifndef SRC_VULKAN_APPLICATION
#define SRC_VULKAN_APPLICATION

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

#define ELPP_STL_LOGGING
#include "easylogging++.h"
#include "glfw_controller.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/fence.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/swapchain.hpp"

class VulkanApplication {
    vki::VulkanInstance instance;
    el::Logger *logger;
    VkFormat swapChainFormat;
    VkExtent2D swapChainExtent;
    const vki::PhysicalDevice pickPhysicalDevice();
    const vki::Swapchain createSwapChain(
        const vki::PhysicalDevice &physicalDevice,
        const vki::LogicalDevice &logicalDevice,
        const vki::QueueFamily &graphicsQueueFamily,
        const vki::QueueFamily &presentQueueFamily,
        const GLFWControllerWindow &window);
    vki::GraphicsPipeline createGraphicsPipeline(
        const vki::LogicalDevice &logicalDevice,
        const vki::RenderPass &renderPass,
        const vki::PipelineLayout &pipelineLayout);
    void recordCommandBuffer(
        const std::shared_ptr<vki::Framebuffer> &framebuffer,
        const vki::RenderPass &renderPass,
        const vki::GraphicsPipeline &pipeline,
        const vki::CommandBuffer &commandBuffer,
        const vki::Buffer &vertexBuffer);

public:
    void drawFrame(
        const vki::LogicalDevice &logicalDevice,
        const vki::Swapchain &swapchain, const vki::RenderPass &renderPass,
        const vki::GraphicsPipeline &pipeline,
        const std::vector<std::shared_ptr<vki::Framebuffer>> &framebuffers,
        const vki::CommandBuffer &commandBuffer,
        const vki::Fence &inFlightFence,
        const vki::Semaphore &imageAvailableSemaphore,
        const vki::Semaphore &renderFinishedSemaphore,
        const vki::Buffer &vertexBuffer,
        const vki::GraphicsQueueMixin &graphicsQueue,
        const vki::PresentQueueMixin &presentQueue);
    VulkanApplication(const VulkanApplication &other) = delete;
    VulkanApplication(vki::VulkanInstanceParams params,
                      const GLFWController &controller,
                      const GLFWControllerWindow &window);
};
#endif
