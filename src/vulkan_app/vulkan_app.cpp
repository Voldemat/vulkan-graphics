#include "./vulkan_app.hpp"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_metal.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "glfw_controller.hpp"
#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/swapchain.hpp"

using namespace vki;

void VulkanApplication::pickPhysicalDevice() {
    const auto &devices = instance.getPhysicalDevices();
    const auto &it = std::ranges::find_if(
        devices, [](const vki::PhysicalDevice &device) { return device.isSuitable(); });
    if (it == devices.end()) {
        throw std::runtime_error("No suitable physical devices present");
    };
    physicalDevice = *it;
    graphicsQueueIndex =
        physicalDevice->getFamilyTypeIndex(vki::QueueFamilyType::GRAPHIC);
    presentQueueIndex =
        physicalDevice->getFamilyTypeIndex(vki::QueueFamilyType::PRESENT);
};

void VulkanApplication::createLogicalDevice() {
    device = std::make_unique<vki::LogicalDevice>(physicalDevice.value());
};

void VulkanApplication::createSwapChain(const GLFWControllerWindow &window) {
    auto details = physicalDevice->getSwapchainDetails(instance.getSurface());
    swapChainFormat = details.chooseFormat().format;
    swapChainExtent = details.chooseSwapExtent(window);
    swapchain = std::make_unique<vki::Swapchain>(
        *(device.value().get()),
        physicalDevice.value(),
        instance.getSurface(),
        window
    );
};


VulkanApplication::VulkanApplication(vki::VulkanInstanceParams params,
                                     const GLFWControllerWindow &window)
    : instance{ params, window } {
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain(window);
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
};

void VulkanApplication::createCommandPool() {
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags =
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex.value();
    VkResult result = vkCreateCommandPool(device.value()->getVkDevice(), &commandPoolCreateInfo,
                                          nullptr, &commandPool);
    assertSuccess(result, "vkCreateCommandPool");
};

void VulkanApplication::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result =
        vkAllocateCommandBuffers(device.value()->getVkDevice(), &allocInfo, &commandBuffer);
    assertSuccess(result, "vkAllocateCommandBuffers");
};

void VulkanApplication::recordCommandBuffer(uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    assertSuccess(result, "vkBeginCommandBuffer");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.value()->getVkRenderPass();
    renderPassInfo.framebuffer = swapChainFrameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    result = vkEndCommandBuffer(commandBuffer);
    assertSuccess(result, "vkEndCommandBuffer");
};

void VulkanApplication::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result = vkCreateSemaphore(device.value()->getVkDevice(), &semaphoreInfo, nullptr,
                                        &imageAvailableSemaphore);
    assertSuccess(result, "vkCreateSemaphore - imageAvailableSemaphore");
    result = vkCreateSemaphore(device.value()->getVkDevice(), &semaphoreInfo, nullptr,
                               &renderFinishedSemaphore);
    assertSuccess(result, "vkCreateSemaphore - renderFinishedSemaphore");
    result = vkCreateFence(device.value()->getVkDevice(), &fenceInfo, nullptr, &inFlightFence);
    assertSuccess(result, "vkCreateFence");
};

