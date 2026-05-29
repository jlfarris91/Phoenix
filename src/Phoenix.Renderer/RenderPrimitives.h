#pragma once

#include "RendererTypes.h"
#include "Transform2D.h"
#include "Phoenix.Math/Rect2D.h"

namespace Phoenix::App
{
    struct Sprite2DCall
    {
        HTexture        Texture;
        Math::Rect2D    SourceRect;            // source region within the texture atlas (pixels)
        glm::vec2       WorldPos;              // world-space center
        float           Rotation = 0.f;        // radians, clockwise
        glm::vec2       Scale = {1.f, 1.f};
        Color4b         Tint = Color4b::White();
    };

    struct Line2DCall
    {
        glm::vec2   Start;
        glm::vec2   End;
        Color4b     Color = Color4b::White();
        float       Thickness = 1.f;
    };

    struct Circle2DCall
    {
        glm::vec2   Center;
        float       Radius = 1.f;
        Color4b     Color = Color4b::White();
        bool        Filled = false;
    };

    struct Rect2DCall
    {
        glm::vec2   Min;
        glm::vec2   Max;
        Color4b     Color = Color4b::White();
        bool        Filled = false;
    };

    struct Text2DCall
    {
        glm::vec2   WorldPos;
        char        Text[64] = {};
        Color4b     Color = Color4b::White();
        float       Size = 12.f;
    };

    struct Mesh2DCall
    {
        HMesh2D     Mesh;
        HTexture    Texture;               // invalid = vertex color only
        glm::vec2   WorldPos;
        float       Rotation = 0.f;        // radians, clockwise
        glm::vec2   Scale = {1.f, 1.f};
        Color4b     Tint = Color4b::White();
    };

    struct LineMesh2DCall
    {
        HLineMesh2D         Mesh;
        HTexture            Texture;
        Math::Transform2D   Transform;
        Color4b             Tint = Color4b::White();
    };
}
