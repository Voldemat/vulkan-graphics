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
#include <iostream>
#include <limits>
#include <memory>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_set>
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
#include "vulkan_app/vki/queue_family.hpp"
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
pickQueueFamily(const std::vector<vki::QueueFamily> &families);

vki::GraphicsPipeline createGraphicsPipeline(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass,
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
               const std::shared_ptr<vki::Buffer> &vertexBuffer,
               const vki::GraphicsQueueMixin &graphicsQueue,
               const vki::PresentQueueMixin &presentQueue);

void recordCommandBuffer(const vki::Framebuffer &framebuffer,
                         const vki::Swapchain &swapchain,
                         const VkExtent2D &swapchainExtent,
                         const vki::RenderPass &renderPass,
                         const vki::GraphicsPipeline &pipeline,
                         const vki::CommandBuffer &commandBuffer,
                         const std::shared_ptr<vki::Buffer> &vertexBuffer);

vki::PresentMode choosePresentMode(
    const std::unordered_set<vki::PresentMode> &presentModes) {
    if (presentModes.contains(vki::PresentMode::MAILBOX_KHR))
        return vki::PresentMode::MAILBOX_KHR;
    return vki::PresentMode::IMMEDIATE_KHR;
};

const VkSurfaceFormatKHR requiredFormat = {
    .format = VK_FORMAT_B8G8R8A8_SRGB,
    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
};

VkSurfaceFormatKHR chooseFormat(const vki::SurfaceFormatSet &formats) {
    if (formats.contains(requiredFormat)) return requiredFormat;
    throw std::runtime_error("Required surface format is not found");
};

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                            const GLFWControllerWindow &window) {
    if (capabilities.currentExtent.width <=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    };
    const auto &[width, height] = window.getFramebufferSize();
    return (VkExtent2D){
        .width = std::clamp(width, capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width),
        .height = std::clamp(height, capabilities.minImageExtent.height,
                             capabilities.maxImageExtent.height)
    };
};

