#include "./app.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <chrono>
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
#include <utility>
#include <vector>

#include "./glfw_controller.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "magic_enum.hpp"
#include "shaders.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/descriptor_pool.hpp"
#include "vulkan_app/vki/descriptor_set_layout.hpp"
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

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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
    { .pos = { -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f } },
    { .pos = { 0.5f, -0.5f }, .color = { 1.0f, 1.0f, 0.0f } },
    { .pos = { 0.5f, 0.5f }, .color = { 0.0f, 0.0f, 1.0f } },
    { .pos = { -0.5f, 0.5f }, .color = { 0.0f, 0.0f, 1.0f } },
};

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

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
               const std::vector<VkDescriptorSet> &descriptorSets);

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

vki::GraphicsPipeline createGraphicsPipeline(
    const vki::LogicalDevice &logicalDevice, el::Logger &logger,
    VkExtent2D swapchainExtent, const vki::RenderPass &renderPass,
    const vki::PipelineLayout &pipelineLayout) {
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
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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

vki::Buffer createStagingBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const std::size_t &size, void *data) {
    VkBufferCreateInfo stagingBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    auto stagingBuffer = vki::Buffer(logicalDevice, stagingBufferCreateInfo);
    logger.info(std::format("Created staging buffer: {}",
                            (void *)stagingBuffer.getVkBuffer()));
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
    logger.info(std::format("Created staging buffer memory: {}",
                            (void *)stagingBufferMemory.getVkMemory()));
    stagingBufferMemory.write(allocInfo.allocationSize, data);
    logger.info(std::format("({}) Filled staging buffer memory",
                            (void *)stagingBufferMemory.getVkMemory()));
    stagingBuffer.bindMemory(std::move(stagingBufferMemory));
    return stagingBuffer;
};

vki::Buffer createVertexBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::Buffer &stagingBuffer,
    const vki::CommandBuffer &commandBuffer,
    const vki::GraphicsQueueMixin &graphicsQueue) {
    const auto &verticesSize = sizeof(vertices[0]) * vertices.size();
    VkBufferCreateInfo vertexBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = verticesSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    auto vertexBuffer = vki::Buffer(logicalDevice, vertexBufferCreateInfo);
    logger.info("Created vertex buffer");
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
    vertexBuffer.bindMemory(std::move(vertexBufferMemory));
    commandBuffer.copyBuffer(stagingBuffer, vertexBuffer,
                             { (VkBufferCopy){
                                 .srcOffset = 0,
                                 .dstOffset = 0,
                                 .size = allocInfo.allocationSize,
                             } });
    return vertexBuffer;
};

vki::Buffer createIndexBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::Buffer &stagingBuffer,
    const vki::CommandBuffer &commandBuffer,
    const vki::GraphicsQueueMixin &graphicsQueue) {
    const auto &indicesSize = sizeof(indices[0]) * indices.size();
    VkBufferCreateInfo indicesBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = indicesSize,
        .usage =
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    auto indicesBuffer = vki::Buffer(logicalDevice, indicesBufferCreateInfo);
    logger.info("Created indices buffer");
    const auto &indicesBufferMemoryRequirements =
        indicesBuffer.getMemoryRequirements();
    const auto &indicesBufferMemoryTypeIndex = vki::utils::findMemoryType(
        indicesBufferMemoryRequirements.memoryTypeBits, memoryProperties,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = indicesBufferMemoryRequirements.size,
        .memoryTypeIndex = indicesBufferMemoryTypeIndex,
    };
    logger.info("Created indices buffer memory");
    indicesBuffer.bindMemory(vki::Memory(logicalDevice, allocInfo));
    commandBuffer.copyBuffer(stagingBuffer, indicesBuffer,
                             { (VkBufferCopy){
                                 .srcOffset = 0,
                                 .dstOffset = 0,
                                 .size = allocInfo.allocationSize,
                             } });
    return indicesBuffer;
};

