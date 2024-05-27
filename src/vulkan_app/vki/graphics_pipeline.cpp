#include "./graphics_pipeline.hpp"

#include <vulkan/vulkan_core.h>

#include "vulkan_app/vki/base.hpp"
#include "vulkan_app/vki/logical_device.hpp"
#include "vulkan_app/vki/pipeline_layout.hpp"
#include "vulkan_app/vki/render_pass.hpp"
#include "vulkan_app/vki/shader_module.hpp"

vki::GraphicsPipeline::GraphicsPipeline(
    const vki::ShaderModule &vertShader,
    const vki::ShaderModule &fragmentShader, VkExtent2D extent,
    const vki::PipelineLayout &pipelineLayout,
    const vki::RenderPass &renderPass, const vki::LogicalDevice &logicalDevice

    )
    : device{ logicalDevice.getVkDevice() } {
    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
    vertexShaderCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderCreateInfo.module = vertShader.getVkShaderModule();
    vertexShaderCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};
    fragmentShaderCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderCreateInfo.module = fragmentShader.getVkShaderModule();
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
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;

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
    pipelineInfo.layout = pipelineLayout.getVkPipelineLayout();
    pipelineInfo.renderPass = renderPass.getVkRenderPass();
    pipelineInfo.subpass = 0;

    VkResult result =
        vkCreateGraphicsPipelines(logicalDevice.getVkDevice(), VK_NULL_HANDLE,
                                  1, &pipelineInfo, nullptr, &vkPipeline);
    vki::assertSuccess(result, "vkCreateGraphicsPipelines");
};

VkPipeline vki::GraphicsPipeline::getVkPipeline() const { return vkPipeline; };

vki::GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(device, vkPipeline, nullptr);
};
