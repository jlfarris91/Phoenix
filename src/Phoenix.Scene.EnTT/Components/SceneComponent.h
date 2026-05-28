#pragma once

#include "Phoenix.Math/Transform2D.h"

namespace Phoenix::EnTT
{
    struct SceneComponent
    {
        int32_t             Layer = 0;
        Math::Transform2D   LocalTransform;
        glm::mat3           WorldTransform;
    };
}
