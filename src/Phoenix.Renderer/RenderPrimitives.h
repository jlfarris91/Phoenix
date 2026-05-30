#pragma once

#include "ResourceHandle.h"
#include "Phoenix.Math/Rect2D.h"

namespace Phoenix::App
{
    struct Sprite2DCall
    {
        Renderer::HResource Texture;
        Math::Rect2D        SourceRect;            // source region within the texture atlas (pixels)
        glm::vec2           WorldPos;              // world-space center
        float               Rotation = 0.f;        // radians, clockwise
        glm::vec2           Scale = {1.f, 1.f};
        glm::vec4           Tint = glm::vec4(1);
    };

    struct Line2DCall
    {
        glm::vec2   Start;
        glm::vec2   End;
        glm::vec4   Color = glm::vec4(1);
        float       Thickness = 1.f;
    };

    struct Circle2DCall
    {
        glm::vec2   Center;
        float       Radius = 1.f;
        glm::vec4   Color = glm::vec4(1);
        bool        Filled = false;
    };

    struct Rect2DCall
    {
        glm::vec2   Min;
        glm::vec2   Max;
        glm::vec4   Color = glm::vec4(1);
        bool        Filled = false;
    };

    struct Text2DCall
    {
        glm::vec2   WorldPos;
        char        Text[64] = {};
        glm::vec4   Color = glm::vec4(1.0f);
        float       Size = 12.f;
    };

    struct Mesh2DCall
    {
        Renderer::HResource Mesh;
        Renderer::HResource Texture;               // invalid = vertex color only
        glm::vec2           WorldPos;
        float               Rotation = 0.f;        // radians, clockwise
        glm::vec2           Scale = {1.f, 1.f};
        glm::vec4           Tint = glm::vec4(1.0f);
    };

    struct LineMesh2DCall
    {
        Renderer::HResource Mesh;
        glm::vec2           Scale = glm::vec2(1.0f);
        glm::vec4           Tint = glm::vec4(1.0f);
        glm::mat4           Transform;
    };
}
