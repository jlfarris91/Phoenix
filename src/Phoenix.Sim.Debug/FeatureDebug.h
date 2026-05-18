#pragma once

#include "Phoenix.Sim/Features.h"
#include "Phoenix.Sim/Containers/FixedArray.h"
#include "Phoenix.Sim.Debug/DebugShape.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API FeatureDebugScratchBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK(FeatureDebugScratchBlock)

        TInlineArray<DebugShape, 8192> Shapes;
        Color Colors[256];
    };

    class PHOENIX_SIM_API FeatureDebug : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureDebug)
        {
            FEATURE_WORLD_BLOCK(FeatureDebugScratchBlock, EBufferBlockType::Scratch)
            FEATURE_CHANNEL(FeatureChannels::PreWorldUpdate)
            FEATURE_CHANNEL(FeatureChannels::DebugRender)
        }

    public:

        static void DrawCircle(WorldRef world, const Vec2& pt, Distance radius, const Color& color);
        static void DrawCircle(WorldRef world, const Circle2& circle, const Color& color);

        static void DrawEllipse(WorldRef world, const Vec2& pt, const Vec2& radius, const Color& color);
        static void DrawEllipse(WorldRef world, const Ellipse2& ellipse, const Color& color);

        static void DrawLine(WorldRef world, const Vec2& v0, const Vec2& v1, const Color& color);
        static void DrawLine(WorldRef world, const Line2& line, const Color& color);

        static void DrawLines(WorldRef world, const Vec2* points, size_t num, const Color& color);
        static void DrawLines(WorldRef world, const Line2* lines, size_t num, const Color& color);

        static void DrawRay(WorldRef world, const Vec2& start, const Vec2& dir, const Color& color);

        static void DrawBox(WorldRef world, const Vec2& min, const Vec2& max, const Color& color);
        static void DrawBox(WorldRef world, const Box2& box, const Color& color);

        static void DrawDebugText(WorldRef world, const Vec2& pt, const char* str, size_t len, const Color& color);

        static Color GetColor(WorldConstRef world, size_t index);

    protected:

        void OnWorldInitialize(WorldRef world) override;
        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;
        void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) override;
    };
}

PHX_DEFINE_TYPE(Phoenix::FeatureDebug)
{
    registration
        .Namespace("Phoenix.Debug")
        .StaticMethod<void, WorldRef, const Vec2&, Distance, const Color&>    ("DrawCircle(world, pt, radius, color)",  &FeatureDebug::DrawCircle)
        .StaticMethod<void, WorldRef, const Vec2&, const Vec2&, const Color&> ("DrawEllipse(world, pt, radii, color)",  &FeatureDebug::DrawEllipse)
        .StaticMethod<void, WorldRef, const Vec2&, const Vec2&, const Color&> ("DrawLine(world, start, end, color)",    &FeatureDebug::DrawLine)
        .StaticMethod<void, WorldRef, const Vec2&, const Vec2&, const Color&> ("DrawRay(world, start, dir, color)",     &FeatureDebug::DrawRay)
        .StaticMethod<void, WorldRef, const Vec2&, const Vec2&, const Color&> ("DrawBox(world, min, max, color)",       &FeatureDebug::DrawBox)
        .StaticMethod                                                         ("GetColor(world, index)",                &FeatureDebug::GetColor);
}