uint32_t getImageCount(const VkSurfaceCapabilitiesKHR &capabilities) {
    return std::clamp(capabilities.minImageCount + 1,
                      capabilities.minImageCount, capabilities.maxImageCount);
};

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
    const auto &surfaceDetails = surface.getDetails(physicalDevice);
    const auto &swapchainExtent =
        chooseSwapExtent(surfaceDetails.capabilities, window);
    const auto &swapchainFormat = chooseFormat(surfaceDetails.formats);
    const auto &surfaceMinImageCount =
        getImageCount(surfaceDetails.capabilities);
    const auto &swapchainPresentMode =
        choosePresentMode(surfaceDetails.presentModes);
    const vki::Swapchain &swapchain = vki::Swapchain(
        logicalDevice,
        { .surface = surface,
          .extent = swapchainExtent,
          .presentMode = swapchainPresentMode,
          .format = swapchainFormat,
          .minImageCount = surfaceMinImageCount,
          .preTransform = surfaceDetails.capabilities.currentTransform,
          .imageUsage = { vki::ImageUsage::COLOR_ATTACHMENT },
          .compositeAlpha = vki::CompositeAlpha::OPAQUE_BIT_KHR,
          .isClipped = true,
          .sharingInfo = vki::SwapchainSharingInfo(queueFamily.family,
                                                   queueFamily.family) });
    mainLogger.info("Created swapchain");

    VkAttachmentDescription colorAttachment = {
        .format = swapchainFormat.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    const vki::RenderPassCreateInfo renderPassCreateInfo = {
        .attachments = { colorAttachment },
        .subpasses = { subpass },
        .dependencies = { dependency },
    };
    const auto renderPass =
        vki::RenderPass(logicalDevice, renderPassCreateInfo);
    mainLogger.info("Created render pass");

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    const auto pipelineLayout =
        vki::PipelineLayout(logicalDevice, pipelineLayoutCreateInfo);
    mainLogger.info("Created pipeline layout");

    const auto pipeline = createGraphicsPipeline(
        logicalDevice, swapchain, swapchainExtent, renderPass, pipelineLayout);
    mainLogger.info("Created pipeline");
    const auto &framebuffers =
        swapchain.swapChainImageViews |
        std::views::transform([&swapchain, &swapchainExtent, &renderPass,
                               &logicalDevice](const auto &imageView) {
            return vki::Framebuffer(swapchain, renderPass, swapchainExtent,
                                    logicalDevice, imageView);
        }) |
        std::views::as_rvalue | std::ranges::to<std::vector>();
    mainLogger.info("Created framebuffers");
    const auto &commandPool = vki::CommandPool(logicalDevice, queueFamily);
    mainLogger.info("Created command pool");
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    auto vertexBuffer =
        std::make_shared<vki::Buffer>(logicalDevice, bufferInfo);
    mainLogger.info("Created vertex buffer");
    const auto &memoryRequirements = vertexBuffer->getMemoryRequirements();
    const auto &memoryProperties = physicalDevice.getMemoryProperties();
    const auto &memoryTypeIndex = vki::utils::findMemoryType(
        memoryRequirements.memoryTypeBits, memoryProperties,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    const auto &vertexBufferMemory = vki::Memory(logicalDevice, allocInfo);
    mainLogger.info("Created vertex buffer memory");
    vertexBuffer->bindMemory(vertexBufferMemory);
    void *data;
    vertexBufferMemory.mapMemory(bufferInfo.size, &data);
    memcpy(data, vertices.data(), (size_t)bufferInfo.size);
    vertexBufferMemory.unmapMemory();
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
        drawFrame(logicalDevice, swapchain, swapchainExtent, renderPass,
                  pipeline, framebuffers, commandBuffer, inFlightFence,
                  imageAvailableSemaphore, renderFinishedSemaphore,
                  vertexBuffer, queue, queue);
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
pickQueueFamily(const std::vector<vki::QueueFamily> &families) {
    return &*std::ranges::find_if(families, [](const auto &family) -> bool {
        return queueFamilyFilter(family);
    });
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
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass,
    const vki::PipelineLayout &pipelineLayout) {
    std::vector<char> vertShaderCode(&data_start_shader_vert_spv, &data_end_shader_vert_spv);
    std::vector<char> fragShaderCode(&data_start_shader_frag_spv, &data_end_shader_frag_spv);
    auto vertShader = vki::ShaderModule(logicalDevice, vertShaderCode);
    auto fragmentShader = vki::ShaderModule(logicalDevice, fragShaderCode);
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = attributeDescriptions.size(),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
    return vki::GraphicsPipeline(vertShader, fragmentShader, swapchainExtent,
                                 pipelineLayout, renderPass, logicalDevice,
                                 vertexInputCreateInfo);
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
               const std::shared_ptr<vki::Buffer> &vertexBuffer,
               const vki::GraphicsQueueMixin &graphicsQueue,
               const vki::PresentQueueMixin &presentQueue) {
    inFlightFence.wait();

    uint32_t imageIndex =
        swapchain.acquireNextImageKHR(imageAvailableSemaphore);
    commandBuffer.reset();
    recordCommandBuffer(framebuffers[imageIndex], swapchain, swapchainExtent,
                        renderPass, pipeline, commandBuffer, vertexBuffer);

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

void recordCommandBuffer(const vki::Framebuffer &framebuffer,
                         const vki::Swapchain &swapchain,
                         const VkExtent2D &swapchainExtent,
                         const vki::RenderPass &renderPass,
                         const vki::GraphicsPipeline &pipeline,
                         const vki::CommandBuffer &commandBuffer,
                         const std::shared_ptr<vki::Buffer> &vertexBuffer) {
    commandBuffer.begin();

    VkClearValue clearColor = { .color = {
                                    .float32 = { 0.0f, 0.0f, 0.0f, 1.0f } } };
    vki::RenderPassBeginInfo renderPassBeginInfo = {
        .renderPass = renderPass,
        .framebuffer = framebuffer,
        .clearValues = { clearColor },
        .renderArea = { .offset = { 0, 0 }, .extent = swapchainExtent }
    };
    commandBuffer.beginRenderPass(renderPassBeginInfo,
                                  vki::SubpassContentsType::INLINE);
    commandBuffer.bindPipeline(pipeline, vki::PipelineBindPointType::GRAPHICS);
    commandBuffer.bindVertexBuffers({
        .firstBinding = 0,
        .bindingCount = 1,
        .buffers = { vertexBuffer },
        .offsets = { 0 },
    });
    commandBuffer.draw({ .vertexCount = 3,
                         .instanceCount = 1,
                         .firstVertex = 0,
                         .firstInstance = 0 });
    commandBuffer.endRenderPass();
    commandBuffer.end();
};
