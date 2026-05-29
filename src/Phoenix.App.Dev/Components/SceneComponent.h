#pragma once

#include "Phoenix.Math/Transform2D.h"

namespace Phoenix::App::Dev
{
    struct SceneComponent
    {
        int32_t     Layer = 0;
        glm::mat4   WorldTransform = glm::mat4(1.0f);
    };
}
