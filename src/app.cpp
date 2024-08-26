#include "./app.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "./glfw_controller.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/fence.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/memory.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/shader_module.hpp"
#include "vulkan_app/vki/swapchain.hpp"
#include "vulkan_app/vki/utils.hpp"
#define ELPP_STL_LOGGING
#include "easylogging++.h"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/structs.hpp"
#include "vulkan_app/vki/surface.hpp"

vki::PhysicalDevice pickPhysicalDevice(const vki::VulkanInstance &instance,
                                       const vki::Surface &surface,
                                       el::Logger &logger);

vki::QueueFamilyWithOp<1, vki::QueueOperationType::GRAPHIC,
                       vki::QueueOperationType::PRESENT>
pickQueueFamily(const std::vector<std::shared_ptr<vki::QueueFamily>> &families);

const vki::Swapchain createSwapChain(
    const vki::Surface &surface, const vki::PhysicalDevice &physicalDevice,
    const vki::LogicalDevice &logicalDevice,
    const vki::QueueFamily &graphicsQueueFamily,
    const vki::QueueFamily &presentQueueFamily,
    const GLFWControllerWindow &window);

vki::GraphicsPipeline createGraphicsPipeline(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const std::shared_ptr<vki::RenderPass> &renderPass,
    const vki::PipelineLayout &pipelineLayout);

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        return { .binding = 0,
                 .stride = sizeof(Vertex),
                 .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
    };

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
        return {
            (VkVertexInputAttributeDescription){
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, pos) },
            (VkVertexInputAttributeDescription){
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color) },
        };
    };
};

const std::vector<Vertex> vertices = {
    { .pos = { 0.0f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f } },
    { .pos = { 0.5f, 0.5f }, .color = { 1.0f, 1.0f, 0.0f } },
    { .pos = { -0.5f, 0.5f }, .color = { 0.0f, 0.0f, 1.0f } }
};

void drawFrame(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const std::shared_ptr<vki::RenderPass> &renderPass,
    const vki::GraphicsPipeline &pipeline,
    const std::vector<std::shared_ptr<vki::Framebuffer>> &framebuffers,
    const vki::CommandBuffer &commandBuffer, const vki::Fence &inFlightFence,
    const vki::Semaphore &imageAvailableSemaphore,
    const vki::Semaphore &renderFinishedSemaphore,
    const vki::Buffer &vertexBuffer,
    const vki::GraphicsQueueMixin &graphicsQueue,
    const vki::PresentQueueMixin &presentQueue);

void recordCommandBuffer(const std::shared_ptr<vki::Framebuffer> &framebuffer,
                         const vki::Swapchain &swapchain,
                         const std::shared_ptr<vki::RenderPass> &renderPass,
                         const vki::GraphicsPipeline &pipeline,
                         const vki::CommandBuffer &commandBuffer,
                         const vki::Buffer &vertexBuffer);

