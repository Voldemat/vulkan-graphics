#ifndef SRC_VULKAN_APPLICATION
#define SRC_VULKAN_APPLICATION

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <exception>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "glfw_controller.hpp"

class VulkanError : public std::exception {
    std::string resultString;
    std::string message;
    std::string finalMessage;

public:
    VulkanError(const VkResult &result, std::string msg) {
        resultString = string_VkResult(result);
        message = msg;
        finalMessage = std::format("{}: {}", msg, resultString);
    };
    const char *what() const noexcept { return finalMessage.data(); };
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void assertSuccess(const VkResult &result, const std::string message);

class VulkanApplication {
    VkInstance instance;
    VkApplicationInfo appInfo;
    VkDevice device;
    VkInstanceCreateInfo *createInfo;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR windowSurface;
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
    void createInstance(std::vector<const char *> extensions);
    void createWindowSurface(const GLFWControllerWindow &window);
    void pickPhysicalDevice();
    void pickQueueFamilies();
    void createLogicalDevice();
    void createSwapChain(const GLFWControllerWindow& window);
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void recordCommandBuffer(uint32_t imageIndex);
    void createSyncObjects();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    SwapChainSupportDetails queryDeviceSwapChainSupportDetails(
        const VkPhysicalDevice &device);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                                const GLFWControllerWindow &window);
    VkPresentModeKHR choosePresentMode(const SwapChainSupportDetails& details);
    VkSurfaceFormatKHR chooseFormat(const SwapChainSupportDetails& details);

public:
    void drawFrame();
    VulkanApplication(const VulkanApplication &other) = delete;
    VulkanApplication(std::vector<const char *> extensions,
                      const GLFWControllerWindow &window);
    ~VulkanApplication();
};
#endif
