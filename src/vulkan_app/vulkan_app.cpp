#include "./vulkan_app.hpp"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_metal.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"
#include "glfw_controller.hpp"

void VulkanApplication::createInstance(std::vector<const char *> extensions) {
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pNext = nullptr;

    const char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation",
    };
    createInfo = new VkInstanceCreateInfo();
    createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo->pApplicationInfo = &appInfo;
    createInfo->enabledExtensionCount = (uint32_t)extensions.size();
    createInfo->ppEnabledExtensionNames = extensions.data();
    createInfo->enabledLayerCount = 1;
    createInfo->ppEnabledLayerNames = validationLayers;
    createInfo->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    VkResult result = vkCreateInstance(createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "vkCreateInstance");
    };
};

void VulkanApplication::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find gpu with vulkan support");
    };
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        std::cout << std::format(
                         "Name: {}\nType: {}", properties.deviceName,
                         string_VkPhysicalDeviceType(properties.deviceType))
                  << std::endl;
    };
    physicalDevice = devices[0];
};

void VulkanApplication::pickQueueFamilies() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilies.data());
    uint32_t i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
        };
        VkBool32 presentSupport = false;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice, i, windowSurface, &presentSupport);
        if (result != VK_SUCCESS) {
            throw VulkanError(result, "vkGetPhysicalDeviceSurfaceSupportKHR");
        };
        if (presentSupport) {
            presentQueueIndex = i;
        };
        i++;
    }
    if (!graphicsQueueIndex.has_value()) {
        throw std::runtime_error(
            "Suitable queue graphics family was not found");
    };
};

void VulkanApplication::createLogicalDevice() {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
    graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsQueueCreateInfo.queueFamilyIndex = graphicsQueueIndex.value();
    graphicsQueueCreateInfo.queueCount = 1;
    graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfoArray;
    queueCreateInfoArray.push_back(graphicsQueueCreateInfo);
    if (presentQueueIndex != graphicsQueueIndex) {
        VkDeviceQueueCreateInfo presentQueueCreateInfo{};
        presentQueueCreateInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentQueueCreateInfo.queueFamilyIndex = presentQueueIndex.value();
        presentQueueCreateInfo.queueCount = 1;
        presentQueueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfoArray.push_back(presentQueueCreateInfo);
    };

    std::vector<const char *> deviceExtensions;
    deviceExtensions.push_back("VK_KHR_portability_subset");
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfoArray.size();
    createInfo.pQueueCreateInfos = queueCreateInfoArray.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult result =
        vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "vkCreateDevice");
    };
    vkGetDeviceQueue(device, graphicsQueueIndex.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueIndex.value(), 0, &presentQueue);
};

void VulkanApplication::createSwapChain(const GLFWControllerWindow &window) {
    auto details = queryDeviceSwapChainSupportDetails(physicalDevice);
    VkSurfaceFormatKHR format = chooseFormat(details);
    VkPresentModeKHR presentMode = choosePresentMode(details);
    swapChainExtent = chooseSwapExtent(details.capabilities, window);
    swapChainFormat = format.format;
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    };
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = windowSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (graphicsQueueIndex != presentQueueIndex) {
        std::vector<uint32_t> queueIndices;
        queueIndices.push_back(graphicsQueueIndex.value());
        queueIndices.push_back(presentQueueIndex.value());
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueIndices.data();
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    };
    VkResult result =
        vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
    assertSuccess(result, "vkCreateSwapchainKHR");

    uint32_t swapChainImageCount;
    result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount,
                                     nullptr);
    assertSuccess(result, "vkGetSwapchainImagesKHR");
    swapChainImages.resize(swapChainImageCount);
    result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount,
                                     swapChainImages.data());
    assertSuccess(result, "vkGetSwapchainImagesKHR");
};

void assertSuccess(const VkResult &result, const std::string message) {
    if (result != VK_SUCCESS) {
        throw VulkanError(result, message);
    };
};

