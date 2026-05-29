#pragma once

#include "RendererTypes.h"

namespace Phoenix::App::Dev
{
    struct Circle2DComponent
    {
        int32_t     Layer = 0;
        glm::vec2   Center;
        float       Radius = 1.f;
        Color4b     Color = Color4b::White();
        bool        Filled = false;
    };
}
