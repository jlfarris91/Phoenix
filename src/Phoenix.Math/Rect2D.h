#pragma once

#include <glm/glm.hpp>

namespace Phoenix::Math
{
    struct Rect2D
    {
        glm::vec2 Min;
        glm::vec2 Max;

        float GetWidth() const;

        float GetHeight() const;
    };
}