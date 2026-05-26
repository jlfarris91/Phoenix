#pragma once

#include <glm/glm.hpp>

namespace Phoenix::Math
{
    struct Transform2D
    {
        glm::vec2 Position;
        glm::vec2 Scale;
        float Rotation = 0;
    };
}