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

        // World Y+ is up; screen Y+ is down.
        Vec2f WorldToScreen(Vec2f world) const
        {
            return {
                (world.X - Center.X) * PixelsPerUnit + Width  * 0.5f,
                Height * 0.5f - (world.Y - Center.Y) * PixelsPerUnit
            };
        }

        Vec2f ScreenToWorld(Vec2f screen) const
        {
            return {
                (screen.X - Width  * 0.5f) / PixelsPerUnit + Center.X,
                (Height * 0.5f - screen.Y) / PixelsPerUnit + Center.Y
            };
        }
    };
}