void VulkanApplication::drawFrame() {
    VkResult result =
        vkWaitForFences(device.value()->getVkDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    assertSuccess(result, "vkWaitForFences");
    result = vkResetFences(device.value()->getVkDevice(), 1, &inFlightFence);
    assertSuccess(result, "vkResetFences");

    uint32_t imageIndex;
    result = vkAcquireNextImageKHR(device.value()->getVkDevice(), swapchain.value()->getVkSwapchain(), UINT64_MAX,
                                   imageAvailableSemaphore, VK_NULL_HANDLE,
                                   &imageIndex);
    assertSuccess(result, "vkAcquireNextImageKHR");
    result = vkResetCommandBuffer(commandBuffer, 0);
    assertSuccess(result, "vkResetCommandBuffer");
    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(device.value()->graphicsQueue, 1, &submitInfo, inFlightFence);
    assertSuccess(result, "vkQueueSubmit");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapchain.value() -> getVkSwapchain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(device.value()->presentQueue, &presentInfo);
    assertSuccess(result, "vkQueuePresentKHR");
};

void VulkanApplication::createFramebuffers() {
    swapChainFrameBuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = { swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.value()->getVkRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(device.value()->getVkDevice(), &framebufferInfo, nullptr,
                                              &swapChainFrameBuffers[i]);
        assertSuccess(result, "vkCreateFramebuffer");
    };
};

void VulkanApplication::createRenderPass() {
    renderPass = std::make_unique<vki::RenderPass>(swapChainFormat, *device.value().get());
};

static std::vector<char> readFile(const std::string &filename) {
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

VkShaderModule VulkanApplication::createShaderModule(
    const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    createInfo.codeSize = code.size();
    VkShaderModule shaderModule;
    VkResult result =
        vkCreateShaderModule(device.value()->getVkDevice(), &createInfo, nullptr, &shaderModule);
    assertSuccess(result, "vkCreateShaderModule");
    return shaderModule;
};

void VulkanApplication::createGraphicsPipeline() {
    auto vertShaderCode = readFile("../src/shaders/vertex.spv");
    auto fragmentShaderCode = readFile("../src/shaders/fragment.spv");
    auto vertShader = createShaderModule(vertShaderCode);
    auto fragmentShader = createShaderModule(fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
    vertexShaderCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderCreateInfo.module = vertShader;
    vertexShaderCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};
    fragmentShaderCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderCreateInfo.module = fragmentShader;
    fragmentShaderCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertexShaderCreateInfo, fragmentShaderCreateInfo
    };

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyCreateInfo{};
    pipelineInputAssemblyCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssemblyCreateInfo.topology =
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportCreateInfo{};
    viewportCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pViewports = &viewport;
    viewportCreateInfo.scissorCount = 1;
    viewportCreateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.minSampleShading = 1.0f;
    multisample.pSampleMask = nullptr;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult result = vkCreatePipelineLayout(device.value()->getVkDevice(), &pipelineLayoutCreateInfo,
                                             nullptr, &pipelineLayout);
    assertSuccess(result, "vkCreatePipelineLayout");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineInfo.pInputAssemblyState = &pipelineInputAssemblyCreateInfo;
    pipelineInfo.pViewportState = &viewportCreateInfo;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisample;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass.value()->getVkRenderPass();
    pipelineInfo.subpass = 0;

    result = vkCreateGraphicsPipelines(device.value()->getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
                                       nullptr, &graphicsPipeline);
    assertSuccess(result, "vkCreateGraphicsPipelines");
    vkDestroyShaderModule(device.value()->getVkDevice(), vertShader, nullptr);
    vkDestroyShaderModule(device.value()->getVkDevice(), fragmentShader, nullptr);
};

void VulkanApplication::createImageViews() {
    swapChainImageViews.resize(swapchain.value()->swapChainImages.size());
    int index = 0;
    for (const auto &image : swapchain.value()->swapChainImages) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        VkResult result = vkCreateImageView(device.value()->getVkDevice(), &createInfo, nullptr,
                                            &swapChainImageViews[index]);
        assertSuccess(result, "vkCreateImageView");
        index++;
    };
};

VulkanApplication::~VulkanApplication() {
    vkDeviceWaitIdle(device.value()->getVkDevice());
    vkDestroySemaphore(device.value()->getVkDevice(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device.value()->getVkDevice(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(device.value()->getVkDevice(), inFlightFence, nullptr);
    vkDestroyCommandPool(device.value()->getVkDevice(), commandPool, nullptr);
    for (const auto &framebuffer : swapChainFrameBuffers) {
        vkDestroyFramebuffer(device.value()->getVkDevice(), framebuffer, nullptr);
    };
    vkDestroyPipeline(device.value()->getVkDevice(), graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device.value()->getVkDevice(), pipelineLayout, nullptr);
    for (const auto &imageView : swapChainImageViews) {
        vkDestroyImageView(device.value()->getVkDevice(), imageView, nullptr);
    };
};
