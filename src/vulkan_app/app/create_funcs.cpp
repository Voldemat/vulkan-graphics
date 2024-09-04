#include "./create_funcs.hpp"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "easylogging++.h"
#include "glfw_controller.hpp"
#include "vulkan_app/app/uniform_buffer_object.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/descriptor_pool.hpp"
#include "vulkan_app/vki/descriptor_set_layout.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/memory.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/shader_module.hpp"
#include "vulkan_app/vki/surface.hpp"
#include "vulkan_app/vki/swapchain.hpp"

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

vki::PresentMode choosePresentMode(
    const std::unordered_set<vki::PresentMode> &presentModes) {
    if (presentModes.contains(vki::PresentMode::MAILBOX_KHR))
        return vki::PresentMode::MAILBOX_KHR;
    return vki::PresentMode::IMMEDIATE_KHR;
};