SwapChainSupportDetails VulkanApplication::queryDeviceSwapChainSupportDetails(
    const VkPhysicalDevice &device) {
    SwapChainSupportDetails details;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, windowSurface, &details.capabilities);
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface,
                                                  &formatCount, nullptr);
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    details.formats.resize(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, windowSurface, &formatCount, details.formats.data());
    assertSuccess(result, "vkGetPhysicalDeviceSurfaceFormatsKHR");

    uint32_t modesCount;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface,
                                                       &modesCount, nullptr);
    assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    details.presentModes.resize(modesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, windowSurface, &modesCount, details.presentModes.data());
    assertSuccess(result, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    if (details.formats.empty() || details.presentModes.empty()) {
        throw std::runtime_error("Formats or presentModes are empty");
    };
    return details;
};

VkPresentModeKHR VulkanApplication::choosePresentMode(
    const SwapChainSupportDetails &details) {
    for (const auto &presentMode : details.presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        };
    };
    return VK_PRESENT_MODE_FIFO_KHR;
};

VkSurfaceFormatKHR VulkanApplication::chooseFormat(
    const SwapChainSupportDetails &details) {
    for (const auto &format : details.formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        };
    };
    throw std::runtime_error("Required surface format is not found");
};

VkExtent2D VulkanApplication::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities,
    const GLFWControllerWindow &window) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    };
    int width, height;
    glfwGetFramebufferSize(window.getGLFWWindow(), &width, &height);

    VkExtent2D actualExtent = { static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height) };
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
    return actualExtent;
};

void VulkanApplication::createWindowSurface(
    const GLFWControllerWindow &window) {
    VkResult result = glfwCreateWindowSurface(instance, window.getGLFWWindow(),
                                              nullptr, &windowSurface);
    if (result != VK_SUCCESS) {
        throw VulkanError(result, "glfwCreateWindowSurface");
    };
};

VulkanApplication::VulkanApplication(std::vector<const char *> extensions,
                                     const GLFWControllerWindow &window) {
    createInstance(std::move(extensions));
    createWindowSurface(window);
    pickPhysicalDevice();
    pickQueueFamilies();
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
    VkResult result = vkCreateCommandPool(device, &commandPoolCreateInfo,
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
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    assertSuccess(result, "vkAllocateCommandBuffers");
};

void VulkanApplication::recordCommandBuffer(uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    assertSuccess(result, "vkBeginCommandBuffer");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
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

    VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                                        &imageAvailableSemaphore);
    assertSuccess(result, "vkCreateSemaphore - imageAvailableSemaphore");
    result = vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                               &renderFinishedSemaphore);
    assertSuccess(result, "vkCreateSemaphore - renderFinishedSemaphore");
    result = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence);
    assertSuccess(result, "vkCreateFence");
};

void VulkanApplication::drawFrame() {
    VkResult result =
        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    assertSuccess(result, "vkWaitForFences");
    result = vkResetFences(device, 1, &inFlightFence);
    assertSuccess(result, "vkResetFences");

    uint32_t imageIndex;
    result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                          imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    assertSuccess(result, "vkAcquireNextImageKHR");
    result = vkResetCommandBuffer(commandBuffer, 0);
    assertSuccess(result, "vkResetCommandBuffer");
    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    assertSuccess(result, "vkQueueSubmit");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    assertSuccess(result, "vkQueuePresentKHR");
};

void VulkanApplication::createFramebuffers() {
    swapChainFrameBuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = { swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                                              &swapChainFrameBuffers[i]);
        assertSuccess(result, "vkCreateFramebuffer");
    };
};

void VulkanApplication::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    VkResult result =
        vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
    assertSuccess(result, "vkCreateRenderPass");
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
        vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
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

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo,
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
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                       nullptr, &graphicsPipeline);
    assertSuccess(result, "vkCreateGraphicsPipelines");
    vkDestroyShaderModule(device, vertShader, nullptr);
    vkDestroyShaderModule(device, fragmentShader, nullptr);
};

void VulkanApplication::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    int index = 0;
    for (const auto &image : swapChainImages) {
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
        VkResult result = vkCreateImageView(device, &createInfo, nullptr,
                                            &swapChainImageViews[index]);
        assertSuccess(result, "vkCreateImageView");
        index++;
    };
};

VulkanApplication::~VulkanApplication() {
    vkDeviceWaitIdle(device);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (const auto &framebuffer : swapChainFrameBuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    };
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for (const auto &imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    };
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, windowSurface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
};
