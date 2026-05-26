#pragma once

#include "Phoenix.Math/Transform2D.h"

namespace Phoenix::EnTT
{
    struct SceneComponent
    {
        Math::Transform2D LocalTransform;
        Math::Transform2D WorldTransform;
    };
}
