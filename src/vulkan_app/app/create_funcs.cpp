#include "./create_funcs.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "easylogging++.h"
#include "glfw_controller.hpp"
#include "vulkan_app/app/create_buffers.hpp"
#include "vulkan_app/app/image_loaders.hpp"
#include "vulkan_app/app/uniform_buffer_object.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/descriptor_pool.hpp"
#include "vulkan_app/vki/descriptor_set_layout.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/image.hpp"
#include "vulkan_app/vki/image_view.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/memory.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue.hpp"
#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/sampler.hpp"
#include "vulkan_app/vki/shader_module.hpp"
#include "vulkan_app/vki/structs.hpp"
#include "vulkan_app/vki/surface.hpp"
#include "vulkan_app/vki/swapchain.hpp"
#include "vulkan_app/vki/utils.hpp"

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

std::vector<VkDescriptorSet> createDescriptorSets(
    const vki::LogicalDevice &logicalDevice,
    const std::vector<vki::Buffer> &uniformBuffers,
    const vki::DescriptorPool &descriptorPool,
    const vki::DescriptorSetLayout &descriptorSetLayout,
    const vki::Sampler &textureSampler, const vki::ImageView &textureImageView,
    el::Logger &logger) {
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
    std::vector<VkWriteDescriptorSet> writeInfos(descriptorSets.size() * 2);
    VkDescriptorImageInfo imageInfo = {
        .sampler = textureSampler.getVkSampler(),
        .imageView = textureImageView.getVkImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
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
        VkWriteDescriptorSet imageDescriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };
        writeInfos[i] = descriptorWrite;
        writeInfos[descriptorSets.size() + i] = imageDescriptorWrite;
    };
    logicalDevice.updateWriteDescriptorSets(writeInfos);
    return descriptorSets;
};

vki::DescriptorPool createDescriptorPool(
    const vki::LogicalDevice &logicalDevice,
    const uint32_t &uniformBuffersCount) {
    VkDescriptorPoolSize poolSize = { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      .descriptorCount = uniformBuffersCount };
    VkDescriptorPoolSize samplerPoolSize = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = uniformBuffersCount
    };
    VkDescriptorPoolSize poolSizes[] = { poolSize, samplerPoolSize };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = uniformBuffersCount,
        .poolSizeCount = 2,
        .pPoolSizes = poolSizes,
    };
    return vki::DescriptorPool(logicalDevice, descriptorPoolCreateInfo);
};

