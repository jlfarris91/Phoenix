#pragma once

#include <SceneView.h>

namespace Phoenix
{
    struct SceneCamera
    {
        Renderer::Vec2f Position;        // world-space center
        float           Zoom   = 1.f;    // pixels per world unit
        int             Width  = 0;      // viewport width in pixels
        int             Height = 0;      // viewport height in pixels

        Renderer::SceneView GetView() const
        {
            return { Position, Zoom, Width, Height };
        }
    };
}
