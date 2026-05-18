#include "FeatureDebug.h"

#include "Debug.h"
#include "Phoenix.Sim/Worlds.h"
#include "Phoenix.Sim/Strings/FeatureString.h"

using namespace Phoenix;

void FeatureDebug::DrawCircle(WorldRef world, const Vec2& pt, Distance radius, const Color& color)
{
    DrawCircle(world, Circle2(pt, radius), color);
}

void FeatureDebug::DrawCircle(WorldRef world, const Circle2& circle, const Color& color)
{
    FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();
    block.Shapes.EmplaceBack(circle, color);
}

void FeatureDebug::DrawEllipse(WorldRef world, const Vec2& pt, const Vec2& radius, const Color& color)
{
    DrawEllipse(world, Ellipse2(pt, radius), color);
}

void FeatureDebug::DrawEllipse(WorldRef world, const Ellipse2& ellipse, const Color& color)
{
    FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();
    block.Shapes.EmplaceBack(ellipse, color);
}

void FeatureDebug::DrawLine(WorldRef world, const Vec2& v0, const Vec2& v1, const Color& color)
{
    DrawLine(world, Line2(v0, v1), color);
}

void FeatureDebug::DrawLine(WorldRef world, const Line2& line, const Color& color)
{
    FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();
    block.Shapes.EmplaceBack(line, color);
}

void FeatureDebug::DrawLines(WorldRef world, const Vec2* points, size_t num, const Color& color)
{
    for (size_t i = 0; i < num; ++i)
    {
        DrawLine(world, points[i], points[(i + 1) % num], color);
    }
}

void FeatureDebug::DrawLines(WorldRef world, const Line2* lines, size_t num, const Color& color)
{
    for (size_t i = 0; i < num; ++i)
    {
        DrawLine(world, lines[i].Start, lines[i].End, color);
    }
}

void FeatureDebug::DrawRay(WorldRef world, const Vec2& start, const Vec2& dir, const Color& color)
{
    DrawLine(world, start, start + dir, color);
}

void FeatureDebug::DrawBox(WorldRef world, const Vec2& min, const Vec2& max, const Color& color)
{
    DrawBox(world, Box2(min, max), color);
}

void FeatureDebug::DrawBox(WorldRef world, const Box2& box, const Color& color)
{
    FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();
    block.Shapes.EmplaceBack(box, color);
}

void FeatureDebug::DrawDebugText(
    WorldRef world,
    const Vec2& pt,
    const char* str,
    size_t len,
    const Color& color)
{
    FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();
    block.Shapes.EmplaceBack(DebugShape::TextShape{ pt, FName(str, len) }, color);
}

Color FeatureDebug::GetColor(WorldConstRef world, size_t index)
{
    const FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();
    return block.Colors[index % _countof(block.Colors)];
}

void FeatureDebug::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    Random& random = world.GetRandom();
    FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();

    for (Color& color : block.Colors)
    {
        color = Color(random.Next<uint8>() % 255, random.Next<uint8>() % 255, random.Next<uint8>() % 255, 255);
    }
}

void FeatureDebug::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    IFeature::OnPreWorldUpdate(world, args);
    FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();
    block.Shapes.Reset();
}

void FeatureDebug::OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer)
{
    IFeature::OnDebugRender(world, state, renderer);

    const FeatureDebugScratchBlock& block = world.GetBlockRef<FeatureDebugScratchBlock>();

    for (const DebugShape& shape : block.Shapes)
    {
        switch (shape.ShapeType)
        {
            case DebugShape::Type::Line:
                renderer.DrawLine(shape.Data.Line.Start, shape.Data.Line.End, shape.Color);
                break;
            case DebugShape::Type::Circle:
                renderer.DrawCircle(shape.Data.Circle.Origin, shape.Data.Circle.Radius, shape.Color);
                break;
            case DebugShape::Type::Ellipse:
                renderer.DrawEllipse(shape.Data.Ellipse.Origin, shape.Data.Ellipse.Radius, shape.Color);
                break;
            case DebugShape::Type::Box:
                renderer.DrawRect(shape.Data.Box.Min, shape.Data.Box.Max, shape.Color);
                break;
            case DebugShape::Type::Text:
                {
                    const char* text = FeatureString::Get(world, shape.Data.Text.Name);
                    uint32 len = (uint32)strlen(text);
                    renderer.DrawDebugText(shape.Data.Text.Pos, text, len, shape.Color);
                }
                break;
            case DebugShape::Type::Invalid:
                // Handle invalid shape type
                break;
        }
    }
}