vki::RenderPass createRenderPass(const vki::LogicalDevice &logicalDevice,
                                 const VkFormat &swapchainFormat,
                                 const VkFormat &depthFormat,
                                 const VkSampleCountFlagBits &sampleCount) {
    VkAttachmentDescription colorAttachment = {
        .format = swapchainFormat,
        .samples = sampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment = {
        .format = depthFormat,
        .samples = sampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription resolveAttachment = {
        .format = swapchainFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference resolveAttachmentRef = {
        .attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = &resolveAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef
    };
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };
    const vki::RenderPassCreateInfo renderPassCreateInfo = {
        .attachments = { colorAttachment, depthAttachment, resolveAttachment },
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
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding,
                                                samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = bindings
    };
    return vki::DescriptorSetLayout(logicalDevice,
                                    descriptorSetLayoutCreateInfo);
};

std::vector<vki::Framebuffer> createFramebuffers(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const VkExtent2D &swapchainExtent, const vki::RenderPass &renderPass,
    const vki::ImageView &depthImageView,
    const vki::ImageView &multisampleImageView) {
    return swapchain.swapChainImageViews |
           std::views::transform(
               [&swapchain, &swapchainExtent, &renderPass, &logicalDevice,
                &depthImageView, &multisampleImageView](const auto &imageView) {
                   std::array attachments = {
                       multisampleImageView.getVkImageView(),
                       depthImageView.getVkImageView(),
                       imageView.getVkImageView(),
                   };

                   VkFramebufferCreateInfo createInfo = {
                       .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                       .renderPass = renderPass.getVkRenderPass(),
                       .attachmentCount = attachments.size(),
                       .pAttachments = attachments.data(),
                       .width = swapchainExtent.width,
                       .height = swapchainExtent.height,
                       .layers = 1,
                   };
                   return vki::Framebuffer(logicalDevice, createInfo);
               }) |
           std::ranges::to<std::vector>();
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
            const auto &features = device.getFeatures();
            return hasNeccesaryQueueFamilies && hasNeccesaryExtensions &&
                   features.samplerAnisotropy;
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

vki::PresentMode choosePresentMode(
    const std::unordered_set<vki::PresentMode> &presentModes) {
    if (presentModes.contains(vki::PresentMode::MAILBOX_KHR))
        return vki::PresentMode::MAILBOX_KHR;
    return vki::PresentMode::FIFO_KHR;
};

vki::Sampler createTextureSampler(
    const vki::LogicalDevice &logicalDevice,
    const VkPhysicalDeviceProperties &deviceProperties) {
    VkSampler sampler;
    VkSamplerCreateInfo samplerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,

    };
    return vki::Sampler(logicalDevice, samplerCreateInfo);
};

std::tuple<vki::Image, vki::ImageView> createTextureImage(
    const vki::LogicalDevice &logicalDevice,
    const vki::CommandPool &commandPool,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::GraphicsQueueMixin &queue) {
    const auto &imageData = load_jpeg_image(std::filesystem::path("check.jpg"));
    const auto &mipLevels = static_cast<uint32_t>(std::floor(std::log2(
                                std::max(imageData.width, imageData.height)))) +
                            1;
    VkDeviceSize size = imageData.width * imageData.height * 4;
    const auto &stagingBuffer = createStagingBuffer(
        logicalDevice, memoryProperties, logger, size, imageData.buffer);
    VkImageCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent = { .width = imageData.width,
                    .height = imageData.height,
                    .depth = 1 },
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    auto image = vki::Image(logicalDevice, createInfo);
    const auto &imageMemoryRequirements = image.getMemoryRequirements();
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = imageMemoryRequirements.size,
        .memoryTypeIndex = vki::utils::findMemoryType(
            imageMemoryRequirements.memoryTypeBits, memoryProperties,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    image.bindMemory(vki::Memory(logicalDevice, allocInfo));

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image.getVkImage(),
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                              .baseMipLevel = 0,
                              .levelCount = mipLevels,
                              .baseArrayLayer = 0,
                              .layerCount = 1 }
    };

    const auto &commandBuffer = commandPool.createCommandBuffer();
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                              .mipLevel = 0,
                              .baseArrayLayer = 0,
                              .layerCount = 1 },
        .imageOffset = { 0, 0, 0 },
        .imageExtent = createInfo.extent
    };

    int32_t mipWidth = imageData.width;
    int32_t mipHeight = imageData.height;
    const auto &submitInfo = vki::SubmitInfo(
        (vki::SubmitInfoInputData){ .commandBuffers = { &commandBuffer } });
    commandBuffer.record([&]() {
        vkCmdPipelineBarrier(commandBuffer.getVkCommandBuffer(),
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &barrier);
        vkCmdCopyBufferToImage(commandBuffer.getVkCommandBuffer(),
                               stagingBuffer.getVkBuffer(), image.getVkImage(),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);
        barrier.subresourceRange.levelCount = 1;
        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer.getVkCommandBuffer(),
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                                 0, nullptr, 1, &barrier);
            VkImageBlit blit = {
                .srcSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .mipLevel = i - 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1 },
                .srcOffsets = { { 0, 0, 0 },
                                { .x = mipWidth, .y = mipHeight, .z = 1 } },
                .dstSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .mipLevel = i,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1 },
                .dstOffsets = { { 0, 0, 0 },
                                { .x = mipWidth > 1 ? (mipWidth / 2) : 1,
                                  .y = mipHeight > 1 ? (mipHeight / 2) : 1,
                                  .z = 1 } }
            };
            vkCmdBlitImage(
                commandBuffer.getVkCommandBuffer(), image.getVkImage(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.getVkImage(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer.getVkCommandBuffer(),
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        };
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        vkCmdPipelineBarrier(commandBuffer.getVkCommandBuffer(),
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &barrier);
    });

    queue.submit({ submitInfo }, std::nullopt);
    queue.waitIdle();

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.getVkImage(),
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = createInfo.format,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                              .baseMipLevel = 0,
                              .levelCount = mipLevels,
                              .baseArrayLayer = 0,
                              .layerCount = 1 }
    };
    auto imageView = vki::ImageView(logicalDevice, imageViewCreateInfo);
    return { std::move(image), std::move(imageView) };
};

