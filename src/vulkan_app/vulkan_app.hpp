#ifndef SRC_VULKAN_APPLICATION
#define SRC_VULKAN_APPLICATION

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "glfw_controller.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/swapchain.hpp"


class VulkanApplication {
    vki::VulkanInstance instance;
    std::optional<std::unique_ptr<vki::LogicalDevice>> device;
    std::optional<vki::PhysicalDevice> physicalDevice;
    std::optional<std::unique_ptr<vki::Swapchain>> swapchain;
    std::optional<uint32_t> graphicsQueueIndex;
    std::optional<uint32_t> presentQueueIndex;
    std::vector<VkImageView> swapChainImageViews;
    VkFormat swapChainFormat;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(const GLFWControllerWindow &window);
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void recordCommandBuffer(uint32_t imageIndex);
    void createSyncObjects();
    VkShaderModule createShaderModule(const std::vector<char> &code);

public:
    void drawFrame();
    VulkanApplication(const VulkanApplication &other) = delete;
    VulkanApplication(vki::VulkanInstanceParams params,
                      const GLFWControllerWindow &window);
    ~VulkanApplication();
};
#endif
