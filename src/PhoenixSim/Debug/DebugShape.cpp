#include "DebugShape.h"

Phoenix::DebugShape::DebugShape()
    : Data{}
    , ShapeType(Type::Invalid)
    , Color(Color::White)
{
}

Phoenix::DebugShape::DebugShape(const Line2& line, const Phoenix::Color& color)
    : Data{ .Line = line }
    , ShapeType(Type::Line)
    , Color(color)
{
}

Phoenix::DebugShape::DebugShape(const Circle2& circle, const Phoenix::Color& color)
    : Data{ .Circle = circle }
    , ShapeType(Type::Circle)
    , Color(color)
{
}

Phoenix::DebugShape::DebugShape(const Ellipse2& ellipse, const Phoenix::Color& color)
    : Data{ .Ellipse = ellipse }
    , ShapeType(Type::Ellipse)
    , Color(color)
{
}

Phoenix::DebugShape::DebugShape(const Box2& box, const Phoenix::Color& color)
    : Data{ .Box = box }
    , ShapeType(Type::Box)
    , Color(color)
{
}

Phoenix::DebugShape::DebugShape(const TextShape& text, const Phoenix::Color& color)
    : Data{ .Text = text }
    , ShapeType(Type::Text)
    , Color(color)
{
}
