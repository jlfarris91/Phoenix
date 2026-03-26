#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/Containers/FixedArray.h"
#include "PhoenixSim/Debug/DebugShape.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API FeatureDebugScratchBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK(FeatureDebugScratchBlock)

        TInlineArray<DebugShape, 8192> Shapes;
        Color Colors[1024];
    };

    class PHOENIX_SIM_API FeatureDebug : public IFeature
    {
        PHX_REFLECT_TYPE(FeatureDebug, IFeature)

    public:

        FeatureDebug();

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