std::tuple<vki::Buffer, vki::Buffer> createVertexAndIndicesBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::CommandPool &commandPool,
    const vki::GraphicsQueueMixin &graphicsQueue) {
    const auto &verticesSize = sizeof(vertices[0]) * vertices.size();
    const auto &vertexStagingBuffer =
        createStagingBuffer(logicalDevice, memoryProperties, logger,
                            verticesSize, (void *)vertices.data());
    logger.info("Created vertex staging buffer");
    const auto &indicesSize = sizeof(indices[0]) * indices.size();
    const auto &indexStagingBuffer =
        createStagingBuffer(logicalDevice, memoryProperties, logger,
                            indicesSize, (void *)indices.data());
    logger.info("Created index staging buffer");
    const auto &commandBuffer = commandPool.createCommandBuffer();
    commandBuffer.begin();
    auto vertexBuffer =
        createVertexBuffer(logicalDevice, memoryProperties, logger,
                           vertexStagingBuffer, commandBuffer, graphicsQueue);
    auto indicesBuffer =
        createIndexBuffer(logicalDevice, memoryProperties, logger,
                          indexStagingBuffer, commandBuffer, graphicsQueue);
    commandBuffer.end();
    graphicsQueue.submit({ vki::SubmitInfo((vki::SubmitInfoInputData){
                             .commandBuffers = { &commandBuffer } }) },
                         std::nullopt);
    graphicsQueue.waitIdle();
    return { std::move(vertexBuffer), std::move(indicesBuffer) };
};

std::tuple<std::vector<vki::Buffer>, std::vector<void *>> createUniformBuffers(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const unsigned int buffersCount) {
    std::vector<vki::Buffer> buffers;
    buffers.reserve(buffersCount);
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(UniformBufferObject),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    for (int i = 0; i < buffersCount; i++) {
        auto buffer = vki::Buffer(logicalDevice, bufferCreateInfo);
        const auto &memoryRequirements = buffer.getMemoryRequirements();
        const auto &memoryTypeIndex = vki::utils::findMemoryType(
            memoryRequirements.memoryTypeBits, memoryProperties,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex,
        };
        buffer.bindMemory(vki::Memory(logicalDevice, allocInfo));
        buffers.emplace_back(std::move(buffer));
    };
    std::vector<void *> uniformMapped(buffers.size());
    for (const auto &[buffer, memory] :
         std::views::zip(buffers, uniformMapped)) {
        buffer.getMemory().value().mapMemory(sizeof(UniformBufferObject),
                                             &memory);
    };
    logger.info("Mapped uniform buffers");
    return { std::move(buffers), uniformMapped };
};

std::vector<VkDescriptorSet> createDescriptorSets(
    const vki::LogicalDevice &logicalDevice,
    const std::vector<vki::Buffer> &uniformBuffers,
    const vki::DescriptorPool &descriptorPool,
    const vki::DescriptorSetLayout &descriptorSetLayout, el::Logger &logger) {
    std::vector<VkDescriptorSetLayout> layouts(
        uniformBuffers.size(), descriptorSetLayout.getVkDescriptorSetLayout());
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool.getVkDescriptorPool(),
        .descriptorSetCount = static_cast<uint32_t>(uniformBuffers.size()),
        .pSetLayouts = layouts.data()
    };
    const auto &descriptorSets =
        logicalDevice.allocateDescriptorSets(allocInfo);
    logger.info("Allocated descriptor sets");
    std::vector<VkDescriptorBufferInfo> bufferInfos(descriptorSets.size());
    std::vector<VkWriteDescriptorSet> writeInfos(descriptorSets.size());
    for (size_t i = 0; i < descriptorSets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i].getVkBuffer(),
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };
        bufferInfos[i] = bufferInfo;
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &bufferInfo,
            .pTexelBufferView = nullptr
        };
        writeInfos[i] = descriptorWrite;
    };
    logicalDevice.updateWriteDescriptorSets(writeInfos);
    return descriptorSets;
};

vki::DescriptorPool createDescriptorPool(
    const vki::LogicalDevice &logicalDevice,
    const uint32_t &uniformBuffersCount) {
    VkDescriptorPoolSize poolSize = { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      .descriptorCount = uniformBuffersCount };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = uniformBuffersCount,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };
    return vki::DescriptorPool(logicalDevice, descriptorPoolCreateInfo);
};

