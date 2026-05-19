#pragma once

#include <cstdint>

namespace Phoenix::Renderer
{
    struct Vec2f
    {
        float X = 0.f;
        float Y = 0.f;
    };

    struct Rect2f
    {
        float X = 0.f;
        float Y = 0.f;
        float W = 0.f;
        float H = 0.f;
    };

    struct Color4b
    {
        uint8_t R = 255;
        uint8_t G = 255;
        uint8_t B = 255;
        uint8_t A = 255;

        static constexpr Color4b White()       { return {255, 255, 255, 255}; }
        static constexpr Color4b Black()       { return {  0,   0,   0, 255}; }
        static constexpr Color4b Red()         { return {255,   0,   0, 255}; }
        static constexpr Color4b Green()       { return {  0, 255,   0, 255}; }
        static constexpr Color4b Blue()        { return {  0,   0, 255, 255}; }
        static constexpr Color4b Yellow()      { return {255, 255,   0, 255}; }
        static constexpr Color4b Transparent() { return {  0,   0,   0,   0}; }
    };

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
        Vec2f   Pos;
        Vec2f   UV    = {0.f, 0.f};
        Color4b Color = Color4b::White();
    };
}
