#pragma once

#include <glm/vec2.hpp>

#include "SceneView.h"

namespace Phoenix::EnTT
{
    class CameraComponent
    {
        glm::vec2   Position;        // world-space center
        float       Zoom   = 1.f;    // pixels per world unit
        int         Width  = 0;      // viewport width in pixels
        int         Height = 0;      // viewport height in pixels

        Renderer::SceneView GetView() const
        {
            return { Position, Zoom, Width, Height };
        }
    };
}
