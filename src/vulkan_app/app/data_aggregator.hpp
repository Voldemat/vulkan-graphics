#pragma once

#include <math.h>

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <span>
#include <vector>

#include "vulkan_app/app/vertex.hpp"
struct ShapeData {
    uint32_t vertexOffset;
    uint32_t indexOffset;
    uint32_t vertexCount;
    uint32_t indexCount;
};

class DataAggregator {
    void bind();

public:
    std::vector<Vertex> vertexArray;
    std::vector<uint32_t> indexArray;
    std::vector<ShapeData> shapes;

    inline const std::span<const Vertex> getVertices() const {
        return vertexArray;
    };

    inline const std::span<const uint32_t> getIndices() const {
        return indexArray;
    };

    inline const std::span<const ShapeData> getObjectsOffsets() const {
        return shapes;
    };
};

struct Triangle {
    std::array<Vertex, 3> vertices;
    std::array<uint32_t, 3> indices;
    uint32_t vertexOffset;
    uint32_t indexOffset;
    explicit Triangle(DataAggregator &aggregator,
                      const std::array<Vertex, 3> &vertices,
                      const std::array<uint32_t, 3> &indices)
        : vertices{ vertices }, indices{ indices } {
        for (const auto &[vertex, vIndex] :
             std::views::zip(vertices, indices)) {
            aggregator.vertexArray.push_back(vertex);
            aggregator.indexArray.push_back(vIndex);
        };
        vertexOffset = aggregator.vertexArray.size() - 3;
        indexOffset = aggregator.indexArray.size() - 3;
        aggregator.shapes.push_back((ShapeData){ .vertexOffset = vertexOffset,
                                                 .indexOffset = indexOffset,
                                                 .vertexCount = 3,
                                                 .indexCount = 3 });
    };

    void sync(DataAggregator &aggregator) const {
        memcpy(&aggregator.vertexArray[vertexOffset], vertices.data(),
               sizeof(vertices[0]) * vertices.size());
        memcpy(&aggregator.indexArray[indexOffset], indices.data(),
               sizeof(indices[0]) * indices.size());
    };
};

struct Circle {
    std::span<Vertex> vertices;
    std::span<uint32_t> indices;
    uint32_t vertexOffset;
    uint32_t indexOffset;
    uint32_t vertexCount;
    explicit Circle(DataAggregator &aggregator, const float radius,
                    const uint32_t rings, const uint32_t segments) {
        assert(rings > 1);
        assert(segments > 2);
        const float deltaRingAngle = static_cast<float>(M_PI / rings);
        const float deltaSegAngle = static_cast<float>(2.0 * M_PI / segments);
        uint32_t verticeIndex = 0;

        // Generate the group of rings for the sphere
        for (uint32_t ringIdx = 0; ringIdx <= rings; ++ringIdx) {
            float r0 = radius * std::sin(ringIdx * deltaRingAngle);
            float y0 = radius * std::cos(ringIdx * deltaRingAngle);
            // Generate the group of segments for the current ring
            for (uint32_t segIdx = 0; segIdx <= segments; ++segIdx) {
                float x0 = r0 * std::sin(segIdx * deltaSegAngle);
                float z0 = r0 * std::cos(segIdx * deltaSegAngle);

                aggregator.vertexArray.emplace_back(
                    (Vertex){ .pos = { x0, y0, z0 }, .color = { 1, 1, 1 } });

                if (ringIdx != rings) {
                    // each vertex (except the last) has six indicies pointing
                    // to it
                    aggregator.indexArray.push_back(verticeIndex + segments +
                                                    1);
                    aggregator.indexArray.push_back(verticeIndex);
                    aggregator.indexArray.push_back(verticeIndex + segments);
                    aggregator.indexArray.push_back(verticeIndex + segments +
                                                    1);
                    aggregator.indexArray.push_back(verticeIndex + 1);
                    aggregator.indexArray.push_back(verticeIndex);
                    ++verticeIndex;
                }
            }
        }
        vertexOffset = aggregator.vertexArray.size() - verticeIndex - 1;
        indexOffset = aggregator.indexArray.size() - verticeIndex * 6;
        vertices = std::span(aggregator.vertexArray)
                       .subspan(vertexOffset, verticeIndex + 1);
        indices = std::span(aggregator.indexArray)
                      .subspan(indexOffset, verticeIndex * 6);
        aggregator.shapes.push_back((ShapeData){
            .vertexOffset = vertexOffset,
            .indexOffset = indexOffset,
            .vertexCount = verticeIndex + 1,
            .indexCount = verticeIndex + 1 });
    }

    void sync(DataAggregator &aggregator) const {
        memcpy(&aggregator.vertexArray[vertexOffset], vertices.data(),
               sizeof(vertices[0]) * vertices.size());
        memcpy(&aggregator.indexArray[indexOffset], indices.data(),
               sizeof(indices[0]) * indices.size());
    };
};