std::tuple<vki::Image, vki::ImageView> createDepthImage(
    const vki::LogicalDevice &logicalDevice,
    const vki::CommandPool &commandPool, const VkFormat &depthFormat,
    const VkSampleCountFlagBits &sampleCount,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    const VkExtent2D &swapchainExtent, el::Logger &logger,
    const vki::GraphicsQueueMixin &queue) {
    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depthFormat,
        .extent = { .width = swapchainExtent.width,
                    .height = swapchainExtent.height,
                    .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = sampleCount,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    auto image = vki::Image(logicalDevice, imageCreateInfo);
    const auto &imageMemoryRequirements = image.getMemoryRequirements();
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = imageMemoryRequirements.size,
        .memoryTypeIndex = vki::utils::findMemoryType(
            imageMemoryRequirements.memoryTypeBits, memoryProperties,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    image.bindMemory(vki::Memory(logicalDevice, allocInfo));

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.getVkImage(),
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageCreateInfo.format,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                              .baseMipLevel = 0,
                              .levelCount = 1,
                              .baseArrayLayer = 0,
                              .layerCount = 1 }
    };
    auto imageView = vki::ImageView(logicalDevice, imageViewCreateInfo);
    return { std::move(image), std::move(imageView) };
};

VkFormat findDepthFormat(const vki::PhysicalDevice &physicalDevice) {
    const VkFormat formats[] = { VK_FORMAT_D32_SFLOAT,
                                 VK_FORMAT_D32_SFLOAT_S8_UINT,
                                 VK_FORMAT_D24_UNORM_S8_UINT };
    for (const auto &format : formats) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice.getVkDevice(),
                                            format, &properties);
        if (properties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        };
    };
    throw std::runtime_error("No format was found for depth attachment");
};

VkSampleCountFlagBits getMaxUsableSampleCount(
    const vki::PhysicalDevice &physicalDevice) {
    const auto &limits = physicalDevice.properties.limits;
    VkSampleCountFlags counts = limits.framebufferColorSampleCounts &
                                limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
};

std::tuple<vki::Image, vki::ImageView> createMultisampleImage(
    const vki::LogicalDevice &logicalDevice, const VkFormat &swapchainFormat,
    const VkExtent2D &swapchainExtent, const VkSampleCountFlagBits &sampleCount,
    const VkPhysicalDeviceMemoryProperties &memoryProperties,
    el::Logger &logger, const vki::GraphicsQueueMixin &queue) {
    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = swapchainFormat,
        .extent = { .width = swapchainExtent.width,
                    .height = swapchainExtent.height,
                    .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = sampleCount,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    auto image = vki::Image(logicalDevice, imageCreateInfo);
    const auto &imageMemoryRequirements = image.getMemoryRequirements();
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = imageMemoryRequirements.size,
        .memoryTypeIndex = vki::utils::findMemoryType(
            imageMemoryRequirements.memoryTypeBits, memoryProperties,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    image.bindMemory(vki::Memory(logicalDevice, allocInfo));
    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.getVkImage(),
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageCreateInfo.format,
        .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                              .baseMipLevel = 0,
                              .levelCount = 1,
                              .baseArrayLayer = 0,
                              .layerCount = 1 }
    };
    auto imageView = vki::ImageView(logicalDevice, imageViewCreateInfo);
    return { std::move(image), std::move(imageView) };
};