vki::RenderPass createRenderPass(const vki::LogicalDevice &logicalDevice,
                                 const VkFormat &swapchainFormat) {
    VkAttachmentDescription colorAttachment = {
        .format = swapchainFormat,
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
    return vki::RenderPass(logicalDevice, renderPassCreateInfo);
};

vki::DescriptorSetLayout createDescriptorSetLayout(
    const vki::LogicalDevice &logicalDevice) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
    };
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };
    return vki::DescriptorSetLayout(logicalDevice,
                                    descriptorSetLayoutCreateInfo);
};

vki::PipelineLayout createPipelineLayout(
    const vki::LogicalDevice &logicalDevice,
    const vki::DescriptorSetLayout &descriptorSetLayout) {
    const auto &setLayout = descriptorSetLayout.getVkDescriptorSetLayout();
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &setLayout
    };
    return vki::PipelineLayout(logicalDevice, pipelineLayoutCreateInfo);
};

std::vector<vki::Framebuffer> createFramebuffers(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass) {
    return swapchain.swapChainImageViews |
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
};

void run_app() {
    auto &mainLogger = *el::Loggers::getLogger("main");
    GLFWController controller;
    mainLogger.info("Created GLFWController");

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
    mainLogger.info("Created vulkan instance");

    vki::Surface surface(instance, window);
    mainLogger.info("Created surface");

    vki::PhysicalDevice physicalDevice =
        pickPhysicalDevice(instance, surface, mainLogger);
    mainLogger.info(
        std::format("Picked physical device: {}", (std::string)physicalDevice));

    const auto &queueFamily =
        pickQueueFamily(physicalDevice.getQueueFamilies());
    mainLogger.info(std::format("Picked graphics and present queue family: {}",
                                (std::string)queueFamily.family));

    const auto &queueCreateInfo =
        vki::QueueCreateInfo<1, 1, vki::QueueOperationType::GRAPHIC,
                             vki::QueueOperationType::PRESENT>(queueFamily);
    const vki::LogicalDevice logicalDevice =
        vki::LogicalDevice(physicalDevice, std::make_tuple(queueCreateInfo));
    mainLogger.info("Created logical device");
    const auto &queue = logicalDevice.getQueue<0>(queueCreateInfo);

    const auto &surfaceDetails = surface.getDetails(physicalDevice);
    mainLogger.info("Got surface details");

    const auto &swapchainExtent =
        chooseSwapExtent(surfaceDetails.capabilities, window);
    mainLogger.info(
        std::format("Choose swapchain extent: width - {}, height - {}",
                    swapchainExtent.width, swapchainExtent.height));

    const auto &swapchainFormat = chooseFormat(surfaceDetails.formats);
    mainLogger.info(
        std::format("Choose swapchain format: format - {}, colorSpace - {}",
                    magic_enum::enum_name(swapchainFormat.format),
                    magic_enum::enum_name(swapchainFormat.colorSpace)));

    const auto swapchainMinImageCount =
        std::clamp(surfaceDetails.capabilities.minImageCount + 1,
                   surfaceDetails.capabilities.minImageCount,
                   surfaceDetails.capabilities.maxImageCount);
    mainLogger.info(std::format("Choose swapchain minImageCount: {}",
                                swapchainMinImageCount));

    const auto &swapchainPresentMode =
        choosePresentMode(surfaceDetails.presentModes);
    mainLogger.info(std::format("Choose swapchain present mode: {}",
                                magic_enum::enum_name(swapchainPresentMode)));

    const vki::Swapchain &swapchain = vki::Swapchain(
        logicalDevice,
        { .surface = surface,
          .extent = swapchainExtent,
          .presentMode = swapchainPresentMode,
          .format = swapchainFormat,
          .minImageCount = swapchainMinImageCount,
          .preTransform = surfaceDetails.capabilities.currentTransform,
          .imageUsage = { vki::ImageUsage::COLOR_ATTACHMENT },
          .compositeAlpha = vki::CompositeAlpha::OPAQUE_BIT_KHR,
          .isClipped = true,
          .sharingInfo = vki::SwapchainSharingInfo(queueFamily.family,
                                                   queueFamily.family) });
    mainLogger.info("Created swapchain");

    const auto &renderPass =
        createRenderPass(logicalDevice, swapchainFormat.format);
    mainLogger.info("Created render pass");

    const auto &descriptorSetLayout = createDescriptorSetLayout(logicalDevice);
    mainLogger.info("Created descriptor set layout");

    const auto &pipelineLayout =
        createPipelineLayout(logicalDevice, descriptorSetLayout);
    mainLogger.info("Created pipeline layout");

    const auto &pipeline = createGraphicsPipeline(
        logicalDevice, mainLogger, swapchainExtent, renderPass, pipelineLayout);
    mainLogger.info("Created pipeline");

    const auto &framebuffers = createFramebuffers(logicalDevice, swapchain,
                                                  swapchainExtent, renderPass);
    mainLogger.info("Created framebuffers");

    const auto &commandPool = vki::CommandPool(logicalDevice, queueFamily);
    mainLogger.info("Created command pool");

    const auto &memoryProperties = physicalDevice.getMemoryProperties();
    const auto &[vertexBuffer, indexBuffer] = createVertexAndIndicesBuffer(
        logicalDevice, memoryProperties, mainLogger, commandPool, queue);
    mainLogger.info("Created index and vertex buffers");
    const auto &[uniformBuffers, uniformMappedMemory] =
        createUniformBuffers(logicalDevice, memoryProperties, mainLogger,
                             swapchain.swapChainImageViews.size());
    mainLogger.info("Created uniform buffers");

    const auto &descriptorPool =
        createDescriptorPool(logicalDevice, uniformBuffers.size());
    mainLogger.info("Created descriptor pool");

    const auto &descriptorSets =
        createDescriptorSets(logicalDevice, uniformBuffers, descriptorPool,
                             descriptorSetLayout, mainLogger);
    mainLogger.info("Created descriptor sets");

    const auto &commandBuffer = commandPool.createCommandBuffer();
    mainLogger.info("Created command buffer");

    const auto &imageAvailableSemaphore = vki::Semaphore(logicalDevice);
    const auto &renderFinishedSemaphore = vki::Semaphore(logicalDevice);
    const auto &inFlightFence = vki::Fence(logicalDevice, true);
    mainLogger.info("Created semaphores and fences");

    mainLogger.info("Entering main loop...");
    while (!window.shouldClose()) {
        controller.pollEvents();
        drawFrame(logicalDevice, swapchain, swapchainExtent, renderPass,
                  pipeline, framebuffers, commandBuffer, inFlightFence,
                  imageAvailableSemaphore, renderFinishedSemaphore,
                  vertexBuffer, indexBuffer, queue, queue, uniformMappedMemory,
                  pipelineLayout, descriptorSets);
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
    return *std::ranges::find_if(families, [](const auto &family) -> bool {
        return queueFamilyFilter(family);
    });
};

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
    const VkDescriptorSet &descriptorSet) {
    commandBuffer.record([&]() {
        VkClearValue clearColor = { .color = { .float32 = { 0.0f, 0.0f, 0.0f,
                                                            1.0f } } };
        vki::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass,
            .framebuffer = framebuffer,
            .clearValues = { clearColor },
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
                commandBuffer.drawIndexed(
                    { .indexCount = static_cast<unsigned int>(indices.size()),
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
               const std::vector<VkDescriptorSet> &descriptorSets) {
    inFlightFence.waitAndReset();

    uint32_t imageIndex =
        swapchain.acquireNextImageKHR(imageAvailableSemaphore);
    commandBuffer.reset();
    recordCommandBuffer(framebuffers[imageIndex], swapchain, swapchainExtent,
                        renderPass, pipeline, commandBuffer, vertexBuffer,
                        indexBuffer, pipelineLayout,
                        descriptorSets[imageIndex]);
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
