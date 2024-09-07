#include "./app.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "GLFW/glfw3.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "vulkan_app/app/frame_state.hpp"

// clang-format off
#define ELPP_STL_LOGGING
#include "easylogging++.h"
// clang-format on

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "./glfw_controller.hpp"
#include "magic_enum.hpp"
#include "vulkan_app/app/create_buffers.hpp"
#include "vulkan_app/app/create_funcs.hpp"
#include "vulkan_app/app/create_pipeline.hpp"
#include "vulkan_app/app/draw_frame.hpp"
#include "vulkan_app/app/vertex.hpp"
#include "vulkan_app/vki/buffer.hpp"
#include "vulkan_app/vki/command_pool.hpp"
#include "vulkan_app/vki/fence.hpp"
#include "vulkan_app/vki/framebuffer.hpp"
#include "vulkan_app/vki/graphics_pipeline.hpp"
#include "vulkan_app/vki/instance.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/memory.hpp"
#include "vulkan_app/vki/physical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/queue_family.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/semaphore.hpp"
#include "vulkan_app/vki/shader_module.hpp"
#include "vulkan_app/vki/structs.hpp"
#include "vulkan_app/vki/surface.hpp"
#include "vulkan_app/vki/swapchain.hpp"

const std::vector<Vertex> vertices = { { .pos = { -0.5f, -0.5f, 0.0f },
                                         .color = { 1.0f, 0.0f, 0.0f },
                                         .texCoord = { 1.0f, 0.0f } },
                                       { .pos = { 0.5f, -0.5f, 0.0f },
                                         .color = { 1.0f, 1.0f, 0.0f },
                                         .texCoord = { 0.0f, 0.0f } },
                                       { .pos = { 0.5f, 0.5f, 0.0f },
                                         .color = { 0.0f, 0.0f, 1.0f },
                                         .texCoord = { 0.0f, 1.0f } },
                                       { .pos = { -0.5f, 0.5f, 0.0f },
                                         .color = { 0.0f, 0.0f, 1.0f },
                                         .texCoord = { 1.0f, 1.0f } },

                                       { .pos = { -0.5f, -0.5f, -0.5f },
                                         .color = { 1.0f, 0.0f, 0.0f },
                                         .texCoord = { 0.0f, 0.0f } },
                                       { .pos = { 0.5f, -0.5f, -0.5f },
                                         .color = { 0.0f, 1.0f, 0.0f },
                                         .texCoord = { 1.0f, 0.0f } },
                                       { .pos = { 0.5f, 0.5f, -0.5f },
                                         .color = { 0.0f, 0.0f, 1.0f },
                                         .texCoord = { 1.0f, 1.0f } },
                                       { .pos = { -0.5f, 0.5f, -0.5f },
                                         .color = { 1.0f, 1.0f, 1.0f },
                                         .texCoord = { 0.0f, 1.0f } } };

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };
void processInput(const GLFWControllerWindow &window, FrameState &state,
                  float speedConf) {
    const auto &now = std::chrono::high_resolution_clock::now();
    float deltaTime = (now - state.timeOfLastFrame).count();
    float speed = speedConf * deltaTime;
    if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
        state.cameraPos += speed * state.cameraFront;
    if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
        state.cameraPos -= speed * state.cameraFront;
    if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
        state.cameraPos -=
            glm::normalize(glm::cross(state.cameraFront, state.cameraUp)) *
            speed;
    if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
        state.cameraPos +=
            glm::normalize(glm::cross(state.cameraFront, state.cameraUp)) *
            speed;
    double xpos, ypos;
    glfwGetCursorPos(window.getGLFWWindow(), &xpos, &ypos);
    if (state.firstMouse) {
        state.firstMouse = false;
        state.lastXPos = xpos;
        state.lastYPos = ypos;
    };
    double xoffset = (xpos - state.lastXPos) * 0.1f;
    double yoffset = (ypos - state.lastYPos) * 0.1f;
    state.lastXPos = xpos;
    state.lastYPos = ypos;
    state.yaw += xoffset;
    state.pitch += yoffset;
    if (state.pitch > 89.0f) state.pitch = 89.0f;
    if (state.pitch < -89.0f) state.pitch = -89.0f;
    glm::vec3 direction;
    direction.x = -cos(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    direction.y = sin(glm::radians(state.pitch));
    direction.z = sin(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    state.cameraFront = glm::normalize(direction);
};

void run_app() {
    auto &mainLogger = *el::Loggers::getLogger("main");
    GLFWController controller;
    mainLogger.info("Created GLFWController");

    bool shouldClose = false;
    GLFWControllerWindow window =
        controller.createWindow("Hello triangle", 800, 600, true);
    glfwSetInputMode(window.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    window.registerKeyCallback([&shouldClose](int key, int scancode, int action, int mods){
        if (key == GLFW_KEY_W && mods == 8) {
            shouldClose = true;
        };
    });
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
        vki::LogicalDevice(physicalDevice, physicalDevice.getFeatures(),
                           std::make_tuple(queueCreateInfo));
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

    const auto &depthFormat = findDepthFormat(physicalDevice);
    const auto &sampleCount = getMaxUsableSampleCount(physicalDevice);
    const auto &renderPass = createRenderPass(
        logicalDevice, swapchainFormat.format, depthFormat, sampleCount);
    mainLogger.info("Created render pass");

    const auto &descriptorSetLayout = createDescriptorSetLayout(logicalDevice);
    mainLogger.info("Created descriptor set layout");

    const auto &pipelineLayout =
        createPipelineLayout(logicalDevice, descriptorSetLayout);
    mainLogger.info("Created pipeline layout");

    const auto &pipeline =
        createGraphicsPipeline(logicalDevice, mainLogger, swapchainExtent,
                               renderPass, pipelineLayout, sampleCount);
    mainLogger.info("Created pipeline");

    const auto &commandPool = vki::CommandPool(logicalDevice, queueFamily);
    mainLogger.info("Created command pool");

    const auto &memoryProperties = physicalDevice.getMemoryProperties();

    const auto &[multisampleImage, multisampleImageView] =
        createMultisampleImage(logicalDevice, swapchainFormat.format,
                               swapchainExtent, sampleCount, memoryProperties,
                               mainLogger, queue);
    const auto &[depthImage, depthImageView] =
        createDepthImage(logicalDevice, commandPool, depthFormat, sampleCount,
                         memoryProperties, swapchainExtent, mainLogger, queue);

    const auto &framebuffers =
        createFramebuffers(logicalDevice, swapchain, swapchainExtent,
                           renderPass, depthImageView, multisampleImageView);
    mainLogger.info("Created framebuffers");

    const auto &[vertexBuffer, indexBuffer] = createVertexAndIndicesBuffer(
        logicalDevice, memoryProperties, mainLogger, commandPool, queue,
        vertices, indices);
    mainLogger.info("Created index and vertex buffers");
    const auto &[uniformBuffers, uniformMappedMemory] =
        createUniformBuffers(logicalDevice, memoryProperties, mainLogger,
                             swapchain.swapChainImageViews.size());
    mainLogger.info("Created uniform buffers");

    const auto &descriptorPool =
        createDescriptorPool(logicalDevice, uniformBuffers.size());
    mainLogger.info("Created descriptor pool");

    const auto &[textureImage, textureImageView] = createTextureImage(
        logicalDevice, commandPool, memoryProperties, mainLogger, queue);
    const auto &textureSampler =
        createTextureSampler(logicalDevice, physicalDevice.getProperties());

    const auto &descriptorSets = createDescriptorSets(
        logicalDevice, uniformBuffers, descriptorPool, descriptorSetLayout,
        textureSampler, textureImageView, mainLogger);
    mainLogger.info("Created descriptor sets");

    const auto &commandBuffer = commandPool.createCommandBuffer();
    mainLogger.info("Created command buffer");

    const auto &imageAvailableSemaphore = vki::Semaphore(logicalDevice);
    const auto &renderFinishedSemaphore = vki::Semaphore(logicalDevice);
    const auto &inFlightFence = vki::Fence(logicalDevice, true);
    mainLogger.info("Created semaphores and fences");

    FrameState frameState = {
        .projection = glm::perspective(
            glm::radians(45.0f),
            swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f),
        .timeOfLastFrame = std::chrono::high_resolution_clock::now(),
        .cameraPos = glm::vec3(0.0f, 0.0f, 3.0f),
        .cameraFront = glm::vec3(0.0f, 0.0f, -1.0f),
        .cameraUp = glm::vec3(0.0f, -1.0f, 0.0f),
        .pitch = 0.0f,
        .yaw = -90.0f,
        .firstMouse = true
    };
    frameState.projection[1][1] *= -1;
    const auto &speedConf = 0.000000004f;
    float lastFrame = 0.0f;
    mainLogger.info("Entering main loop...");
    while (!(window.shouldClose() || shouldClose)) {
        controller.pollEvents();
        processInput(window, frameState, speedConf);
        frameState.timeOfLastFrame = std::chrono::high_resolution_clock::now();
        drawFrame(logicalDevice, swapchain, swapchainExtent, renderPass,
                  pipeline, framebuffers, commandBuffer, inFlightFence,
                  imageAvailableSemaphore, renderFinishedSemaphore,
                  vertexBuffer, indexBuffer, queue, queue, uniformMappedMemory,
                  pipelineLayout, descriptorSets, indices.size(), frameState);
    };

    mainLogger.info("Waiting for queued operations to complete...");
    logicalDevice.waitIdle();
};
