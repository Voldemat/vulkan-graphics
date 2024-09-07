#pragma once

#include <chrono>
#include <ratio>

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

struct FrameState {
    glm::mat4 projection;
    std::chrono::time_point<
        std::chrono::high_resolution_clock,
        std::chrono::duration<long long, std::ratio<1LL, 1000000000LL>>>
        timeOfLastFrame;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    double lastXPos;
    double lastYPos;
    float pitch;
    float yaw;
    bool firstMouse;
};
