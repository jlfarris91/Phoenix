#pragma once

#include "RendererTypes.h"


namespace Phoenix::Renderer
{
    struct Sprite2DCall
    {
        HTexture Texture;
        Rect2f   SourceRect;            // source region within the texture atlas (pixels)
        Vec2f    WorldPos;              // world-space center
        float    Rotation = 0.f;        // radians, clockwise
        Vec2f    Scale    = {1.f, 1.f};
        Color4b  Tint     = Color4b::White();
    };

    struct Line2DCall
    {
        Vec2f   Start;
        Vec2f   End;
        Color4b Color     = Color4b::White();
        float   Thickness = 1.f;
    };

    struct Circle2DCall
    {
        Vec2f   Center;
        float   Radius = 1.f;
        Color4b Color  = Color4b::White();
        bool    Filled = false;
    };

    struct Rect2DCall
    {
        Vec2f   Min;
        Vec2f   Max;
        Color4b Color  = Color4b::White();
        bool    Filled = false;
    };

    struct Text2DCall
    {
        Vec2f   WorldPos;
        char    Text[64] = {};
        Color4b Color    = Color4b::White();
        float   Size     = 12.f;
    };

    struct Mesh2DCall
    {
        HMesh2D  Mesh;
        HTexture Texture;               // invalid = vertex color only
        Vec2f    WorldPos;
        float    Rotation = 0.f;        // radians, clockwise
        Vec2f    Scale    = {1.f, 1.f};
        Color4b  Tint     = Color4b::White();
    };
}
