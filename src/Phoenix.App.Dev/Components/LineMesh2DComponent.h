#pragma once

#include <glm/vec4.hpp>

#include "ResourceHandle.h"
#include "ResourcePtr.h"
#include "Resources/LineMesh2D.h"

namespace Phoenix::App::Dev
{
    struct LineMesh2DComponent
    {
        int32_t                             Layer = 0;
        Renderer::TResourcePtr<LineMesh2D>  Mesh;
        glm::vec4                           Tint = glm::vec4(1.0f);
        float                               Scale = 1.0f;
    };
}
