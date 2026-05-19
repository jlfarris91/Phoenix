#pragma once

#include "RendererTypes.h"

namespace Phoenix::Renderer
{
    struct SceneView
    {
        Vec2f Center;               // world-space camera center
        float PixelsPerUnit = 1.f;  // screen pixels per world unit
        int   Width  = 0;           // viewport width in pixels
        int   Height = 0;           // viewport height in pixels
    };
}
