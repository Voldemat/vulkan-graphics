#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <tuple>
#include <vector>

#include "easylogging++.h"
#include "vulkan_app/app/vertex.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/queue.hpp"

std::tuple<vki::Buffer, vki::Buffer> createVertexAndIndicesBuffer(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::CommandPool &commandPool,
    const vki::GraphicsQueueMixin &graphicsQueue,
    const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);

std::tuple<std::vector<vki::Buffer>, std::vector<void *>> createUniformBuffers(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const unsigned int buffersCount);