void run_app() {
    auto &mainLogger = *el::Loggers::getLogger("main");
    mainLogger.info("Creating GLFWController...");
    GLFWController controller;
    mainLogger.info("Created GLFWController");
    mainLogger.info("Obtaining GLFWControllerWindow...");
    GLFWControllerWindow window =
        controller.createWindow("Hello triangle", 800, 600);
    mainLogger.info("Obtained GLFWControllerWindow...");
    const auto &requiredExtensions = controller.getRequiredExtensions();
    mainLogger.info("GLFW Required extensions: ");
    mainLogger.info(requiredExtensions);
    vki::VulkanInstance instance({
        .extensions = requiredExtensions,
        .appName = "Hello triangle",
        .appVersion = { 1, 0, 0 },
        .apiVersion = VK_API_VERSION_1_3,
        .layers = { "VK_LAYER_KHRONOS_validation" },
    });
    vki::Surface surface(instance, window);
    vki::PhysicalDevice physicalDevice =
        pickPhysicalDevice(instance, surface, mainLogger);
    const auto &queueFamilies = physicalDevice.getQueueFamilies();
    mainLogger.info(
        std::format("Picked physical device: {}", (std::string)physicalDevice));
    const auto &queueFamily = pickQueueFamily(queueFamilies);
    mainLogger.info(std::format("Picked graphics and present queue family: {}",
                                (std::string)*queueFamily.family));
    const auto &queueCreateInfo =
        vki::QueueCreateInfo<1, 1, vki::QueueOperationType::GRAPHIC,
                             vki::QueueOperationType::PRESENT>(queueFamily);
    const vki::LogicalDevice logicalDevice =
        vki::LogicalDevice(physicalDevice, std::make_tuple(queueCreateInfo));
    mainLogger.info("Created logical device");
    const auto &queue = logicalDevice.getQueue<0>(queueCreateInfo);

    const vki::Swapchain &swapchain =
        createSwapChain(surface, physicalDevice, logicalDevice,
                        *queueFamily.family, *queueFamily.family, window);
    mainLogger.info("Created swapchain");
    const auto renderPass =
        std::make_shared<vki::RenderPass>(swapchain.getFormat(), logicalDevice);
    mainLogger.info("Created render pass");
    const auto pipelineLayout = vki::PipelineLayout(logicalDevice);
    mainLogger.info("Created pipeline layout");
    const auto pipeline = createGraphicsPipeline(logicalDevice, swapchain,
                                                 renderPass, pipelineLayout);
    mainLogger.info("Created pipeline");
    const auto framebuffers =
        swapchain.swapChainImageViews |
        std::views::transform([&swapchain, &renderPass,
                               &logicalDevice](const auto &imageView) {
            return std::make_shared<vki::Framebuffer>(swapchain, renderPass,
                                                      swapchain.getExtent(),
                                                      logicalDevice, imageView);
        }) |
        std::ranges::to<std::vector>();
    mainLogger.info("Created framebuffers");
    const auto &commandPool = vki::CommandPool(logicalDevice, queueFamily);
    mainLogger.info("Created command pool");
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    auto vertexBuffer = vki::Buffer(logicalDevice, bufferInfo);
    mainLogger.info("Created vertex buffer");
    const auto &memoryRequirements = vertexBuffer.getMemoryRequirements();
    const auto &memoryProperties = physicalDevice.getMemoryProperties();
    const auto &memoryTypeIndex = vki::utils::findMemoryType(
        memoryRequirements.memoryTypeBits, memoryProperties,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    const auto &vertexBufferMemory =
        std::make_shared<vki::Memory>(logicalDevice, allocInfo);
    mainLogger.info("Created vertex buffer memory");
    vertexBuffer.bindMemory(vertexBufferMemory);
    void *data;
    vertexBufferMemory->mapMemory(bufferInfo.size, &data);
    memcpy(data, vertices.data(), (size_t)bufferInfo.size);
    vertexBufferMemory->unmapMemory();
    mainLogger.info("Filled vertex buffer memory");
    const auto &commandBuffer = commandPool.createCommandBuffer();
    mainLogger.info("Created command buffer");
    const auto &imageAvailableSemaphore = vki::Semaphore(logicalDevice);
    const auto &renderFinishedSemaphore = vki::Semaphore(logicalDevice);
    const auto &inFlightFence = vki::Fence(logicalDevice);
    mainLogger.info("Created semaphores and fences");
    mainLogger.info("Entering main loop...");
    while (!window.shouldClose()) {
        controller.pollEvents();
        drawFrame(logicalDevice, swapchain, renderPass, pipeline, framebuffers,
                  commandBuffer, inFlightFence, imageAvailableSemaphore,
                  renderFinishedSemaphore, vertexBuffer, queue, queue);
    };

    mainLogger.info("Waiting for queued operations to complete...");
    logicalDevice.waitIdle();
};

const auto &queueFamilyFilter =
    [](const vki::QueueFamily &queueFamily) -> bool {
    return queueFamily.doesSupportsOperations(
               { vki::QueueOperationType::GRAPHIC,
                 vki::QueueOperationType::PRESENT }) &&
           queueFamily.queueCount >= 1;
};

std::function<bool(const VkExtensionProperties &)> buildExtensionFilter(
    const std::string &name) {
    return [&name](const VkExtensionProperties &ext) -> bool {
        return std::strcmp(name.c_str(), ext.extensionName) == 0;
    };
};

vki::PhysicalDevice pickPhysicalDevice(const vki::VulkanInstance &instance,
                                       const vki::Surface &surface,
                                       el::Logger &logger) {
    logger.info("Getting all available physical devices...");
    const auto &devices = instance.getPhysicalDevices(surface);
    const auto &it = std::ranges::find_if(
        devices, [](const vki::PhysicalDevice &device) -> bool {
            const bool hasNeccesaryQueueFamilies =
                device.hasQueueFamilies({ queueFamilyFilter });
            const bool hasNeccesaryExtensions = device.hasExtensions(
                { buildExtensionFilter(VK_KHR_SWAPCHAIN_EXTENSION_NAME) });
            return hasNeccesaryQueueFamilies && hasNeccesaryExtensions;
        });
    if (it == devices.end()) {
        throw std::runtime_error("No suitable physical devices present");
    };
    return *it;
};

vki::QueueFamilyWithOp<1, vki::QueueOperationType::GRAPHIC,
                       vki::QueueOperationType::PRESENT>
pickQueueFamily(
    const std::vector<std::shared_ptr<vki::QueueFamily>> &families) {
    return *std::ranges::find_if(families, [](const auto &family) -> bool {
        return queueFamilyFilter(*family);
    });
};

const vki::Swapchain createSwapChain(
    const vki::Surface &surface, const vki::PhysicalDevice &physicalDevice,
    const vki::LogicalDevice &logicalDevice,
    const vki::QueueFamily &graphicsQueueFamily,
    const vki::QueueFamily &presentQueueFamily,
    const GLFWControllerWindow &window) {
    auto details = physicalDevice.getSwapchainDetails(surface);
    return vki::Swapchain(logicalDevice, physicalDevice, graphicsQueueFamily,
                          presentQueueFamily, surface, window);
};

std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    };
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
};

