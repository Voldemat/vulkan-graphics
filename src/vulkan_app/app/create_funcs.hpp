#pragma once

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "easylogging++.h"
#include "glfw_controller.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/descriptor_pool.hpp"
#include "vulkan_app/vki/descriptor_set_layout.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/image.hpp"
#include "vulkan_app/vki/image_view.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/sampler.hpp"
#include "vulkan_app/vki/surface.hpp"
#include "vulkan_app/vki/swapchain.hpp"

VkSurfaceFormatKHR chooseFormat(const vki::SurfaceFormatSet &formats);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                            const GLFWControllerWindow &window);

vki::PresentMode choosePresentMode(
    const std::unordered_set<vki::PresentMode> &presentModes);

std::vector<VkDescriptorSet> createDescriptorSets(
    const vki::LogicalDevice &logicalDevice,
    const std::vector<vki::Buffer> &uniformBuffers,
    const vki::DescriptorPool &descriptorPool,
    const vki::DescriptorSetLayout &descriptorSetLayout,
    const vki::Sampler &textureSampler, const vki::ImageView &textureImageView,
    el::Logger &logger);

vki::DescriptorPool createDescriptorPool(
    const vki::LogicalDevice &logicalDevice,
    const uint32_t &uniformBuffersCount);

vki::RenderPass createRenderPass(const vki::LogicalDevice &logicalDevice,
                                 const VkFormat &swapchainFormat,
                                 const VkFormat &depthFormat,
                                 const VkSampleCountFlagBits &sampleCount);

vki::DescriptorSetLayout createDescriptorSetLayout(
    const vki::LogicalDevice &logicalDevice);

std::vector<vki::Framebuffer> createFramebuffers(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass,
    const vki::ImageView &depthImageView,
    const vki::ImageView &multisampleImageView);

vki::PhysicalDevice pickPhysicalDevice(const vki::VulkanInstance &instance,
                                       const vki::Surface &surface,
                                       el::Logger &logger);

vki::QueueFamilyWithOp<1, vki::QueueOperationType::GRAPHIC,
                       vki::QueueOperationType::PRESENT>
pickQueueFamily(const std::vector<vki::QueueFamily> &families);

vki::Sampler createTextureSampler(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceProperties &deviceProperties);

std::tuple<vki::Image, vki::ImageView> createTextureImage(
    const vki::LogicalDevice &logicalDevice,
    const vki::CommandPool &commandPool,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::GraphicsQueueMixin &queue);

std::tuple<vki::Image, vki::ImageView> createDepthImage(
    const vki::LogicalDevice &logicalDevice,
    const vki::CommandPool &commandPool, const VkFormat &depthFormat,
    const VkSampleCountFlagBits &sampleCount,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    const VkExtent2D &swapchainExtent, el::Logger &logger,
    const vki::GraphicsQueueMixin &queue);

VkFormat findDepthFormat(const vki::PhysicalDevice &physicalDevice);

VkSampleCountFlagBits getMaxUsableSampleCount(
    const vki::PhysicalDevice &physicalDevice);

std::tuple<vki::Image, vki::ImageView> createMultisampleImage(
    const vki::LogicalDevice &logicalDevice, const VkFormat &swapchainFormat,
    const VkExtent2D &swapchainExtent, const VkSampleCountFlagBits &sampleCount,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::GraphicsQueueMixin &queue);
