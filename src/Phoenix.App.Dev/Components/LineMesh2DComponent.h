#pragma once

#include "RendererTypes.h"

namespace Phoenix::App::Dev
{
    struct LineMesh2DComponent
    {
        int32_t               Layer = 0;
        App::HLineMesh2D Mesh;
        App::HTexture    Texture;
        Color4b               Tint = Color4b::White();
    };
}
