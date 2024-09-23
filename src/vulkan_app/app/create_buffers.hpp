#pragma once

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <span>
#include <tuple>
#include <vector>

#include "easylogging++.h"
#include "vulkan_app/app/vertex.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/queue.hpp"

vki::Buffer createStagingBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const std::size_t &size, void *data);
std::tuple<vki::Buffer, vki::Buffer> createVertexAndIndicesBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::CommandPool &commandPool,
    const vki::GraphicsQueueMixin &graphicsQueue,
    const std::span<const Vertex> &vertices,
    const std::span<const unsigned int> &indices);

std::tuple<std::vector<vki::Buffer>, std::vector<void *>> createUniformBuffers(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const unsigned int buffersCount);
