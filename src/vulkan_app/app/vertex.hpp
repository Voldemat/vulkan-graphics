#pragma once

#include <vulkan/vulkan_core.h>

#include <array>
#include <cstddef>

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        return { .binding = 0,
                 .stride = sizeof(Vertex),
                 .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
    };

    static std::array<VkVertexInputAttributeDescription, 3>
    getAttributeDescriptions() {
        return {
            (VkVertexInputAttributeDescription){
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, pos) },
            (VkVertexInputAttributeDescription){
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color) },
            (VkVertexInputAttributeDescription){
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord) },
        };
    };
};
