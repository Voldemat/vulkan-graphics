#include "./app.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <functional>
#include <limits>
#include <optional>
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
#include "shaders.hpp"
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
    { .pos = { -0.5f, 0.5f }, .color = { 0.0f, 0.0f, 1.0f } },
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
               const vki::Buffer &vertexBuffer,
               const vki::GraphicsQueueMixin &graphicsQueue,
               const vki::PresentQueueMixin &presentQueue);

void recordCommandBuffer(const vki::Framebuffer &framebuffer,
                         const vki::Swapchain &swapchain,
                         const VkExtent2D &swapchainExtent,
                         const vki::RenderPass &renderPass,
                         const vki::GraphicsPipeline &pipeline,
                         const vki::CommandBuffer &commandBuffer,
                         const vki::Buffer &vertexBuffer);

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

vki::GraphicsPipeline createGraphicsPipeline(
    const vki::LogicalDevice &logicalDevice, el::Logger &logger,
    VkExtent2D swapchainExtent, const vki::RenderPass &renderPass) {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    const auto pipelineLayout =
        vki::PipelineLayout(logicalDevice, pipelineLayoutCreateInfo);
    logger.info("Created pipeline layout");

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

    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShader.getVkShaderModule(),
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragmentShader.getVkShaderModule(),
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertexShaderCreateInfo, fragmentShaderCreateInfo
    };

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchainExtent.width),
        .height = static_cast<float>(swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = { .offset = { .x = 100, .y = 100 },
                         .extent = swapchainExtent };
    VkPipelineViewportStateCreateInfo viewportCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisample = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT

    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &pipelineInputAssemblyCreateInfo,
        .pViewportState = &viewportCreateInfo,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisample,
        .pColorBlendState = &colorBlending,
        .layout = pipelineLayout.getVkPipelineLayout(),
        .renderPass = renderPass.getVkRenderPass(),
        .subpass = 0,
    };
    return vki::GraphicsPipeline(logicalDevice, pipelineInfo);
};

vki::Buffer createStagingBuffer(const vki::LogicalDevice &logicalDevice,
                                const vki::PhysicalDevice &physicalDevice,
                                el::Logger &logger) {
    VkBufferCreateInfo stagingBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(vertices[0]) * vertices.size(),
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    auto stagingBuffer = vki::Buffer(logicalDevice, stagingBufferCreateInfo);
    logger.info("Created vertex buffer");
    const auto &memoryProperties = physicalDevice.getMemoryProperties();
    const auto &stagingBufferMemoryRequirements =
        stagingBuffer.getMemoryRequirements();
    const auto &stagingBufferMemoryTypeIndex = vki::utils::findMemoryType(
        stagingBufferMemoryRequirements.memoryTypeBits, memoryProperties,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = stagingBufferMemoryRequirements.size,
        .memoryTypeIndex = stagingBufferMemoryTypeIndex,
    };
    auto stagingBufferMemory = vki::Memory(logicalDevice, allocInfo);
    logger.info("Created vertex buffer memory");
    stagingBuffer.bindMemoryAndTakeOwnership(stagingBufferMemory);
    stagingBufferMemory.write(allocInfo.allocationSize,
                              (void *)vertices.data());
    logger.info("Filled vertex buffer memory");
    return stagingBuffer;
};

vki::Buffer createVertexBuffer(const vki::LogicalDevice &logicalDevice,
                               const vki::PhysicalDevice &physicalDevice,
                               el::Logger &logger,
                               const vki::CommandPool &commandPool,
                               const vki::GraphicsQueueMixin &graphicsQueue) {
    const auto &stagingBuffer =
        createStagingBuffer(logicalDevice, physicalDevice, logger);
    VkBufferCreateInfo vertexBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(vertices[0]) * vertices.size(),
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    auto vertexBuffer = vki::Buffer(logicalDevice, vertexBufferCreateInfo);
    logger.info("Created vertex buffer");
    const auto &memoryProperties = physicalDevice.getMemoryProperties();
    const auto &vertexBufferMemoryRequirements =
        vertexBuffer.getMemoryRequirements();
    const auto &vertexBufferMemoryTypeIndex = vki::utils::findMemoryType(
        vertexBufferMemoryRequirements.memoryTypeBits, memoryProperties,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = vertexBufferMemoryRequirements.size,
        .memoryTypeIndex = vertexBufferMemoryTypeIndex,
    };
    auto vertexBufferMemory = vki::Memory(logicalDevice, allocInfo);
    logger.info("Created vertex buffer memory");
    vertexBuffer.bindMemoryAndTakeOwnership(vertexBufferMemory);
    const auto &commandBuffer = commandPool.createCommandBuffer();
    commandBuffer.begin();
    commandBuffer.copyBuffer(stagingBuffer, vertexBuffer,
                             { (VkBufferCopy){
                                 .srcOffset = 0,
                                 .dstOffset = 0,
                                 .size = allocInfo.allocationSize,
                             } });
    commandBuffer.end();
    graphicsQueue.submit({ vki::SubmitInfo((vki::SubmitInfoInputData){
                             .commandBuffers = { &commandBuffer } }) },
                         std::nullopt);
    graphicsQueue.waitIdle();
    return vertexBuffer;
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

    const auto &pipeline = createGraphicsPipeline(logicalDevice, mainLogger,
                                                  swapchainExtent, renderPass);
    mainLogger.info("Created pipeline");
    const auto &framebuffers =
        swapchain.swapChainImageViews |
        std::views::transform([&swapchain, &swapchainExtent, &renderPass,
                               &logicalDevice](const auto &imageView) {
            VkImageView attachments[] = { imageView };

            VkFramebufferCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass.getVkRenderPass(),
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = swapchainExtent.width,
                .height = swapchainExtent.height,
                .layers = 1,
            };
            return vki::Framebuffer(logicalDevice, createInfo);
        }) |
        std::ranges::to<std::vector>();
    mainLogger.info("Created framebuffers");
    const auto &commandPool = vki::CommandPool(logicalDevice, queueFamily);
    mainLogger.info("Created command pool");
    const auto &vertexBuffer = createVertexBuffer(
        logicalDevice, physicalDevice, mainLogger, commandPool, queue);
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
               const vki::Buffer &vertexBuffer,
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
                         const vki::Buffer &vertexBuffer) {
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
