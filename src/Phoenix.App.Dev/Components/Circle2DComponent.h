#pragma once

#include <glm/glm.hpp>

namespace Phoenix::App::Dev
{
    struct Circle2DComponent
    {
        int32_t     Layer = 0;
        glm::vec2   Center;
        float       Radius = 1.0f;
        glm::vec4   Color = glm::vec4(1.0f);
        bool        Filled = false;
    };
}
