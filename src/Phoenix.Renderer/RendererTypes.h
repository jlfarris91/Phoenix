#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "Color.h"

namespace Phoenix::Renderer
{
    // Opaque handle to a GPU-resident texture. The backend owns the memory.
    struct HTexture
    {
        uint32_t Id = 0;

        bool IsValid() const { return Id != 0; }
        bool operator==(const HTexture&) const = default;
    };

    // Opaque handle to a GPU-resident 2D mesh. The backend owns the memory.
    struct HMesh2D
    {
        uint32_t Id = 0;

        bool IsValid() const { return Id != 0; }
        bool operator==(const HMesh2D&) const = default;
    };

    // Opaque handle to an off-screen render target. The backend owns the memory.
    struct HRenderTarget
    {
        uint32_t Id = 0;

        bool IsValid() const { return Id != 0; }
        bool operator==(const HRenderTarget&) const = default;
    };

    struct Vertex2D
    {
        glm::vec2 Pos;
        glm::vec2 UV    = {0.f, 0.f};
        Color4b Color = Color4b::White();
    };
}
