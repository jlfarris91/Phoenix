#pragma once

#include <glm/glm.hpp>

namespace Phoenix::App
{
    struct SceneView
    {
        // World-space camera center
        glm::vec2 Center;

        // Screen pixels per world unit
        float PixelsPerUnit = 1.f;

        // Viewport width in pixels
        int   Width  = 0;

        // Viewport height in pixels
        int   Height = 0;

        // World Y+ is up; screen Y+ is down.
        glm::vec2 WorldToScreen(glm::vec2 world) const
        {
            return {
                (world.x - Center.x) * PixelsPerUnit + Width  * 0.5f,
                Height * 0.5f - (world.y - Center.y) * PixelsPerUnit
            };
        }

        glm::vec2 ScreenToWorld(glm::vec2 screen) const
        {
            return {
                (screen.x - Width  * 0.5f) / PixelsPerUnit + Center.x,
                (Height * 0.5f - screen.y) / PixelsPerUnit + Center.y
            };
        }
    };
}
