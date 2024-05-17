#ifndef SRC_VULKAN_APPLICATION
#define SRC_VULKAN_APPLICATION

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "glfw_controller.hpp"
#include "vulkan_app/vki/vki_instance.hpp"
#include "vulkan_app/vki/vki_physical_device.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void assertSuccess(const VkResult &result, const std::string message);

class VulkanApplication {
    vki::VulkanInstance instance;
    VkDevice device;
    std::optional<vki::PhysicalDevice> physicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    std::optional<uint32_t> graphicsQueueIndex;
    std::optional<uint32_t> presentQueueIndex;
    std::vector<VkImage> swapChainImages;
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
    SwapChainSupportDetails queryDeviceSwapChainSupportDetails(
        const VkPhysicalDevice &device);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                                const GLFWControllerWindow &window);
    VkPresentModeKHR choosePresentMode(const SwapChainSupportDetails &details);
    VkSurfaceFormatKHR chooseFormat(const SwapChainSupportDetails &details);

public:
    void drawFrame();
    VulkanApplication(const VulkanApplication &other) = delete;
    VulkanApplication(vki::VulkanInstanceParams params,
                      const GLFWControllerWindow &window);
    ~VulkanApplication();
};
#endif
