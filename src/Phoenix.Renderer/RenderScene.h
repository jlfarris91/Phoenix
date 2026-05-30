#pragma once

#include "SceneView.h"
#include "RenderPrimitives.h"

#include <variant>
#include <vector>

namespace Phoenix::App
{
    using DrawCall = std::variant<
        Sprite2DCall,
        Mesh2DCall,
        LineMesh2DCall,
        Line2DCall,
        Circle2DCall,
        Rect2DCall,
        Text2DCall>;

    struct RenderCommand
    {
        int      Layer = 0;
        DrawCall Call;
    };

    struct RenderScene
    {
        SceneView                   View;
        Renderer::HResource         Target;    // invalid = window backbuffer
        std::vector<RenderCommand>  Commands;

        void Clear();
    };
}
