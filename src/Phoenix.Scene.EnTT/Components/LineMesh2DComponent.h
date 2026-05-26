#pragma once

#include "Phoenix/Name.h"
#include "RendererTypes.h"

namespace Phoenix::EnTT
{
    struct LineMesh2DComponent
    {
        FName Asset;
        float Scale = 1.0f;
        Color4b Tint = Color4b::White();
    };
}
