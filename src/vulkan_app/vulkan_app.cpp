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
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "glfw_controller.hpp"
#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/command_buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/fence.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/shader_module.hpp"
#include "vulkan_app/vki/swapchain.hpp"

using namespace vki;

const vki::PhysicalDevice VulkanApplication::pickPhysicalDevice() {
    const auto &devices = instance.getPhysicalDevices();
    const auto &it = std::ranges::find_if(
        devices,
        [](const vki::PhysicalDevice &device) { return device.isSuitable(); });
    if (it == devices.end()) {
        throw std::runtime_error("No suitable physical devices present");
    };
    return *it;
};

const vki::Swapchain VulkanApplication::createSwapChain(
    const vki::PhysicalDevice &physicalDevice,
    const vki::LogicalDevice &logicalDevice,
    const GLFWControllerWindow &window) {
    auto details = physicalDevice.getSwapchainDetails(instance.getSurface());
    swapChainFormat = details.chooseFormat().format;
    swapChainExtent = details.chooseSwapExtent(window);
    return vki::Swapchain(logicalDevice, physicalDevice, instance.getSurface(),
                          window);
};

VulkanApplication::VulkanApplication(vki::VulkanInstanceParams params,
                                     const GLFWController &controller,
                                     const GLFWControllerWindow &window)
    : instance{ params, window } {
    const vki::PhysicalDevice &physicalDevice = pickPhysicalDevice();
    const auto logicalDevice = vki::LogicalDevice(physicalDevice);
    const vki::Swapchain &swapchain =
        createSwapChain(physicalDevice, logicalDevice, window);
    const auto renderPass = vki::RenderPass(swapChainFormat, logicalDevice);
    const auto pipelineLayout = vki::PipelineLayout(logicalDevice);
    const auto pipeline =
        createGraphicsPipeline(logicalDevice, renderPass, pipelineLayout);
    const auto framebuffers =
        swapchain.swapChainImageViews |
        std::views::transform([&swapchain, &renderPass, &logicalDevice,
                               this](const auto &imageView) {
            return std::make_shared<vki::Framebuffer>(swapchain, renderPass,
                                                      swapChainExtent,
                                                      logicalDevice, imageView);
        }) |
        std::ranges::to<std::vector>();
    const auto &commandPool = vki::CommandPool(logicalDevice, physicalDevice);
    const auto &commandBuffer = commandPool.createCommandBuffer();
    const auto &imageAvailableSemaphore = vki::Semaphore(logicalDevice);
    const auto &renderFinishedSemaphore = vki::Semaphore(logicalDevice);
    const auto &inFlightFence = vki::Fence(logicalDevice);

    while (!window.shouldClose()) {
        controller.pollEvents();
        drawFrame(logicalDevice, swapchain, renderPass, pipeline, framebuffers,
                  commandBuffer, inFlightFence, imageAvailableSemaphore,
                  renderFinishedSemaphore);
    };

    logicalDevice.waitIdle();
};

void VulkanApplication::recordCommandBuffer(
    const std::shared_ptr<vki::Framebuffer> &framebuffer,
    const vki::RenderPass &renderPass, const vki::GraphicsPipeline &pipeline,
    const vki::CommandBuffer &commandBuffer) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result =
        vkBeginCommandBuffer(commandBuffer.getVkCommandBuffer(), &beginInfo);
    assertSuccess(result, "vkBeginCommandBuffer");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.getVkRenderPass();
    renderPassInfo.framebuffer = framebuffer->getVkFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer.getVkCommandBuffer(), &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer.getVkCommandBuffer(),
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline.getVkPipeline());
    vkCmdDraw(commandBuffer.getVkCommandBuffer(), 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer.getVkCommandBuffer());
    result = vkEndCommandBuffer(commandBuffer.getVkCommandBuffer());
    assertSuccess(result, "vkEndCommandBuffer");
};

void VulkanApplication::drawFrame(
    const vki::LogicalDevice &logicalDevice, const vki::Swapchain &swapchain,
    const vki::RenderPass &renderPass, const vki::GraphicsPipeline &pipeline,
    const std::vector<std::shared_ptr<vki::Framebuffer>> &framebuffers,
    const vki::CommandBuffer &commandBuffer, const vki::Fence &inFlightFence,
    const vki::Semaphore &imageAvailableSemaphore,
    const vki::Semaphore &renderFinishedSemaphore) {
    inFlightFence.wait();

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        logicalDevice.getVkDevice(), swapchain.getVkSwapchain(), UINT64_MAX,
        imageAvailableSemaphore.getVkSemaphore(), VK_NULL_HANDLE, &imageIndex);
    assertSuccess(result, "vkAcquireNextImageKHR");
    result = vkResetCommandBuffer(commandBuffer.getVkCommandBuffer(), 0);
    assertSuccess(result, "vkResetCommandBuffer");
    recordCommandBuffer(framebuffers[imageIndex], renderPass, pipeline,
                        commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore.getVkSemaphore() };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkCommandBuffer buffer = commandBuffer.getVkCommandBuffer();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    VkSemaphore signalSemaphores[] = {
        renderFinishedSemaphore.getVkSemaphore()
    };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(logicalDevice.graphicsQueue, 1, &submitInfo,
                           inFlightFence.getVkFence());
    assertSuccess(result, "vkQueueSubmit");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapchain.getVkSwapchain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(logicalDevice.presentQueue, &presentInfo);
    assertSuccess(result, "vkQueuePresentKHR");
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

vki::GraphicsPipeline VulkanApplication::createGraphicsPipeline(
    const vki::LogicalDevice &logicalDevice, const vki::RenderPass &renderPass,
    const vki::PipelineLayout &pipelineLayout) {
    auto vertShaderCode = readFile("../src/shaders/vertex.spv");
    auto fragmentShaderCode = readFile("../src/shaders/fragment.spv");
    auto vertShader = vki::ShaderModule(logicalDevice, vertShaderCode);
    auto fragmentShader = vki::ShaderModule(logicalDevice, fragmentShaderCode);
    return vki::GraphicsPipeline(vertShader, fragmentShader, swapChainExtent,
                                 pipelineLayout, renderPass, logicalDevice);
};
