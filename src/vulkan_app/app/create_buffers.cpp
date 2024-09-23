#include "./create_buffers.hpp"

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstring>
#include <format>
#include <optional>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

#include "easylogging++.h"
#include "vulkan_app/app/uniform_buffer_object.hpp"
#include "vulkan_app/app/vertex.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/memory.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/shader_module.hpp"
#include "vulkan_app/vki/structs.hpp"
#include "vulkan_app/vki/swapchain.hpp"
#include "vulkan_app/vki/utils.hpp"

vki::Buffer createVertexBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::Buffer &stagingBuffer,
    const vki::CommandBuffer &commandBuffer,
    const vki::GraphicsQueueMixin &graphicsQueue,
    const std::size_t &vertexContainerSize) {
    VkBufferCreateInfo vertexBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vertexContainerSize,
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
    const vki::GraphicsQueueMixin &graphicsQueue,
    const std::size_t &indicesContainerSize) {
    VkBufferCreateInfo indicesBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = indicesContainerSize,
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

std::tuple<vki::Buffer, vki::Buffer> createVertexAndIndicesBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::CommandPool &commandPool,
    const vki::GraphicsQueueMixin &graphicsQueue,
    const std::span<const Vertex> &vertices,
    const std::span<const unsigned int> &indices) {
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
    auto vertexBuffer = createVertexBuffer(
        logicalDevice, memoryProperties, logger, vertexStagingBuffer,
        commandBuffer, graphicsQueue, verticesSize);
    auto indicesBuffer = createIndexBuffer(
        logicalDevice, memoryProperties, logger, indexStagingBuffer,
        commandBuffer, graphicsQueue, indicesSize);
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