vki::GraphicsPipeline createGraphicsPipeline(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const std::shared_ptr<vki::RenderPass> &renderPass,
    const vki::PipelineLayout &pipelineLayout) {
    auto vertShaderCode = readFile("../src/shaders/vertex.spv");
    auto fragmentShaderCode = readFile("../src/shaders/fragment.spv");
    auto vertShader = vki::ShaderModule(logicalDevice, vertShaderCode);
    auto fragmentShader = vki::ShaderModule(logicalDevice, fragmentShaderCode);
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = attributeDescriptions.size(),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
    return vki::GraphicsPipeline(
        vertShader, fragmentShader, swapchain.getExtent(), pipelineLayout,
        renderPass, logicalDevice, vertexInputCreateInfo);
};

void drawFrame(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const std::shared_ptr<vki::RenderPass> &renderPass,
    const vki::GraphicsPipeline &pipeline,
    const std::vector<std::shared_ptr<vki::Framebuffer>> &framebuffers,
    const vki::CommandBuffer &commandBuffer, const vki::Fence &inFlightFence,
    const vki::Semaphore &imageAvailableSemaphore,
    const vki::Semaphore &renderFinishedSemaphore,
    const vki::Buffer &vertexBuffer,
    const vki::GraphicsQueueMixin &graphicsQueue,
    const vki::PresentQueueMixin &presentQueue) {
    inFlightFence.wait();

    uint32_t imageIndex =
        swapchain.acquireNextImageKHR(imageAvailableSemaphore);
    commandBuffer.reset();
    recordCommandBuffer(framebuffers[imageIndex], swapchain, renderPass,
                        pipeline, commandBuffer, vertexBuffer);

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

void recordCommandBuffer(const std::shared_ptr<vki::Framebuffer> &framebuffer,
                         const vki::Swapchain &swapchain,
                         const std::shared_ptr<vki::RenderPass> &renderPass,
                         const vki::GraphicsPipeline &pipeline,
                         const vki::CommandBuffer &commandBuffer,
                         const vki::Buffer &vertexBuffer) {
    commandBuffer.begin();

    VkClearValue clearColor = { .color = {
                                    .float32 = { 0.0f, 0.0f, 0.0f, 1.0f } } };
    vki::RenderPassBeginInfo renderPassBeginInfo = {
        .renderPass = renderPass,
        .framebuffer = framebuffer,
        .clearValues = { clearColor },
        .renderArea = { .offset = { 0, 0 }, .extent = swapchain.getExtent() }
    };
    commandBuffer.beginRenderPass(renderPassBeginInfo,
                                  vki::SubpassContentsType::INLINE);
    commandBuffer.bindPipeline(pipeline, vki::PipelineBindPointType::GRAPHICS);
    VkBuffer vertexBuffers[] = { vertexBuffer.getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer.getVkCommandBuffer(), 0, 1,
                           vertexBuffers, offsets);
    vkCmdDraw(commandBuffer.getVkCommandBuffer(), 3, 1, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();
};
