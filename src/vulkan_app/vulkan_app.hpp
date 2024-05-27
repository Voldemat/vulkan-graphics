#ifndef SRC_VULKAN_APPLICATION
#define SRC_VULKAN_APPLICATION

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

#include "glfw_controller.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/swapchain.hpp"

class VulkanApplication {
    vki::VulkanInstance instance;
    VkFormat swapChainFormat;
    VkExtent2D swapChainExtent;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    const vki::PhysicalDevice pickPhysicalDevice();
    const vki::Swapchain createSwapChain(
        const vki::PhysicalDevice &physicalDevice,
        const vki::LogicalDevice &logicalDevice,
        const GLFWControllerWindow &window);
    vki::GraphicsPipeline createGraphicsPipeline(
        const vki::LogicalDevice &logicalDevice,
        const vki::RenderPass &renderPass,
        const vki::PipelineLayout &pipelineLayout);
    void createCommandBuffer(const vki::LogicalDevice &logicalDevice,
                             const vki::CommandPool &commandPool);
    void recordCommandBuffer(
        const std::shared_ptr<vki::Framebuffer> &framebuffer,
        const vki::RenderPass &renderPass,
        const vki::GraphicsPipeline &pipeline);
    void createSyncObjects(const vki::LogicalDevice &logicalDevice);

public:
    void drawFrame(
        const vki::LogicalDevice &logicalDevice,
        const vki::Swapchain &swapchain, const vki::RenderPass &renderPass,
        const vki::GraphicsPipeline &pipeline,
        const std::vector<std::shared_ptr<vki::Framebuffer>> &framebuffers);
    VulkanApplication(const VulkanApplication &other) = delete;
    VulkanApplication(vki::VulkanInstanceParams params,
                      const GLFWController &controller,
                      const GLFWControllerWindow &window);
};
#endif
